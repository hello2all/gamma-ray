#include "bitmex_gateway.h"

BitmexMarketDataGateway::BitmexMarketDataGateway(BitmexWebsocket &ws, BitmexSymbolProdiver &symbol)
    : ws(ws), symbol(symbol)
{
  this->quote_channel = "quote:" + symbol.symbol;
  ws.set_handler(this->quote_handle, [this](json quote) {
    Models::MarketQuote mq(quote["data"][0]);
    this->market_quote(this, mq);
  });
  ws.connect_changed += Poco::delegate(this, &BitmexMarketDataGateway::subscribe_to_quote);
}

BitmexMarketDataGateway::~BitmexMarketDataGateway()
{
  ws.connect_changed -= Poco::delegate(this, &BitmexMarketDataGateway::subscribe_to_quote);
}

void BitmexMarketDataGateway::subscribe_to_quote(const void *, Models::ConnectivityStatus &cs)
{
  if (cs == Models::ConnectivityStatus::connected)
  {
    json j = {{"op", "subscribe"}, {"args", {this->quote_channel}}};
    ws.send(j);
  }
}

BitmexOrderEntryGateway::BitmexOrderEntryGateway()
{
}

BitmexOrderEntryGateway::~BitmexOrderEntryGateway()
{
}

BitmexPositionGateway::BitmexPositionGateway()
{
}

BitmexPositionGateway::~BitmexPositionGateway()
{
}

BitmexRateLimit::BitmexRateLimit()
{
}

BitmexRateLimit::~BitmexRateLimit()
{
}

BitmexDetailsGateway::BitmexDetailsGateway()
{
  this->maker_fee = -0.00025;
  this->taker_fee = 0.00075;
  this->min_tick_increment = 0.5;
  this->min_size_increment = 1;
  this->face_value = 1.0;
  this->max_leverage = 100;
}

BitmexDetailsGateway::~BitmexDetailsGateway()
{
}

BitmexCombinedGateway::BitmexCombinedGateway(BitmexDetailsGateway &base, BitmexMarketDataGateway &md, BitmexOrderEntryGateway &oe, BitmexPositionGateway &pg, BitmexRateLimit &rl)
    : base(base), md(md), oe(oe), pg(pg), rl(rl)
{
}

BitmexCombinedGateway::~BitmexCombinedGateway()
{
}