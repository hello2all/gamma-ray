#pragma once

#include <string>
#include <vector>
#include "util.h"
#include "ws.h"
#include "json.hpp"

using json = nlohmann::json;

class BitmexWebsocket
{
  public:
    BitmexWebsocket();
    BitmexWebsocket(std::string uri);
    BitmexWebsocket(std::string uri, std::string api_key, std::string api_secret);

    void on_open(WS::OnOpenCB cb);
    void on_close(WS::OnCloseCB cb);
    void on_message(WS::OnMessageCB cb);
    void connect();
    void send(json j);

  private:
    WS::OnOpenCB open_cb;
    WS::OnCloseCB close_cb;
    WS::OnMessageCB message_cb;
    WS ws;
    std::string uri = "wss://www.bitmex.com/realtime";
    std::string api_key = "";
    std::string api_secret = "";

    std::string signed_url();
};