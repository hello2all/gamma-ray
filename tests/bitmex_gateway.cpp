#include <iostream>
#include <unistd.h>
#include <future>
#include "doctest.h"
#include "json.hpp"
#include "bitmexws.h"
#include "bitmex_gateway.h"
#include "bitmexhttp.h"
#include "models.h"
#include "delta_parser.h"
#include "Poco/Delegate.h"
#include "Poco/DateTime.h"

#define LOG(x) (std::cout << x << std::endl)

using namespace std;
using json = nlohmann::json;

class MarketDataListener
{
private:
  BitmexMarketDataGateway &md;
  BitmexWebsocket &ws;
  BitmexSymbolProvider &symbol;
  void on_market_quote(const void *, Models::MarketQuote &quote)
  {
    // LOG(quote.symbol);
    CHECK(quote.symbol == symbol.symbol);
    ws.close();
  }

public:
  MarketDataListener(BitmexMarketDataGateway &md, BitmexWebsocket &ws, BitmexSymbolProvider &symbol)
      : md(md), ws(ws), symbol(symbol)
  {
    md.market_quote += Poco::delegate(this, &MarketDataListener::on_market_quote);
  }

  ~MarketDataListener()
  {
    md.market_quote -= Poco::delegate(this, &MarketDataListener::on_market_quote);
  }
};

class OrderEntryListener
{
private:
  BitmexOrderEntryGateway &oe;
  BitmexWebsocket &ws;

  void on_trade(const void *, Models::Trade &trade)
  {
    CHECK(trade.size == 25);
    this->ws.close();
  }

public:
  OrderEntryListener(BitmexOrderEntryGateway &oe, BitmexWebsocket &ws)
      : oe(oe), ws(ws)
  {
    oe.trade += Poco::delegate(this, &OrderEntryListener::on_trade);
  }

  ~OrderEntryListener()
  {
    oe.trade -= Poco::delegate(this, &OrderEntryListener::on_trade);
  }
};

TEST_CASE("Bitmex market data gateway")
{
  string uri = "wss://testnet.bitmex.com/realtime";
  string api_key = "iCDc_MrgK2bfZOaRKhz5K99A";
  string api_secret = "VJN9dBwrBInY2dY-SMVzDkb1suv9DQRwxmYKiEh7TcJVTg0w";

  BitmexWebsocket ws_client(uri, api_key, api_secret);
  BitmexSymbolProvider symbol;
  BitmexMarketDataGateway md(ws_client, symbol);
  MarketDataListener MDL(md, ws_client, symbol);

  ws_client.connect();
}

