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

    MarketQuote(json &q)
    {
      this->askPrice = q["askPrice"].get<double>();
      this->askSize = q["askSize"].get<double>();
      this->bidPrice = q["bidPrice"].get<double>();
      this->bidSize = q["bidSize"].get<double>();
      this->symbol = q["symbol"].get<std::string>();
      this->time = util::datetime::parse_iso_8601_string(q["timestamp"].get<std::string>());
    }
  };
} // namespace Models
