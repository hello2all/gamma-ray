#include "bitmex_gateway.h"

BitmexMarketDataGateway::BitmexMarketDataGateway(BitmexWebsocket &ws, BitmexSymbolProdiver &symbol)
    : ws(ws), symbol(symbol)
{
  this->quote_channel = "quote:" + symbol.symbol;
  ws.set_handler(this->quote_handle, [this](json quote) {
    this->market_quote(this, quote);
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

BitmexCombinedGateway::BitmexCombinedGateway(BitmexDetailsGateway &base, BitmexMarketDataGateway &md, BitmexOrderEntryGateway &oe, BitmexPositionGateway &pg, BitmexRateLimit &rl)
    : base(base), md(md), oe(oe), pg(pg), rl(rl)
{
}

BitmexCombinedGateway::~BitmexCombinedGateway()
{
}