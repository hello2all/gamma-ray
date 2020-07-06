#include <iostream>
#include <cstddef>
#include <cstdlib>
#include "doctest.h"
#include "json.hpp"
#include "bitmexws.h"
#include "Poco/Delegate.h"

#define LOG(x) (std::cout << x << std::endl)

using namespace std;
using json = nlohmann::json;

class QuoteListener
{
private:
  BitmexWebsocket &cli;

  void on_quote(json j)
  {
    // LOG(j.dump());
    CHECK(j["data"][0]["symbol"].get<string>() == "XBTUSD");
    this->cli.close();
  }

  void subscribe(const void *, Models::ConnectivityStatus &cs)
  {
    if (cs == Models::ConnectivityStatus::connected)
    {
      json j = {{"op", "subscribe"}, {"args", {"quote:XBTUSD"}}};
      cli.send(j);
    }
  };

public:
  QuoteListener(BitmexWebsocket &cli)
      : cli(cli)
  {
    cli.set_handler("quote", [this](json j) {
      this->on_quote(j);
    });
    cli.connect_changed += Poco::delegate(this, &QuoteListener::subscribe);
  }
  ~QuoteListener()
  {
    cli.connect_changed -= Poco::delegate(this, &QuoteListener::subscribe);
  }
};

TEST_CASE("Bitmex websocket with event trigger")
{
  string uri = "wss://testnet.bitmex.com/realtime";
  string api_key = "iCDc_MrgK2bfZOaRKhz5K99A";
  string api_secret = "VJN9dBwrBInY2dY-SMVzDkb1suv9DQRwxmYKiEh7TcJVTg0w";

  BitmexWebsocket ws_client(uri, api_key, api_secret);
  QuoteListener ql(ws_client);
  ws_client.connect();
}