#pragma once
#include <iostream>
#include "json.hpp"
#include "Poco/DateTime.h"
#include "Poco/DateTimeParser.h"
#include "Poco/DateTimeFormat.h"

using json = nlohmann::json;

namespace Models
{
  // === ENUMS ===
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

  // === FUNCTIONS ===
  Poco::DateTime iso8601_to_datetime(const std::string &s);

  // === CLASSES ===
  class Timestamped
  {
  public:
    Poco::DateTime time;
  };

  class MarketQuote : public Timestamped
  {
  public:
    double askPrice;
    double askSize;
    double bidPrice;
    double bidSize;
    std::string symbol;

    MarketQuote(json &q);
    MarketQuote(double askPrice, double askSize, double bidPrice, double bidSize, const std::string &symbol, Poco::DateTime time);
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

    Quote(double price, double size, Side side);
  };

  class TwoSidedQuote : public Timestamped
  {
  public:
    Quote bid;
    Quote ask;

    TwoSidedQuote(Quote bid, Quote ask, Poco::DateTime time);
  };

  class QuoteOrder
  {
  public:
    Quote quote;
    std::string orderId;

    QuoteOrder(Quote &quote, const std::string &orderId);
  };

  class FairValue : public Timestamped
  {
  public:
    double price;

    FairValue(double price, Poco::DateTime time);
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

    QuotingParameters(double width, double size, double target_base_position, double position_divergence, double skew_factor, double trades_per_minute, double trade_rate_seconds);
  };
} // namespace Models
