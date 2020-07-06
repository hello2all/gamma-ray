#include "ws.h"

WS::WS()
{
  wsclient.set_access_channels(websocketpp::log::alevel::none);
  wsclient.set_error_channels(websocketpp::log::elevel::fatal);

  wsclient.init_asio();

  wsclient.set_open_handler([this](websocketpp::connection_hdl) {
    on_open_cb();
  });

  wsclient.set_message_handler(
      [this](websocketpp::connection_hdl, WSClient::message_ptr msg) {
        std::string raw = msg->get_raw_payload().c_str();
        on_message_cb(raw);
      });

  wsclient.set_close_handler([this](websocketpp::connection_hdl) {
    on_close_cb();
  });

  wsclient.set_interrupt_handler(
      [this](websocketpp::connection_hdl) { throw "Interrupt handler"; });

  wsclient.set_fail_handler(
      [this](websocketpp::connection_hdl) { throw "Fail handler"; });

  wsclient.set_tls_init_handler([](websocketpp::connection_hdl) {
    return websocketpp::lib::make_shared<boost::asio::ssl::context>(
        boost::asio::ssl::context::tlsv12);
  });
}

void WS::configure(std::string _uri)
{
  uri = _uri;
}

void WS::send(json j)
{
  wsclient.send(this->connection, j.dump(), websocketpp::frame::opcode::text);
}

void WS::send(const std::string& s) {
  wsclient.send(this->connection, s, websocketpp::frame::opcode::text);
}

void WS::set_on_open_cb(OnOpenCB open_cb)
{
  on_open_cb = open_cb;
}

void WS::set_on_close_cb(OnCloseCB close_cb)
{
  on_close_cb = close_cb;
}

void WS::set_on_message_cb(OnMessageCB message_cb)
{
  on_message_cb = message_cb;
}

void WS::connect()
{
  websocketpp::lib::error_code ec;
  connection = wsclient.get_connection(uri, ec);
  if (ec)
  {
    std::string err =
        "Could not create connection because: " + ec.message() + "\n";
    throw err;
  }
  wsclient.connect(connection);
  wsclient.run();
}

void WS::close()
{
  this->wsclient.close(this->connection, websocketpp::close::status::going_away, "");
}