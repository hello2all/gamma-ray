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

using json = nlohmann::json;

int main()
{
  // init config
  std::ifstream i("gamma-ray.json");
  json config;
  i >> config;
  std::string ws_uri = config["websocketUrl"].get<std::string>();
  std::string http_uri = config["httpUrl"].get<std::string>();
  std::string api_key = config["apiKey"].get<std::string>();
  std::string api_secret = config["apiSecret"].get<std::string>();
  std::string contract = config["contract"].get<std::string>();

  // init quoting styles
  std::vector<QuotingStrategies::QuotingStyle *> quoting_styles;
  double top_spread = config["quotingParam"]["quotingModeParam"]["Top"]["spread"].get<double>();
  QuotingStrategies::Top top(top_spread);
  quoting_styles.push_back(&top);

  double mid_spread = config["quotingParam"]["quotingModeParam"]["Mid"]["spread"].get<double>();
  QuotingStrategies::Mid mid(mid_spread);
  quoting_styles.push_back(&mid);

  double as_gamma = config["quotingParam"]["quotingModeParam"]["AvellanedaStoikov"]["gamma"].get<double>();
  double as_sigma = config["quotingParam"]["quotingModeParam"]["AvellanedaStoikov"]["sigma"].get<double>();
  double as_k = config["quotingParam"]["quotingModeParam"]["AvellanedaStoikov"]["k"].get<double>();
  QuotingStrategies::AvellanedaStoikov as(as_gamma, as_sigma, as_k);
  quoting_styles.push_back(&as);
  QuotingStrategies::QuotingStyleRegistry quoting_style_registry(quoting_styles);

  Models::QuotingMode mode = Models::get_quoting_mode(config["quotingParam"]["quotingMode"].get<std::string>());
  double size = config["quotingParam"]["size"].get<double>();
  double pairs = config["quotingParam"]["pairs"].get<unsigned int>();
  double interval = config["quotingParam"]["priceInterval"].get<double>();
  double size_inc = config["quotingParam"]["sizeIncrement"].get<double>();
  double tbp = config["quotingParam"]["targetBasePosition"].get<double>();
  double offset = config["quotingParam"]["positionOffset"].get<double>();
  double sk = config["quotingParam"]["skewFactor"].get<double>();
  double tpm = config["quotingParam"]["tradesPerMinute"].get<double>();
  double trs = config["quotingParam"]["tradeRateSeconds"].get<double>();
  Models::QuotingParameters params(mode, size, pairs, interval, size_inc, tbp, offset, sk, tpm, trs);

  BitmexRateLimit rl;
  BitmexDetailsGateway details(contract);
  BitmexWebsocket ws_cli(ws_uri, api_key, api_secret);
  BitmexHttp http_cli(http_uri, api_key, api_secret, rl);

  BitmexSymbolProvider symbol(contract);
  BitmexStore store;
  BitmexDeltaParser parser;
  BitmexMarketDataGateway md(ws_cli, symbol);
  BitmexOrderEntryGateway oe(http_cli, ws_cli, parser, store, symbol);
  BitmexPositionGateway pg(ws_cli, parser, store, symbol);

  MarketFiltration mf(md, oe, details);
  FairValue fv(mf, details);
  QuotingParameters qp(params);
  Skew skew(qp, pg, details);
  QuotingEngine engine(quoting_style_registry, fv, qp, mf, skew, oe, pg, details);
  QuoteDispatcher dispatcher(store, engine, oe, pg, rl, details, params.pairs);
  ws_cli.connect();
}