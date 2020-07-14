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
} // namespace Models