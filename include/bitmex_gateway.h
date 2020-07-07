#pragma once
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "json.hpp"
#include "models.h"
#include "bitmexhttp.h"
#include "bitmexws.h"

class BitmexSymbolProdiver
{
public:
  std::string symbol = "XBTUSD";
  std::string symbol_with_type = "XBTUSD:Perpetual";
};

class BitmexMarketDataGateway
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

  Poco::BasicEvent<Models::MarketQuote> market_quote;
};

class BitmexOrderEntryGateway
{
public:
  BitmexOrderEntryGateway();
  ~BitmexOrderEntryGateway();
  void send_order();
  void cancel_order();
  void replace_order();

  void batch_send_order();
  void batch_cancel_order();
  void batch_replace_order();
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

class BitmexDetailsGateway
{
public:
  double maker_fee = -0.00025;
  double taker_fee = 0.00075;
  double min_tick_increment = 0.5;
  double min_size_increment = 1;
  double face_value = 1.0;
  unsigned int max_leverage = 100;
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