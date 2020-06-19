#include "bitmexws.h"

BitmexWebsocket::BitmexWebsocket()
{
  ws.configure(this->uri);
}

BitmexWebsocket::BitmexWebsocket(std::string uri)
{
  this->uri = uri;
  ws.configure(this->uri);
}

BitmexWebsocket::BitmexWebsocket(std::string uri, std::string api_key, std::string api_secret)
{
  this->uri = uri;
  this->api_key = api_key;
  this->api_secret = api_secret;

  std::string signed_uri = this->signed_url();
  ws.configure(signed_uri);
}

void BitmexWebsocket::on_message(WS::OnMessageCB cb)
{
  ws.set_on_message_cb(cb);
}

void BitmexWebsocket::connect()
{
  ws.connect();
}

void BitmexWebsocket::send(json j) {
  ws.send(j);
}

std::string BitmexWebsocket::signed_url()
{
  std::string expires = std::to_string(util::get_seconds_timestamp(util::current_time()).count() + 60);
  std::string verb = "GET";
  std::string path = "/realtime";

  std::string data = verb + path + expires;
  std::string hmacced = util::encoding::hmac(std::string(api_secret), data, 32);
  std::string sign = util::encoding::string_to_hex((unsigned char *)hmacced.c_str(), 32);

  std::string signed_url = this->uri + "?api-expires=" + expires + "&api-signature=" + sign + "&api-key=" + this->api_key;
  return signed_url;
}

void BitmexWebsocket::on_open(WS::OnOpenCB cb)
{
  ws.set_on_open_cb(cb);
}