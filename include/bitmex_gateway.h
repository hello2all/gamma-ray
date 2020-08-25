#pragma once
#include <optional>
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "json.hpp"
#include "models.h"
#include "bitmexhttp.h"
#include "bitmexws.h"
#include "interfaces.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/DateTime.h"
#include "Poco/Timespan.h"
#include "delta_parser.h"

class BitmexSymbolProvider
{
public:
  std::string base = "XBt";
  std::string quote = "USD";
  std::string underlying = "XBt";
  std::string symbol = "XBTUSD";
  std::string symbol_with_type = "XBTUSD:Perpetual";

  BitmexSymbolProvider() = default;
  BitmexSymbolProvider(const std::string &c);
};

class BitmexMarketDataGateway : public Interfaces::IMarketDataGateway
{
private:
  BitmexWebsocket &ws;
  BitmexSymbolProvider &symbol;
  std::string order_book_channel = "orderBook10:XBTUSD";
  const std::string order_book_handle = "orderBook10";
  void subscribe_to_quote(const void *, Models::ConnectivityStatus &cs);

public:
  BitmexMarketDataGateway(BitmexWebsocket &ws, BitmexSymbolProvider &symbol);
  ~BitmexMarketDataGateway();
};

class BitmexOrderEntryGateway : public Interfaces::IOrderEntryGateway
{
private:
  BitmexHttp &http;
  BitmexWebsocket &ws;
  BitmexDeltaParser &parser;
  BitmexStore &store;
  BitmexSymbolProvider &symbol;
  std::string execution_channel = "execution:XBTUSD";
  const std::string execution_handle = "execution";
  std::string order_channel = "order:XBTUSD";
  const std::string order_handle = "order";
  Poco::UUIDGenerator id_generator = Poco::UUIDGenerator();

  std::optional<json> latest_position;
  std::optional<json> latest_margin;

  static Models::Liquidity decode_liquidity(const std::string &l);
  static Models::Side decode_side(const std::string &s);
  void subscribe(const void *, Models::ConnectivityStatus &cs);
  void on_order(const void *, json &order) override;
  void on_execution(const void *, json &execution) override;

public:
  BitmexOrderEntryGateway(BitmexHttp &http, BitmexWebsocket &ws, BitmexDeltaParser &parser, BitmexStore &store, BitmexSymbolProvider &symbol);
  ~BitmexOrderEntryGateway();

  std::string generate_client_id() override;

  void batch_send_order(std::vector<Models::NewOrder> &orders) override;
  void batch_cancel_order(std::vector<Models::CancelOrder> &cancels) override;
  void batch_replace_order(std::vector<Models::ReplaceOrder> &replaces) override;
  unsigned int cancel_all() override;
  std::optional<json> open_orders() override;
};

class BitmexPositionGateway : public Interfaces::IPositionGateway
{
private:
  BitmexWebsocket &ws;
  BitmexDeltaParser &parser;
  BitmexStore &store;
  BitmexSymbolProvider &symbol;
  std::string position_channel = "position:XBTUSD";
  const std::string position_handle = "position";
  const std::string margin_channel = "margin";
  const std::string margin_handle = "margin";

  void subscribe(const void *, Models::ConnectivityStatus &cs);
  void on_position(const void *, json &position) override;
  void on_margin(const void *, json &margin) override;

public:
  BitmexPositionGateway(BitmexWebsocket &ws, BitmexDeltaParser &parser, BitmexStore &store, BitmexSymbolProvider &symbol);
  ~BitmexPositionGateway();

  std::optional<json> get_latest_position() override;
  std::optional<json> get_latest_margin() override;
};

class BitmexRateLimit : public Interfaces::IRateLimitMonitor
{
private:
  bool is_limited = false;
  const float threshold = 0.05;
  int limit = 60;
  Poco::DateTime next_reset;

public:
  int remain = 60;
  BitmexRateLimit();
  ~BitmexRateLimit();

  void update_rate_limit(int limit, int remain, Poco::DateTime next_reset) override;
  bool is_rate_limited() override;
};

class BitmexDetailsGateway : public Interfaces::IExchangeDetailsGateway
{
public:
  BitmexDetailsGateway();
  BitmexDetailsGateway(const std::string &c);
};

class BitmexCombinedGateway
{
public:
  BitmexCombinedGateway(BitmexDetailsGateway &base, BitmexMarketDataGateway &md, BitmexOrderEntryGateway &oe, BitmexPositionGateway &pg, BitmexRateLimit &rl);

  BitmexDetailsGateway &base;
  BitmexMarketDataGateway &md;
  BitmexOrderEntryGateway &oe;
  BitmexPositionGateway &pg;
  BitmexRateLimit &rl;
};