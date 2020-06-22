#pragma once

#include <string>
#include <map>
#include <functional>
#include "util.h"
#include "ws.h"
#include "json.hpp"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTaskAdapter.h"
#include "Poco/Timestamp.h"
#include "Poco/Logger.h"

using json = nlohmann::json;

class BitmexWebsocket
{
public:
  BitmexWebsocket();
  BitmexWebsocket(std::string uri);
  BitmexWebsocket(std::string uri, std::string api_key, std::string api_secret);
  ~BitmexWebsocket();

  void on_open(WS::OnOpenCB cb);
  void on_close(WS::OnCloseCB cb);
  void connect();
  void send(json j);
  void set_handler(const std::string, std::function<void(json)>);

private:
  WS ws;
  WS::OnOpenCB open_cb;
  WS::OnCloseCB close_cb;
  std::string uri = "wss://www.bitmex.com/realtime";
  std::string api_key = "";
  std::string api_secret = "";
  Poco::Util::Timer timer;
  Poco::Util::TimerTask::Ptr pPingTask;
  Poco::Timestamp last_pong_at = 0;
  std::map<std::string, std::function<void(json)>> handlers;
  Poco::Logger& logger = Poco::Logger::get("gr-bitmexws");

  std::string signed_url();
  void maintain_heartbeat();
  void init_heartbeat();
  void reset_heartbeat();
  void heartbeat(Poco::Util::TimerTask &task);
  void on_message(std::string raw);
};