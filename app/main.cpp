// Executables must have the following defined if the library contains
// doctest definitions. For builds with this disabled, e.g. code shipped to
// users, this can be left out.
#ifdef ENABLE_DOCTEST_IN_LIBRARY
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#endif

#include <iostream>
#include "config.h"
#include "bitmexws.h"
#include "bitmexhttp.h"
#include "bitmex_gateway.h"
#include "delta_parser.h"
#include "fair_value.h"
#include "market_filtration.h"
#include "models.h"
#include "quote_dispatcher.h"
#include "quoting_engine.h"
#include "quoting_parameters.h"
#include "quoting_strategies.h"
#include "skew.h"

int main()
{
  std::string ws_uri = "wss://testnet.bitmex.com/realtime";
  std::string http_uri = "https://testnet.bitmex.com";
  std::string api_key = "iCDc_MrgK2bfZOaRKhz5K99A";
  std::string api_secret = "VJN9dBwrBInY2dY-SMVzDkb1suv9DQRwxmYKiEh7TcJVTg0w";
  Models::QuotingParameters params(Models::QuotingMode::Top, 0.5, 25.0, 0, 100.0, 1.0, 5.0, 300.0);

  QuotingStrategies::Top top;
  std::vector<QuotingStrategies::QuotingStyle *> quoting_styles;
  quoting_styles.push_back(&top);
  QuotingStrategies::QuotingStyleRegistry quoting_style_registry(quoting_styles);

  BitmexRateLimit rl;
  BitmexDetailsGateway details;
  BitmexWebsocket ws_cli(ws_uri, api_key, api_secret);
  BitmexHttp http_cli(http_uri, api_key, api_secret, rl);

  BitmexSymbolProdiver symbol;
  BitmexStore store;
  BitmexDeltaParser parser;
  BitmexMarketDataGateway md(ws_cli, symbol);
  BitmexOrderEntryGateway oe(http_cli, ws_cli, parser, store, symbol);
  BitmexPositionGateway pg(ws_cli, parser, store, symbol);

  MarketFiltration mf(md, oe, details);
  FairValue fv(mf, details);
  QuotingParameters qp(params);
  Skew skew(qp, pg, details);
  QuotingEngine engine(quoting_style_registry, fv, qp, mf, skew, oe, details);
  QuoteDispatcher dispatcher(store, engine, oe, pg, rl, details);
  ws_cli.connect();
}