#pragma once

#include <string>
#include <map>
#include <functional>
#include <future>
#include "util.h"
#include "models.h"
#include "ws.h"
#include "json.hpp"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTaskAdapter.h"
#include "Poco/Timestamp.h"
#include "Poco/Logger.h"
#include "Poco/BasicEvent.h"

using json = nlohmann::json;

class BitmexWebsocket
{
public:
  BitmexWebsocket();
  BitmexWebsocket(const std::string &uri);
  BitmexWebsocket(const std::string &uri, const std::string &api_key, const std::string &api_secret);
  ~BitmexWebsocket();

  Poco::BasicEvent<Models::ConnectivityStatus> connect_changed;

  void connect();
  void close();
  void send(json &j);
  void set_handler(const std::string &, std::function<void(json)>);

private:
  WS ws;
  std::string uri = "wss://www.bitmex.com/realtime";
  std::string api_key = "";
  std::string api_secret = "";
  Models::ConnectivityStatus connect_status = Models::ConnectivityStatus::disconnected;

  Poco::Util::Timer timer;
  Poco::Util::TimerTask::Ptr pPingTask;
  Poco::Timestamp last_pong_at = 0;

  std::map<std::string, std::function<void(json)>> handlers;
  Poco::Logger &logger = Poco::Logger::get("gr-bitmexws");

  long dead_mans_switch_update_interval = 15000; // milliseconds
  long cancel_all_delay = 60000;                 // milliseconds
  Poco::Util::Timer dead_mans_switch_timer;
  Poco::Util::TimerTask::Ptr pDeadSwitchTask;

  std::string signed_url();
  void maintain_heartbeat();
  void init_heartbeat();
  void reset_heartbeat();
  void heartbeat(Poco::Util::TimerTask &task);
  void on_message(const std::string &raw);
  void init_dead_mans_switch();
  void deadswitch(Poco::Util::TimerTask &task);
};