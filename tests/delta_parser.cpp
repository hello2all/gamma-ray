#include <iostream>
#include "doctest.h"
#include "json.hpp"
#include "delta_parser.h"

#define LOG(x) (std::cout << x << std::endl)

using json = nlohmann::json;

BitmexStore store;
BitmexDeltaParser delta_parser;

TEST_CASE("delta parser partial message")
{
  json partial = R"(
      {
        "table":"orderBookL2_25",
        "keys":["symbol","id","side"],
        "types":{"id":"long","price":"float","side":"symbol","size":"long","symbol":"symbol"},
        "foreignKeys":{"side":"side","symbol":"instrument"},
        "attributes":{"id":"sorted","symbol":"grouped"},
        "action":"partial",
        "data":[
          {"symbol":"XBTUSD","id":17999992000,"side":"Sell","size":100,"price":80},
          {"symbol":"XBTUSD","id":17999993000,"side":"Sell","size":20,"price":70},
          {"symbol":"XBTUSD","id":17999994000,"side":"Sell","size":10,"price":60},
          {"symbol":"XBTUSD","id":17999995000,"side":"Buy","size":10,"price":50},
          {"symbol":"XBTUSD","id":17999996000,"side":"Buy","size":20,"price":40},
          {"symbol":"XBTUSD","id":17999997000,"side":"Buy","size":100,"price":30}
        ]
      }
    )"_json;

  delta_parser.onAction(partial["action"], partial["table"], "XBTUSD", store, partial);

  CHECK(partial["data"] == store.data["orderBookL2_25"]["XBTUSD"]);
  CHECK(partial["keys"] == store.keys["orderBookL2_25"]["XBTUSD"]);
}

TEST_CASE("delta parser update message")
{
  json update = R"(
      {
      "table":"orderBookL2_25",
      "action":"update",
      "data":[
        {"symbol":"XBTUSD","id":17999995000,"side":"Buy","size":5}
      ]
    }
    )"_json;

  delta_parser.onAction(update["action"], update["table"], "XBTUSD", store, update);

  for (auto [key, value] : update["data"][0].items())
  {
    CHECK(value == store.data["orderBookL2_25"]["XBTUSD"][3][key]);
  }
}

TEST_CASE("delta parser delete")
{
  json remove = R"(
    {
      "table":"orderBookL2_25",
      "action":"delete",
      "data":[
        {"symbol":"XBTUSD","id":17999995000,"side":"Buy"}
      ]
    }
  )"_json;

  delta_parser.onAction(remove["action"], remove["table"], "XBTUSD", store, remove);

  CHECK(store.data["orderBookL2_25"]["XBTUSD"].size() == 5);
}

TEST_CASE("delta parser insert")
{
  json insert = R"(
    {
      "table":"orderBookL2_25",
      "action":"insert",
      "data":[
        {"symbol":"XBTUSD","id":17999995500,"side":"Buy","size":10,"price":45}
      ]
    }
  )"_json;

  delta_parser.onAction(insert["action"], insert["table"], "XBTUSD", store, insert);

  CHECK(store.data["orderBookL2_25"]["XBTUSD"].size() == 6);
  CHECK(store.data["orderBookL2_25"]["XBTUSD"][5] == insert["data"][0]);
}

TEST_CASE("delta parser trim second half when store exceeds max store length")
{
  json partial = R"(
  {
    "table":"quote",
    "action":"partial",
    "keys": [],
    "data":[
        {
          "buyPrice":9500,
          "buySize":5,
          "sellPrice":9500.5,
          "sellSize":10,
          "timestamp":"2017-04-04T22:16:38.472Z"
        }
    ]
  }
  )"_json;

  delta_parser.onAction(partial["action"], partial["table"], "XBTUSD", store, partial);

  json insert = R"(
    {
      "table":"quote",
      "action":"insert",
      "keys": [],
      "data":[
        {"buyPrice": 9500,"buySize": 5,"sellPrice": 9500.5,"sellSize":10,"timestamp": "2017-04-04T22:16:38.472Z"}
      ]
    }
  )"_json;

  for (unsigned int i = 0; i < delta_parser.MAX_LEN; i++)
  {
    delta_parser.onAction(insert["action"], insert["table"], "XBTUSD", store, insert);
  }

  CHECK((unsigned int)store.data["quote"]["XBTUSD"].size() == (delta_parser.MAX_LEN / 2));
}

TEST_CASE("delta parser do not trim if table are order book or order data")
{
  json partial = R"(
  {
    "table":"orderBookL2_25",
    "keys":["symbol","id","side"],
    "types":{"id":"long","price":"float","side":"symbol","size":"long","symbol":"symbol"},
    "foreignKeys":{"side":"side","symbol":"instrument"},
    "attributes":{"id":"sorted","symbol":"grouped"},
    "action":"partial",
    "data":[
      {"symbol":"XBTUSD","id":17999992000,"side":"Sell","size":100,"price":80},
      {"symbol":"XBTUSD","id":17999993000,"side":"Sell","size":20,"price":70},
      {"symbol":"XBTUSD","id":17999994000,"side":"Sell","size":10,"price":60},
      {"symbol":"XBTUSD","id":17999995000,"side":"Buy","size":10,"price":50},
      {"symbol":"XBTUSD","id":17999996000,"side":"Buy","size":20,"price":40},
      {"symbol":"XBTUSD","id":17999997000,"side":"Buy","size":100,"price":30}
    ]
  }
  )"_json;

  delta_parser.onAction(partial["action"], partial["table"], "XBTUSD", store, partial);

  json insert = R"(
    {
      "table":"orderBookL2_25",
      "action":"insert",
      "data":[
        {"symbol":"XBTUSD","id":17999995500,"side":"Buy","size":10,"price":45}
      ]
    }
  )"_json;

  for (unsigned int i = 0; i < delta_parser.MAX_LEN; i++)
  {
    delta_parser.onAction(insert["action"], insert["table"], "XBTUSD", store, insert);
  }

  CHECK((unsigned int)store.data["orderBookL2_25"]["XBTUSD"].size() >= (delta_parser.MAX_LEN));
}