#include "models.h"

namespace Models
{
  Poco::DateTime iso8601_to_datetime(const std::string &s)
  {
    Poco::DateTime dt;
    int tzd;
    Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT, s, dt, tzd);
    return dt;
  }

  MarketQuote::MarketQuote(json &q)
  {
    this->askPrice = q["askPrice"].get<double>();
    this->askSize = q["askSize"].get<double>();
    this->bidPrice = q["bidPrice"].get<double>();
    this->bidSize = q["bidSize"].get<double>();
    this->symbol = q["symbol"].get<std::string>();
    this->time = iso8601_to_datetime(q["timestamp"].get<std::string>());
  }

  MarketQuote::MarketQuote(double askPrice, double askSize, double bidPrice, double bidSize, const std::string &symbol, Poco::DateTime time)
      : askPrice(askPrice), askSize(askSize), bidPrice(bidPrice), bidSize(bidSize), symbol(symbol)
  {
    this->time = time;
  }

  Quote::Quote(double price, double size, Side side)
      : price(price), size(size), side(side)
  {
  }

  TwoSidedQuote::TwoSidedQuote(Quote bid, Quote ask, Poco::DateTime time)
      : bid(bid), ask(ask)
  {
    this->time = time;
  }

  QuoteOrder::QuoteOrder(Quote &quote, const std::string &orderId)
      : quote(quote), orderId(orderId)
  {
  }

  FairValue::FairValue(double price, Poco::DateTime time)
      : price(price)
  {
    this->time = time;
  }

  QuotingParameters::QuotingParameters(double width, double size, double target_base_position, double position_divergence, double skew_factor, double trades_per_minute, double trade_rate_seconds)
      : width(width), size(size), target_base_position(target_base_position), position_divergence(position_divergence), skew_factor(skew_factor), trades_per_minute(trades_per_minute), trade_rate_seconds(trade_rate_seconds)
  {
  }

  NewOrder::NewOrder(const std::string &symbol, const std::string &clOrdID, double price, double orderQty, Side side, OrderType type, TimeInForce time_in_force, Poco::DateTime time, bool post_only)
      : symbol(symbol), clOrdID(clOrdID), price(price), orderQty(orderQty), side(side), type(type), time_in_force(time_in_force), time(time), post_only(post_only)
  {
  }

  json NewOrder::to_json() const
  {
    std::string s;
    std::string t;
    std::string tif;

    switch (side)
    {
    case Side::Bid:
      s = "Buy";
      break;
    case Side::Ask:
      s = "Sell";
    }

    switch (type)
    {
    case OrderType::Limit:
      t = "Limit";
      break;
    case OrderType::Market:
      t = "Market";
    }

    switch (time_in_force)
    {
    case TimeInForce::GTC:
      tif = "GoodTillCancel";
      break;
    case TimeInForce::FOK:
      tif = "FillOrKill";
      break;
    case TimeInForce::IOC:
      tif = "ImmediatetOrCancel";
    }

    json j = {
        {"symbol", symbol},
        {"clOrdID", clOrdID},
        {"price", price},
        {"orderQty", orderQty},
        {"side", s},
        {"ordType", t},
        {"TimeInForce", tif}};

    if (post_only)
      j["exeInst"] = "ParticipateDoNotInitiate";

    return j;
  }

  ReplaceOrder::ReplaceOrder(const std::string &origClOrdID, double price, double orderQty, Poco::DateTime time)
      : origClOrdID(origClOrdID), price(price), orderQty(orderQty), time(time)
  {
  }

  json ReplaceOrder::to_json() const
  {
    json j = {
        {"origClOrdID", origClOrdID},
        {"price", price},
        {"orderQty", orderQty}};

    return j;
  }

  CancelOrder::CancelOrder(const std::string &clOrdID, Poco::DateTime time)
      : clOrdID(clOrdID), time(time)
  {
  }

  json CancelOrder::to_json() const
  {
    json j = {
        {"cliOrdID", clOrdID}};

    return j;
  }

  Trade::Trade(const std::string ID, Poco::DateTime time, Side side, double size, double price, Liquidity liquidity, double homeNotional, double foreignNotional)
      : ID(ID), time(time), side(side), size(size), price(price), liquidity(liquidity), homeNotional(homeNotional), foreignNotional(foreignNotional)
  {
  }
} // namespace Models