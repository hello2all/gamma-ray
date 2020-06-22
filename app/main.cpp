// Executables must have the following defined if the library contains
// doctest definitions. For builds with this disabled, e.g. code shipped to
// users, this can be left out.
#ifdef ENABLE_DOCTEST_IN_LIBRARY
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#endif

#include <iostream>
#include <stdlib.h>
#include "config.h"
#include "json.hpp"
#include "bitmexws.h"
#include "util.h"
#include <future>
#include <chrono>
#include <thread>
#include "Poco/Environment.h"

using namespace std;
using json = nlohmann::json;

void waitToExe(BitmexWebsocket* cli) {
  cout << "wait to exe" << endl;
  this_thread::sleep_for(chrono::milliseconds(10000));
  json j = {{"op", "subscribe"}, {"args", {"trade:XBTUSD"}}};
  cli->send(j);
}

int main()
{
  cout << "POCO version: ";
  cout << static_cast<int>(Poco::Environment::libraryVersion() >> 24) << ".";
  cout << static_cast<int>((Poco::Environment::libraryVersion() >> 16) & 0xFF) << ".";
  cout << static_cast<int>((Poco::Environment::libraryVersion() >> 8) & 0xFF) << endl;

  string uri = "wss://testnet.bitmex.com/realtime";
  string api_key = "";
  string api_secret = "";
  BitmexWebsocket client(uri, api_key, api_secret);

  auto t1 = std::async(std::launch::async, waitToExe, &client);
  cout << "in b4 async end" << endl;

  client.on_open([](){
    cout << "opened" << endl;
  });

  client.set_handler("trade", [](json msg){
    cout << msg << endl;
  });

  client.connect();
  return 0;
}
