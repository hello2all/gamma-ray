#pragma once
#include <iostream>
#include "json.hpp"
#include "util.h"

using json = nlohmann::json;

namespace Models
{
  enum class Exchange
  {
    bitmex
  };

  enum class ConnectivityStatus
  {
    connected,
    disconnected
  };

  enum class QuotingMode
  {
    Top
  };

  class MarketQuote
  {
  public:
    double askPrice;
    double askSize;
    double bidPrice;
    double bidSize;
    std::string symbol;
    Poco::DateTime time;

    MarketQuote()
    {
    }

    MarketQuote(json &q)
    {
      this->askPrice = q["askPrice"].get<double>();
      this->askSize = q["askSize"].get<double>();
      this->bidPrice = q["bidPrice"].get<double>();
      this->bidSize = q["bidSize"].get<double>();
      this->symbol = q["symbol"].get<std::string>();
      this->time = util::datetime::parse_iso_8601_string(q["timestamp"].get<std::string>());
    }

    MarketQuote(double askPrice, double askSize, double bidPrice, double bidSize, const std::string &symbol, Poco::DateTime time)
        : askPrice(askPrice), askSize(askSize), bidPrice(bidPrice), bidSize(bidSize), symbol(symbol), time(time)
    {
    }
  };

  enum class Side
  {
    Bid,
    Ask
  };

  class Quote
  {
  public:
    double price;
    double size;
    Side side;

    Quote(double price, double size, Side side)
        : price(price), size(size), side(side)
    {
    }
  };

  class QuoteOrder
  {
  public:
    Quote quote;
    std::string orderId;
    QuoteOrder(Quote &quote, const std::string &orderId)
        : quote(quote), orderId(orderId)
    {
    }
  };

  class FairValue
  {
  public:
    double price;
    Poco::DateTime time;

    FairValue()
    {
    }

    FairValue(double price, Poco::DateTime time)
        : price(price), time(time)
    {
    }
  };

  class QuotingParameters
  {
  public:
    double width;
    double size;
    double target_base_position;
    double position_divergence;
    double skew_factor;
    double trades_per_minute;
    double trade_rate_seconds;

    QuotingParameters(double width, double size, double target_base_position, double position_divergence, double skew_factor, double trades_per_minute, double trade_rate_seconds)
        : width(width), size(size), target_base_position(target_base_position), position_divergence(position_divergence), skew_factor(skew_factor), trades_per_minute(trades_per_minute), trade_rate_seconds(trade_rate_seconds)
    {
    }
  };
} // namespace Models
