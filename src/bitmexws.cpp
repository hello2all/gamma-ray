#include "bitmexws.h"

BitmexWebsocket::BitmexWebsocket()
{
  ws.configure(this->uri);
  this->pPingTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::heartbeat);
  this->pDeadSwitchTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::deadswitch);
  ws.set_on_message_cb([this](json msg) { this->on_message(msg); });
  ws.set_on_open_cb([this]() {
    this->init_dead_mans_switch();
    this->connect_status = Models::ConnectivityStatus::connected;
    this->connect_changed(this, this->connect_status);
  });
  ws.set_on_close_cb([this]() {
    this->connect_status = Models::ConnectivityStatus::disconnected;
    this->connect_changed(this, this->connect_status);
  });
}

BitmexWebsocket::BitmexWebsocket(const std::string &uri)
    : uri(uri)
{
  ws.configure(this->uri);
  this->pPingTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::heartbeat);
  this->pDeadSwitchTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::deadswitch);
  ws.set_on_message_cb([this](json msg) { this->on_message(msg); });
  ws.set_on_open_cb([this]() {
    this->init_dead_mans_switch();
    this->connect_status = Models::ConnectivityStatus::connected;
    this->connect_changed(this, this->connect_status);
  });
  ws.set_on_close_cb([this]() {
    this->connect_status = Models::ConnectivityStatus::disconnected;
    this->connect_changed(this, this->connect_status);
  });
}

BitmexWebsocket::BitmexWebsocket(const std::string &uri, const std::string &api_key, const std::string &api_secret)
    : uri(uri), api_key(api_key), api_secret(api_secret)
{
  std::string signed_uri = this->signed_url();
  ws.configure(signed_uri);
  this->pPingTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::heartbeat);
  this->pDeadSwitchTask = new Poco::Util::TimerTaskAdapter<BitmexWebsocket>(*this, &BitmexWebsocket::deadswitch);
  ws.set_on_message_cb([this](json msg) { this->on_message(msg); });
  ws.set_on_open_cb([this]() {
    this->init_dead_mans_switch();
    this->connect_status = Models::ConnectivityStatus::connected;
    this->connect_changed(this, this->connect_status);
  });
  ws.set_on_close_cb([this]() {
    this->connect_status = Models::ConnectivityStatus::disconnected;
    this->connect_changed(this, this->connect_status);
  });
}

BitmexWebsocket::~BitmexWebsocket()
{
}

void BitmexWebsocket::on_message(const std::string &raw)
{
  (void)std::async(std::launch::async, [this]() { this->maintain_heartbeat(); });
  if (raw == "pong")
  {
    return;
  }

  json msg = json::parse(raw);

  if (msg.contains("table"))
  {
    if (this->handlers.count(msg["table"].get<std::string>()))
    {
      // dispatch handler
      this->handlers[msg["table"].get<std::string>()](msg);
    }
    else
    {
      this->logger.warning("Get message on un-specified table");
    }
    return;
  }
  if (msg.contains("error"))
  {
    this->logger.warning(msg["error"]);
    return;
  }
  if (msg.contains("subscribe"))
  {
    this->logger.information("Subscribe to " + msg["subscribe"].get<std::string>() + " " + (msg["success"].get<bool>() ? "success" : "fail"));
    return;
  }
}

void BitmexWebsocket::set_handler(const std::string &h_name, std::function<void(json)> handler)
{
  this->handlers.insert({h_name, handler});
}

void BitmexWebsocket::connect()
{
  ws.connect();
}

void BitmexWebsocket::close()
{
  ws.close();
}

void BitmexWebsocket::send(json &msg)
{
  ws.send(msg);
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

// maintain constant ping to verify connection
void BitmexWebsocket::maintain_heartbeat()
{
  // init heartbeat on start up
  if (this->last_pong_at <= 0)
  {
    this->init_heartbeat();
    return;
  }
  // reset timer if a message is received within 5 seconds
  else if ((Poco::Timestamp() - this->last_pong_at) < 5e6)
  {
    this->reset_heartbeat();
    return;
  }
  // throw error if no messages are not received in 10 seconds
  if ((Poco::Timestamp() - this->last_pong_at) > 10e6)
  {
    this->logger.error("Connection to bitmex interrupted");
    throw std::runtime_error("network error");
  }

  // update last pong at
  this->last_pong_at = Poco::Timestamp();
}

void BitmexWebsocket::init_heartbeat()
{
  this->last_pong_at = Poco::Timestamp();
  this->timer.schedule(this->pPingTask, Poco::Timestamp() + 5e6);
}

void BitmexWebsocket::reset_heartbeat()
{
  this->timer.cancel();
  this->last_pong_at = Poco::Timestamp();
  this->timer.schedule(this->pPingTask, Poco::Timestamp() + 3e6);
}

void BitmexWebsocket::heartbeat(Poco::Util::TimerTask &)
{
  std::string s = "ping";
  this->ws.send(s);
}

void BitmexWebsocket::init_dead_mans_switch()
{
  this->dead_mans_switch_timer.schedule(this->pDeadSwitchTask, 0, this->dead_mans_switch_update_interval);
}

void BitmexWebsocket::deadswitch(Poco::Util::TimerTask &)
{
  json j = {
      {"op", "cancelAllAfter"},
      {"args", this->cancel_all_delay}};

  this->ws.send(j);
}
