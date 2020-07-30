#include "quote_dispatcher.h"

QuoteDispatcher::QuoteDispatcher(BitmexStore &store, QuotingEngine &engine, Interfaces::IOrderEntryGateway &oe, Interfaces::IPositionGateway &pg, Interfaces::IRateLimitMonitor &rl, Interfaces::IExchangeDetailsGateway &details)
    : store(store), engine(engine), oe(oe), pg(pg), rl(rl), details(details)
{
  engine.new_quote += Poco::delegate(this, &QuoteDispatcher::on_new_quote);
}

QuoteDispatcher::~QuoteDispatcher()
{
  engine.new_quote -= Poco::delegate(this, &QuoteDispatcher::on_new_quote);
}

bool QuoteDispatcher::has_enough_margin(double bid_price, double bid_size, double ask_price, double ask_size)
{
  auto margin = this->pg.get_latest_margin();
  if (!margin)
  {
    return false;
  }
  else
  {
    long available = margin.value()["availableMargin"].get<long>();
    long sum = 0;
    if (bid_size > 0)
    {
      sum += static_cast<long>(bid_size * this->details.face_value * this->details.max_leverage * 1.0e6 / bid_price);
    }
    if (ask_size > 0)
    {
      sum += static_cast<long>(ask_size * this->details.face_value * this->details.max_leverage * 1.0e6 / ask_price);
    }

    if (sum < available)
      return true;
    else
      return false;
  }
}

void QuoteDispatcher::on_new_quote(const void *, Models::TwoSidedQuote &two_sided_quote)
{
  bool enough = this->has_enough_margin(two_sided_quote.bid.price, two_sided_quote.bid.size, two_sided_quote.ask.price, two_sided_quote.ask.size);
  if (!enough)
  {
    auto open_orders = this->oe.open_orders();
    if (open_orders && (open_orders.value().size() > 0))
    {
      this->oe.cancel_all();
    }
    return;
  }

  bool rate_limited = this->rl.is_rate_limited();
  if (rate_limited)
  {
    auto open_orders = this->oe.open_orders();
    if (open_orders && (open_orders.value().size() > 0))
    {
      this->oe.cancel_all();
    }
    return;
  }
  // todo: target position
  std::vector<Models::Quote> bids = {two_sided_quote.bid};
  std::vector<Models::Quote> asks = {two_sided_quote.ask};

  this->converge_orders(bids, asks, two_sided_quote.time);
}

void QuoteDispatcher::converge_orders(std::vector<Models::Quote> bids, std::vector<Models::Quote> asks, Poco::DateTime time)
{
  std::vector<Models::ReplaceOrder> to_amend;
  std::vector<Models::NewOrder> to_create;
  std::vector<Models::CancelOrder> to_cancel;

  unsigned int buys_matched = 0;
  unsigned int sells_matched = 0;

  json existing_orders = this->oe.open_orders().value();
  std::cout << existing_orders.dump(2) << std::endl;

  // find orders can be amended
  for (const auto &order : existing_orders)
  {
    Models::Quote *p_desired_quote = nullptr;
    if (order["side"] == "Buy")
    {
      std::cout << "found replacible bid" << std::endl;
      p_desired_quote = &bids[buys_matched];
      buys_matched += 1;
    }
    else
    {
      std::cout << "found replacible ask" << std::endl;
      p_desired_quote = &asks[sells_matched];
      sells_matched += 1;
    }

    if (p_desired_quote)
    {
      if ((p_desired_quote->size != order["leavesQty"].get<double>()) || (p_desired_quote->price != order["price"].get<double>()))
      {
        std::cout << "need replace" << std::endl;
        std::string clOrdID = order["clOrdID"].get<std::string>();
        Models::ReplaceOrder replace(clOrdID, p_desired_quote->price, p_desired_quote->size, time);
        to_amend.push_back(replace);
      }
    }
  }

  while (buys_matched < bids.size())
  {
    std::cout << "new bid" << std::endl;
    std::string clOrdID = oe.generate_client_id();
    const Models::Quote *p_desired_quote = &bids[buys_matched];
    Models::NewOrder new_order(this->details.pair, clOrdID, p_desired_quote->price, p_desired_quote->size, p_desired_quote->side, Models::OrderType::Limit, Models::TimeInForce::GTC, time, true);
    to_create.push_back(new_order);
    ++buys_matched;
  }

  while (sells_matched < asks.size())
  {
    std::cout << "new ask" << std::endl;
    std::string clOrdID = oe.generate_client_id();
    const Models::Quote *p_desired_quote = &asks[sells_matched];
    Models::NewOrder new_order(this->details.pair, clOrdID, p_desired_quote->price, p_desired_quote->size, p_desired_quote->side, Models::OrderType::Limit, Models::TimeInForce::GTC, time, true);
    to_create.push_back(new_order);
    ++sells_matched;
  }

  if (to_amend.size() > 0)
  {
    std::cout << "batch replace" << to_amend.size() << std::endl;
    this->oe.batch_replace_order(to_amend);
  }

  if (to_create.size() > 0)
  {
    std::cout << "batch send" << to_create.size() << std::endl;
    this->oe.batch_send_order(to_create);
  }

  if (to_cancel.size() > 0)
  {
    std::cout << "batch send" << to_cancel.size() << std::endl;
    this->oe.batch_cancel_order(to_cancel);
  }
}