#pragma once
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "json.hpp"
#include "models.h"
#include "bitmexhttp.h"
#include "bitmexws.h"
#include "interfaces.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/DateTime.h"
#include "delta_parser.h"

class BitmexSymbolProdiver
{
public:
  std::string symbol = "XBTUSD";
  std::string symbol_with_type = "XBTUSD:Perpetual";
};

class BitmexMarketDataGateway : public Interfaces::IMarketDataGateway
{
private:
  BitmexWebsocket &ws;
  BitmexSymbolProdiver &symbol;
  std::string quote_channel = "quote:XBTUSD";
  const std::string quote_handle = "quote";
  void subscribe_to_quote(const void *, Models::ConnectivityStatus &cs);

public:
  BitmexMarketDataGateway(BitmexWebsocket &ws, BitmexSymbolProdiver &symbol);
  ~BitmexMarketDataGateway();
};

class BitmexOrderEntryGateway : public Interfaces::IOrderEntryGateway
{
private:
  BitmexHttp &http;
  BitmexWebsocket &ws;
  BitmexDeltaParser &parser;
  BitmexStore &store;
  BitmexSymbolProdiver &symbol;
  std::string execution_channel = "execution:XBTUSD";
  const std::string execution_handle = "execution";
  std::string order_channel = "order:XBTUSD";
  const std::string order_handle = "order";
  Poco::UUIDGenerator id_generator = Poco::UUIDGenerator();

  static Models::Liquidity decode_liquidity(const std::string &l);
  static Models::Side decode_side(const std::string &s);
  void subscribe(const void *, Models::ConnectivityStatus &cs);
  void on_order(const void *, json &order) override;
  void on_execution(const void *, json &execution) override;

public:
  BitmexOrderEntryGateway(BitmexHttp &http, BitmexWebsocket &ws, BitmexDeltaParser &parser, BitmexStore &store, BitmexSymbolProdiver &symbol);
  ~BitmexOrderEntryGateway();

  std::string generate_client_id() override;

  void batch_send_order(std::vector<Models::NewOrder> orders) override;
  void batch_cancel_order(std::vector<Models::CancelOrder> cancels) override;
  void batch_replace_order(std::vector<Models::ReplaceOrder> replaces) override;
  unsigned int cancel_all() override;

  Poco::BasicEvent<Models::Trade> trade;
};

class BitmexPositionGateway
{
public:
  BitmexPositionGateway();
  ~BitmexPositionGateway();
  Poco::BasicEvent<json> position_update;
};

class BitmexRateLimit
{
public:
  BitmexRateLimit();
  ~BitmexRateLimit();
  Poco::BasicEvent<json> rate_limit_update;
};

class BitmexDetailsGateway : public Interfaces::IExchangeDetailsGateway
{
public:
  BitmexDetailsGateway();
  ~BitmexDetailsGateway();
};

class BitmexCombinedGateway
{
public:
  BitmexCombinedGateway(BitmexDetailsGateway &base, BitmexMarketDataGateway &md, BitmexOrderEntryGateway &oe, BitmexPositionGateway &pg, BitmexRateLimit &rl);
  ~BitmexCombinedGateway();

  BitmexDetailsGateway &base;
  BitmexMarketDataGateway &md;
  BitmexOrderEntryGateway &oe;
  BitmexPositionGateway &pg;
  BitmexRateLimit &rl;
};