#pragma once

#include "json.hpp"
#include <functional>
#include "websocketpp/client.hpp"
#include "websocketpp/config/asio_client.hpp"

using json = nlohmann::json;

class WS
{
  public:
    using WSClient = websocketpp::client<websocketpp::config::asio_tls_client>;
    using OnOpenCB = std::function<void()>;
    using OnCloseCB = std::function<void()>;
    using OnMessageCB = std::function<void(std::string raw)>;

    WS();
    void configure(std::string _uri);
    void set_on_open_cb(OnOpenCB open_cb);
    void set_on_close_cb(OnCloseCB close_cb);
    void set_on_message_cb(OnMessageCB message_cb);
    void connect();
    void close();
    void send(json j);
    void send(const std::string& s);

  private:
    WSClient wsclient;
    WSClient::connection_ptr connection;
    OnOpenCB on_open_cb = [](){};
    OnCloseCB on_close_cb = [](){};
    OnMessageCB on_message_cb;
    std::string uri;
};