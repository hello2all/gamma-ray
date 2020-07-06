
#include <iostream>
#include "doctest.h"
#include "json.hpp"
#include "bitmexws.h"
#include "bitmex_gateway.h"
#include "Poco/Delegate.h"

#define LOG(x) (std::cout << x << std::endl)

using namespace std;
using json = nlohmann::json;

class MarketDataListener
{
private:
  BitmexMarketDataGateway &md;
  BitmexWebsocket &ws;
  BitmexSymbolProdiver &symbol;
  void on_market_quote(const void *, json &quote)
  {
    // LOG(quote.dump(2));
    CHECK(quote["data"][0]["symbol"] == symbol.symbol);
    ws.close();
  }

public:
  MarketDataListener(BitmexMarketDataGateway &md, BitmexWebsocket &ws, BitmexSymbolProdiver &symbol)
      : md(md), ws(ws), symbol(symbol)
  {
    md.market_quote += Poco::delegate(this, &MarketDataListener::on_market_quote);
  }

  ~MarketDataListener()
  {
  }
};

TEST_CASE("Bitmex market data gateway")
{
  string uri = "wss://testnet.bitmex.com/realtime";
  string api_key = "iCDc_MrgK2bfZOaRKhz5K99A";
  string api_secret = "VJN9dBwrBInY2dY-SMVzDkb1suv9DQRwxmYKiEh7TcJVTg0w";

  BitmexWebsocket ws_client(uri, api_key, api_secret);
  BitmexSymbolProdiver symbol;
  BitmexMarketDataGateway md(ws_client, symbol);
  MarketDataListener MDL(md, ws_client, symbol);

  ws_client.connect();
}