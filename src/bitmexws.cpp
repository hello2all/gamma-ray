#include "bitmexws.h"

BitmexWebsocket::BitmexWebsocket()
{
  ws.configure(this->uri);
  this->pPingTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::heartbeat);
}

BitmexWebsocket::BitmexWebsocket(std::string uri)
{
  this->uri = uri;
  ws.configure(this->uri);
  this->pPingTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::heartbeat);
}

BitmexWebsocket::BitmexWebsocket(std::string uri, std::string api_key, std::string api_secret)
{
  this->uri = uri;
  this->api_key = api_key;
  this->api_secret = api_secret;

  std::string signed_uri = this->signed_url();
  ws.configure(signed_uri);
  this->pPingTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::heartbeat);
}

BitmexWebsocket::~BitmexWebsocket()
{
}

void BitmexWebsocket::on_message(WS::OnMessageCB cb)
{
  ws.set_on_message_cb([this, cb](json j) {
    cb(j);

    // init heartbeat on start up
    if (this->last_pong_at <= 0)
    {
      this->start_heartbeat();
      return;
    }
    // reset timer if a message is received within 5 seconds
    else if ((Poco::Timestamp() - this->last_pong_at) < 5e6)
    {
      std::cout << this->last_pong_at.epochMicroseconds() << "," << Poco::Timestamp().epochMicroseconds() << "," << ((Poco::Timestamp() - this->last_pong_at) < 5000000) << std::endl;
      this->reset_heartbeat();
      return;
    }
    // throw error if no messages are not received in 10 seconds
    if ((Poco::Timestamp() - this->last_pong_at) > 10e6)
    {
      throw std::runtime_error("network error");
    }

    // update last pong at
    this->last_pong_at = Poco::Timestamp();
  });
}

void BitmexWebsocket::connect()
{
  ws.connect();
}

void BitmexWebsocket::send(json j)
{
  ws.send(j);
}

std::string BitmexWebsocket::signed_url()
{
  std::string expires = std::to_string(util::get_seconds_timestamp(util::current_time()).count() + 60);
  std::string verb = "GET";
  std::string path = "/realtime";

  std::string data = verb + path + expires;
  std::string sign = util::encoding::hmac(std::string(api_secret), data);
  std::string signed_url = this->uri + "?api-expires=" + expires + "&api-signature=" + sign + "&api-key=" + this->api_key;

  return signed_url;
}

void BitmexWebsocket::on_open(WS::OnOpenCB cb)
{
  ws.set_on_open_cb(cb);
}

void BitmexWebsocket::on_close(WS::OnCloseCB cb)
{
  ws.set_on_close_cb(cb);
}

void BitmexWebsocket::start_heartbeat()
{
  this->last_pong_at = Poco::Timestamp();
  this->timer.schedule(this->pPingTask, Poco::Timestamp() + 5e6);
}

void BitmexWebsocket::reset_heartbeat()
{
  this->timer.cancel();
  this->last_pong_at = Poco::Timestamp();
  this->timer.schedule(this->pPingTask, Poco::Timestamp() + 5e6);
}

void BitmexWebsocket::heartbeat(Poco::Util::TimerTask &)
{
  std::string s = "ping";
  this->ws.send(s);
}