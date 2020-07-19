#include "doctest.h"
#include <vector>
#include "bitmexhttp.h"
#include <iostream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

string http_uri = "https://testnet.bitmex.com";
string api_key = "iCDc_MrgK2bfZOaRKhz5K99A";
string api_secret = "VJN9dBwrBInY2dY-SMVzDkb1suv9DQRwxmYKiEh7TcJVTg0w";

BitmexHttp http(http_uri, api_key, api_secret);

TEST_CASE("bitmex rest GET")
{
  const std::string path = "/api/v1/stats";
  const std::string verb = "GET";

  http.call(path, verb)
      .then([](json res) {
        CHECK(res.size() > 0);
      })
      .wait();
}

TEST_CASE("bitmex rest POST")
{
  const std::string path = "/api/v1/order/bulk";
  const std::string verb = "POST";

  json payload = R"(
  {
    "orders": [
      {
        "symbol": "XBTUSD",
        "side": "Buy",
        "orderQty": 25,
        "price": 9000,
        "clOrdID": "order1",
        "ordType": "Limit",
        "timeInForce": "GoodTillCancel",
        "execInst": "ParticipateDoNotInitiate"
      },
      {
        "symbol": "XBTUSD",
        "side": "Sell",
        "orderQty": 25,
        "price": 9500,
        "clOrdID": "order2",
        "ordType": "Limit",
        "timeInForce": "GoodTillCancel",
        "execInst": "ParticipateDoNotInitiate"
      }
    ]
  }
  )"_json;

  http.call(path, verb, payload)
      .then([](json res) {
        CHECK(res.size() == 2);
      })
      .wait();
}

TEST_CASE("bitmex rest PUT")
{
  const std::string path = "/api/v1/order/bulk";
  const std::string verb = "PUT";

  json payload = R"(
  {
    "orders":[
      {
        "origClOrdID":"order1",
        "orderQty":26,
        "price":9001
      },
      {
        "origClOrdID":"order2",
        "orderQty":27,
        "price":9501
      }
    ]
  }
  )"_json;

  http.call(path, verb, payload)
      .then([](json res) {
        CHECK(res.size() == 2);
      })
      .wait();
}

TEST_CASE("bitmex rest DELETE")
{
  const std::string path = "/api/v1/order";
  const std::string verb = "DELETE";

  json payload = R"(
  {
    "clOrdID": [
      "order1",
      "order2"
    ]
  }
  )"_json;

  http.call(path, verb, payload)
      .then([](json res) {
        CHECK(res.size() == 2);
      })
      .wait();
}
