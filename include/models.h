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

    FairValue(double price, Poco::DateTime time)
        : price(price), time(time)
    {
    }
  };
} // namespace Models
