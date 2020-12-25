#include "quote_dispatcher.h"

QuoteDispatcher::QuoteDispatcher(BitmexStore &store, QuotingEngine &engine, Interfaces::IOrderEntryGateway &oe, Interfaces::IPositionGateway &pg, Interfaces::IRateLimitMonitor &rl, Interfaces::IExchangeDetailsGateway &details, unsigned int pairs)
    : store(store), engine(engine), oe(oe), pg(pg), rl(rl), details(details), pairs(pairs)
{
  this->to_amend.reserve(10);
  this->to_create.reserve(10);
  this->to_cancel.reserve(10);

  engine.new_quote += Poco::delegate(this, &QuoteDispatcher::on_new_quote);
  oe.n_orders += Poco::delegate(this, &QuoteDispatcher::on_order_update);
}

QuoteDispatcher::~QuoteDispatcher()
{
  engine.new_quote -= Poco::delegate(this, &QuoteDispatcher::on_new_quote);
  oe.n_orders -= Poco::delegate(this, &QuoteDispatcher::on_order_update);
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
  // bool enough = this->has_enough_margin(two_sided_quote.bid.price, two_sided_quote.bid.size, two_sided_quote.ask.price, two_sided_quote.ask.size);
  // if (!enough)
  // {
  //   auto open_orders = this->oe.open_orders();
  //   if (open_orders && (open_orders.value().size() > 0))
  //   {
  //     this->oe.cancel_all();
  //   }
  //   return;
  // }

  bool rate_limited = this->rl.is_rate_limited();
  if (rate_limited)
  {
    auto open_orders = this->oe.open_orders();
    if (open_orders && (open_orders.value().size() > 0))
    {
      this->oe.cancel_all();
      std::cout << "Rate limit reached, all orders canceled" << std::endl;
    }
    return;
  }

  // todo: target position
  this->converge_orders(two_sided_quote.bids, two_sided_quote.asks, two_sided_quote.time);
}

void QuoteDispatcher::on_order_update(const void *, long &n_orders)
{
  // remove left over orders if any
  if (n_orders <= 2 * this->pairs)
    return;
  auto two_sided_quote = this->engine.get_latest();
  if (!two_sided_quote)
    return;

  bool rate_limited = this->rl.is_rate_limited();
  if (rate_limited)
  {
    auto open_orders = this->oe.open_orders();
    if (open_orders && (open_orders.value().size() > 0))
    {
      this->oe.cancel_all();
      std::cout << "Rate limit reached, all orders canceled" << std::endl;
    }
    return;
  }

  this->converge_orders(two_sided_quote->bids, two_sided_quote->asks, Poco::DateTime());
}

void QuoteDispatcher::converge_orders(std::vector<Models::Quote> &bids, std::vector<Models::Quote> &asks, Poco::DateTime time)
{
  // prevent concurrent convergence
  std::lock_guard<std::mutex> guard(this->converge_mutex);

  this->to_create.clear();
  this->to_amend.clear();
  this->to_cancel.clear();

  unsigned int buys_matched = 0;
  unsigned int sells_matched = 0;
  unsigned int sum = bids.size() + asks.size();

  json existing_orders = this->oe.open_orders().value();
  // find orders can be amended
  for (const auto &order : existing_orders)
  {
    if ((buys_matched + sells_matched) >= sum)
    {
      Models::CancelOrder cancel(order["clOrdID"].get<std::string>(), time);
      this->to_cancel.emplace_back(std::move(cancel));
    }
    else
    {
      Models::Quote *p_desired_quote = nullptr;
      if (order["side"] == "Buy")
      {
        p_desired_quote = &bids[buys_matched];
        buys_matched += 1;
      }
      else
      {
        p_desired_quote = &asks[sells_matched];
        sells_matched += 1;
      }

      if (p_desired_quote)
      {
        if ((p_desired_quote->size != order["leavesQty"].get<double>()) || (p_desired_quote->price != order["price"].get<double>()))
        {
          Models::ReplaceOrder replace(order["clOrdID"].get<std::string>(), p_desired_quote->price, p_desired_quote->size, time);
          this->to_amend.emplace_back(std::move(replace));
        }
      }
    }
  }

  while (buys_matched < bids.size())
  {
    const Models::Quote *p_desired_quote = &bids[buys_matched];
    Models::NewOrder new_order(this->details.pair, oe.generate_client_id(), p_desired_quote->price, p_desired_quote->size, p_desired_quote->side, Models::OrderType::Limit, Models::TimeInForce::GTC, time, true);
    this->to_create.emplace_back(std::move(new_order));
    ++buys_matched;
  }

  while (sells_matched < asks.size())
  {
    const Models::Quote *p_desired_quote = &asks[sells_matched];
    Models::NewOrder new_order(this->details.pair, oe.generate_client_id(), p_desired_quote->price, p_desired_quote->size, p_desired_quote->side, Models::OrderType::Limit, Models::TimeInForce::GTC, time, true);
    this->to_create.emplace_back(std::move(new_order));
    ++sells_matched;
  }

  if (this->to_amend.size() > 0)
  {
    this->oe.batch_replace_order(this->to_amend);
  }

  if (this->to_create.size() > 0)
  {
    this->oe.batch_send_order(this->to_create);
  }

  if (this->to_cancel.size() > 0)
  {
    this->oe.batch_cancel_order(this->to_cancel);
  }
}