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
    Top,
    Mid,
    AvellanedaStoikov
  };

  enum class Side
  {
    Bid,
    Ask
  };

  enum class OrderType
  {
    Limit,
    Market
  };

  enum class TimeInForce
  {
    GTC,
    FOK,
    IOC
  };

  enum class Liquidity
  {
    Taker,
    Maker
  };

  enum class Currency
  {
    XBt,
    ETH,
    USD
  };

  // === Mapping ===
  static std::unordered_map<std::string, QuotingMode> const quoting_mode_table = {
      {"Top", QuotingMode::Top},
      {"Mid", QuotingMode::Mid},
      {"AvellanedaStoikov", QuotingMode::AvellanedaStoikov}};

  // === FUNCTIONS ===
  Poco::DateTime iso8601_to_datetime(const std::string &s);
  QuotingMode get_quoting_mode(const std::string &s);

  // === CLASSES ===
  class Timestamped
  {
  public:
    Poco::DateTime time;
    Timestamped();
    Timestamped(Poco::DateTime &&time);
    Timestamped(const Poco::DateTime &time);
  };

  class MarketQuote : public Timestamped
  {
  public:
    double askPrice;
    double askSize;
    double bidPrice;
    double bidSize;
    std::string symbol;

    MarketQuote(const json &q, bool use_server_time = false);
    MarketQuote(double askPrice, double askSize, double bidPrice, double bidSize, const std::string &symbol, Poco::DateTime &&time);
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
    std::vector<Quote> bids;
    std::vector<Quote> asks;

    TwoSidedQuote(std::vector<Quote> bids, std::vector<Quote> asks, Poco::DateTime time);
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
    QuotingMode mode;
    double size;
    unsigned int pairs;
    double price_interval;
    double size_increment;
    double target_base_position;
    double position_divergence;
    double skew_factor;
    double trades_per_minute;
    double trade_rate_seconds;

    QuotingParameters(QuotingMode mode, double size, unsigned int pairs, double price_interval, double size_increment, double target_base_position, double position_divergence, double skew_factor, double trades_per_minute, double trade_rate_seconds);
  };

  class NewOrder : public Timestamped
  {
  public:
    std::string symbol;
    std::string clOrdID;
    double price;
    double orderQty;
    Side side;
    OrderType type;
    TimeInForce time_in_force;
    bool post_only;

    NewOrder(const std::string &symbol, std::string &&clOrdID, double price, double orderQty, Side side, OrderType type, TimeInForce time_in_force, Poco::DateTime time, bool post_only = true);
    json to_json() const;
  };

  class ReplaceOrder : public Timestamped
  {
  public:
    std::string origClOrdID;
    double price;
    double orderQty;

    ReplaceOrder(const std::string &origClOrdID, double price, double orderQty, Poco::DateTime time);
    json to_json() const;
  };

  class CancelOrder : public Timestamped
  {
  public:
    std::string clOrdID;

    CancelOrder(const std::string &clOrdID, Poco::DateTime time);
    json to_json() const;
  };

  // class BitmexOrder
  // {
  // public:
  //   std::string orderID = "";
  //   std::string clOrdID = "";
  //   std::string clOrdLinkID = "";
  //   long account = 0;
  //   std::string symbol = "XBTUSD";
  //   std::string side = "Buy";
  //   double price = 0;
  //   double orderQty = 0;
  //   double displayQty = 0;
  //   double stopPx = 0;
  //   double pegOffsetValue = 0;
  //   std::string pegPriceType = 0;
  //   std::string currency = "XBT";
  //   std::string settlCurrency = "";
  //   std::string ordType = "Limit";
  //   std::string timeInForce = "GoodTillCancel";
  //   std::string execInst = "ParticipateDoNotInitiate";
  //   std::string contingencyType = "";
  //   std::string exDestination = "";
  //   std::string ordStatus = "New";
  //   std::string triggered = "";
  //   bool workingIndicator = true;
  //   std::string ordRejReason = "";
  //   double leavesQty = 0;
  //   double cumQty = 0;
  //   double avgPx = 0;
  //   std::string multiLegReportingType = "";
  //   std::string text = "";
  //   Poco::DateTime transactTime;
  //   Poco::DateTime timestamp;
  // };

  class Trade : public Timestamped
  {
  public:
    std::string ID;
    Side side;
    double size;
    double price;
    Liquidity liquidity;
    double homeNotional;
    double foreignNotional;

    Trade(const std::string &&ID, Poco::DateTime &&time, Side side, double size, double price, Liquidity liquidity, double homeNotional, double foreignNotional);
  };

  class Skew : public Timestamped
  {
  public:
    double value;

    Skew(double value, Poco::DateTime &&time);
  };

  // class Position
  // {
  // public:
  //   std::string symbol;
  //   long currentQty;
  //   double homeNotional;
  //   double foreignNotioanl;
  //   double avgEntryPrice;
  //   double markPrice;
  //   double liquidationPrice;
  //   double realisedPnl;
  //   double unrealisedPnl;
  //   Poco::DateTime time;

  //   Position(const std::string &symbol, double homeNotional, double foreignNotional, double avgEntryPrice, double markPrice, double liquidationPrice, double unrealisedPnl, double realisedPnl, Poco::DateTime time);
  // };

  // class Margin
  // {
  // public:
  //   Currency currency;
  //   long amount;
  //   long availableMargin;
  //   long initMargin;
  //   long maintMargin;
  //   Poco::DateTime time;

  //   Margin(Currency currency, long amount, long availableMargin, long initMargin, long maintMargin, Poco::DateTime time);
  // };
} // namespace Models
