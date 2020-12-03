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

  QuotingMode get_quoting_mode(const std::string &s)
  {
    auto it = quoting_mode_table.find(s);
    if (it != quoting_mode_table.end())
    {
      return it->second;
    }
    else
    {
      throw std::runtime_error("Invalid quoting mode");
    }
  }

  Timestamped::Timestamped()
  {
  }

  Timestamped::Timestamped(Poco::DateTime &&time)
      : time(std::move(time))
  {
  }

  Timestamped::Timestamped(const Poco::DateTime &time)
      : time(time)
  {
  }

  MarketQuote::MarketQuote(const json &q, bool use_server_time)
  {
    this->askPrice = q["askPrice"].get<double>();
    this->askSize = q["askSize"].get<double>();
    this->bidPrice = q["bidPrice"].get<double>();
    this->bidSize = q["bidSize"].get<double>();
    this->symbol = q["symbol"].get<std::string>();
    if (use_server_time)
      this->time = iso8601_to_datetime(q["timestamp"].get<std::string>());
    else
      this->time = Poco::DateTime();
  }

  MarketQuote::MarketQuote(double askPrice, double askSize, double bidPrice, double bidSize, const std::string &symbol, Poco::DateTime &&time)
      : Timestamped{std::move(time)}, askPrice(askPrice), askSize(askSize), bidPrice(bidPrice), bidSize(bidSize), symbol(symbol)
  {
  }

  Quote::Quote(double price, double size, Side side)
      : price(price), size(size), side(side)
  {
  }

  TwoSidedQuote::TwoSidedQuote(std::vector<Quote> bids, std::vector<Quote> asks, Poco::DateTime time)
      : Timestamped{std::move(time)}, bids(bids), asks(asks)
  {
  }

  FairValue::FairValue(double price, Poco::DateTime time)
      : Timestamped{std::move(time)}, price(price)
  {
  }

  QuotingParameters::QuotingParameters(QuotingMode mode, double size, unsigned int pairs, double price_interval, double size_increment, double target_base_position, double position_divergence, double skew_factor, double trades_per_minute, double trade_rate_seconds)
      : mode(mode), size(size), pairs(pairs), price_interval(price_interval), size_increment(size_increment), target_base_position(target_base_position), position_divergence(position_divergence), skew_factor(skew_factor), trades_per_minute(trades_per_minute), trade_rate_seconds(trade_rate_seconds)
  {
  }

  NewOrder::NewOrder(const std::string &symbol, std::string &&clOrdID, double price, double orderQty, Side side, OrderType type, TimeInForce time_in_force, const Poco::DateTime time, bool post_only)
      : Timestamped{std::move(time)}, symbol(symbol), clOrdID(std::move(clOrdID)), price(price), orderQty(orderQty), side(side), type(type), time_in_force(time_in_force), post_only(post_only)
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
      j["execInst"] = "ParticipateDoNotInitiate";

    return j;
  }

  ReplaceOrder::ReplaceOrder(const std::string &origClOrdID, double price, double orderQty, Poco::DateTime time)
      : Timestamped{std::move(time)}, origClOrdID(origClOrdID), price(price), orderQty(orderQty)
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
      : Timestamped{std::move(time)}, clOrdID(clOrdID)
  {
  }

  json CancelOrder::to_json() const
  {
    json j = {
        {"cliOrdID", clOrdID}};

    return j;
  }

  Trade::Trade(const std::string &&ID, Poco::DateTime &&time, Side side, double size, double price, Liquidity liquidity, double homeNotional, double foreignNotional)
      : Timestamped{std::move(time)}, ID(std::move(ID)), side(side), size(size), price(price), liquidity(liquidity), homeNotional(homeNotional), foreignNotional(foreignNotional)
  {
  }

  Skew::Skew(double value, Poco::DateTime &&time)
      : Timestamped{std::move(time)}, value(value)
  {
  }

  // Position::Position(const std::string &symbol, double homeNotional, double foreignNotional, double avgEntryPrice, double markPrice, double liquidationPrice, double unrealisedPnl, double realisedPnl, Poco::DateTime time)
  //     : symbol(symbol), homeNotional(homeNotional), foreignNotioanl(foreignNotioanl), avgEntryPrice(avgEntryPrice), markPrice(markPrice), liquidationPrice(liquidationPrice), unrealisedPnl(unrealisedPnl), realisedPnl(realisedPnl), time(time)
  // {
  // }

  // Margin::Margin(Currency currency, long amount, long availableMargin, long initMargin, long maintMargin, Poco::DateTime time)
  //     : currency(currency), amount(amount), availableMargin(availableMargin), initMargin(initMargin), maintMargin(maintMargin), time(time)
  // {
  // }
} // namespace Models