TEST_CASE("Bitmex order entry gateway")
{
  string ws_uri = "wss://testnet.bitmex.com/realtime";
  string http_uri = "https://testnet.bitmex.com";
  string api_key = "iCDc_MrgK2bfZOaRKhz5K99A";
  string api_secret = "VJN9dBwrBInY2dY-SMVzDkb1suv9DQRwxmYKiEh7TcJVTg0w";

  BitmexWebsocket ws_cli(ws_uri, api_key, api_secret);
  BitmexHttp http_cli(http_uri, api_key, api_secret);
  BitmexSymbolProvider symbol;
  BitmexStore store;
  BitmexDeltaParser parser;
  BitmexOrderEntryGateway oe(http_cli, ws_cli, parser, store, symbol);
  OrderEntryListener OEL(oe, ws_cli);

  auto handle = std::async(std::launch::async, [&ws_cli]() {
    ws_cli.connect();
  });

  // get middle price
  std::string path = "/api/v1/orderBook/L2?symbol=XBT";
  std::string verb = "GET";
  double mid = 0;
  http_cli.call(path, verb)
      .then([&mid](json res) {
        mid = (res[24]["price"].get<double>() + res[25]["price"].get<double>()) / 2;
      })
      .wait();

  // cancel all open orders
  unsigned int opens = oe.cancel_all();

  std::cout << "Bitmex order entry gateway canceled " << opens << " existing orders " << std::endl;

  std::string clOrdID1 = "order1";
  std::string clOrdID2 = "order2";

  Models::NewOrder order1(symbol.symbol, clOrdID1, mid + 100.25, 25.0, Models::Side::Ask, Models::OrderType::Limit, Models::TimeInForce::GTC, Poco::DateTime(), true);
  Models::NewOrder order2(symbol.symbol, clOrdID2, mid - 100.25, 25.0, Models::Side::Bid, Models::OrderType::Limit, Models::TimeInForce::GTC, Poco::DateTime(), true);
  std::vector<Models::NewOrder> orders = {order1, order2};

  Models::ReplaceOrder replace1(clOrdID1, mid + 50.25, 26.0, Poco::DateTime());
  Models::ReplaceOrder replace2(clOrdID2, mid - 50.25, 27.0, Poco::DateTime());
  std::vector<Models::ReplaceOrder> replaces = {replace1, replace2};

  Models::CancelOrder cancel1(clOrdID1, Poco::DateTime());
  Models::CancelOrder cancel2(clOrdID2, Poco::DateTime());
  std::vector<Models::CancelOrder> cancels = {cancel1, cancel2};

  // test send orders
  oe.batch_send_order(orders);
  sleep(3);
  CHECK(store.data["order"]["XBTUSD"].size() == 2);

  // test replace orders
  oe.batch_replace_order(replaces);
  sleep(3);
  CHECK(store.data["order"]["XBTUSD"][0]["price"].get<double>() == mid + 50.25);
  CHECK(store.data["order"]["XBTUSD"][0]["orderQty"].get<double>() == 26);
  CHECK(store.data["order"]["XBTUSD"][1]["price"].get<double>() == mid - 50.25);
  CHECK(store.data["order"]["XBTUSD"][1]["orderQty"].get<double>() == 27);

  // test cancel orders
  oe.batch_cancel_order(cancels);
  sleep(3);
  CHECK(store.data["order"]["XBTUSD"].size() == 2);

  // test trade
  std::string clOrdID3 = "order3";
  // Models::NewOrder order3(symbol.symbol, clOrdID3, mid - 200.25, 25.0, Models::Side::Ask, Models::OrderType::Limit, Models::TimeInForce::GTC, Poco::DateTime(), false);
  Models::NewOrder order3(symbol.symbol, clOrdID3, mid + 200.25, 25.0, Models::Side::Bid, Models::OrderType::Limit, Models::TimeInForce::GTC, Poco::DateTime(), false);
  std::vector<Models::NewOrder> orders2 = {order3};
  oe.batch_send_order(orders2);

  handle.get();
}

TEST_CASE("Bitmex position gateway")
{
  string ws_uri = "wss://testnet.bitmex.com/realtime";
  string http_uri = "https://testnet.bitmex.com";
  string api_key = "iCDc_MrgK2bfZOaRKhz5K99A";
  string api_secret = "VJN9dBwrBInY2dY-SMVzDkb1suv9DQRwxmYKiEh7TcJVTg0w";

  BitmexWebsocket ws_cli(ws_uri, api_key, api_secret);
  BitmexHttp http_cli(http_uri, api_key, api_secret);
  BitmexSymbolProvider symbol;
  BitmexStore store;
  BitmexDeltaParser parser;

  BitmexOrderEntryGateway oe(http_cli, ws_cli, parser, store, symbol);
  BitmexPositionGateway pg(ws_cli, parser, store, symbol);

  auto handle = std::async(std::launch::async, [&ws_cli]() {
    ws_cli.connect();
  });

  bool haveValues = false;
  while (!haveValues)
  {
    auto pos = pg.get_latest_position();
    auto margin = pg.get_latest_margin();
    if (pos && margin)
      haveValues = true;
    else
      sleep(1);
  }

  auto pos = pg.get_latest_position();
  auto margin = pg.get_latest_margin();
  CHECK(pos.value().contains("currentQty"));
  CHECK(margin.value().contains("amount"));
  ws_cli.close();
}

TEST_CASE("bitmex rate limit monitor")
{
  string http_uri = "https://testnet.bitmex.com";
  string api_key = "iCDc_MrgK2bfZOaRKhz5K99A";
  string api_secret = "VJN9dBwrBInY2dY-SMVzDkb1suv9DQRwxmYKiEh7TcJVTg0w";

  BitmexRateLimit monitor;
  BitmexHttp http_cli(http_uri, api_key, api_secret, monitor);

  const std::string path = "/api/v1/stats";
  const std::string verb = "GET";

  int remain;
  http_cli.call(path, verb)
      .then([](json) {
      })
      .wait();
  remain = monitor.remain;

  http_cli.call(path, verb)
      .then([](json) {
      })
      .wait();

  CHECK((remain - monitor.remain) == 1);
  CHECK(monitor.is_rate_limited() == false);

  monitor.update_rate_limit(60, 11, Poco::DateTime());

  CHECK(monitor.is_rate_limited() == true);
}