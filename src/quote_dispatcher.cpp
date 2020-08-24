#include "quote_dispatcher.h"

QuoteDispatcher::QuoteDispatcher(BitmexStore &store, QuotingEngine &engine, Interfaces::IOrderEntryGateway &oe, Interfaces::IPositionGateway &pg, Interfaces::IRateLimitMonitor &rl, Interfaces::IExchangeDetailsGateway &details)
    : store(store), engine(engine), oe(oe), pg(pg), rl(rl), details(details)
{
  this->bids.reserve(5);
  this->asks.reserve(5);
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
    }
    return;
  }

  // todo: target position

  this->bids.clear();
  this->asks.clear();
  this->bids.push_back(two_sided_quote.bid);
  this->asks.push_back(two_sided_quote.ask);

  this->converge_orders(this->bids, this->asks, two_sided_quote.time);
}

void QuoteDispatcher::on_order_update(const void *, long &n_orders)
{
  if (n_orders == 2)
    return;
  if (!this->engine.get_latest())
    return;

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
    }
    return;
  }

  // todo: target position

  this->converge_orders(this->bids, this->asks, Poco::DateTime());
}

void QuoteDispatcher::converge_orders(std::vector<Models::Quote> &bids, std::vector<Models::Quote> &asks, Poco::DateTime time)
{
  this->to_create.clear();
  this->to_amend.clear();
  this->to_cancel.clear();

  unsigned int buys_matched = 0;
  unsigned int sells_matched = 0;
  unsigned int sum = bids.size() + asks.size();

  // todo: avoid memory copy
  json existing_orders = this->oe.open_orders().value();

  // find orders can be amended
  for (const auto &order : existing_orders)
  {
    if ((buys_matched + sells_matched) >= sum)
    {
      std::string clOrdID = order["clOrdID"].get<std::string>();
      Models::CancelOrder cancel(clOrdID, time);
      this->to_cancel.emplace_back(cancel);
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
          std::string clOrdID = order["clOrdID"].get<std::string>();
          Models::ReplaceOrder replace(clOrdID, p_desired_quote->price, p_desired_quote->size, time);
          this->to_amend.emplace_back(replace);
        }
      }
    }
  }

  while (buys_matched < bids.size())
  {
    std::string clOrdID = oe.generate_client_id();
    const Models::Quote *p_desired_quote = &bids[buys_matched];
    Models::NewOrder new_order(this->details.pair, clOrdID, p_desired_quote->price, p_desired_quote->size, p_desired_quote->side, Models::OrderType::Limit, Models::TimeInForce::GTC, time, true);
    this->to_create.emplace_back(new_order);
    ++buys_matched;
  }

  while (sells_matched < asks.size())
  {
    std::string clOrdID = oe.generate_client_id();
    const Models::Quote *p_desired_quote = &asks[sells_matched];
    Models::NewOrder new_order(this->details.pair, clOrdID, p_desired_quote->price, p_desired_quote->size, p_desired_quote->side, Models::OrderType::Limit, Models::TimeInForce::GTC, time, true);
    this->to_create.emplace_back(new_order);
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