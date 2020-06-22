#pragma once

#include <string>
#include <vector>
#include <functional>
#include "util.h"
#include "ws.h"
#include "json.hpp"
#include "Poco/Util/Timer.h"
#include "Poco/Util/TimerTaskAdapter.h"
#include "Poco/Timestamp.h"

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
  Poco::Util::Timer timer;
  Poco::Util::TimerTask::Ptr pPingTask;
  Poco::Timestamp last_pong_at = 0;

  std::string signed_url();
  void start_heartbeat();
  void reset_heartbeat();
  void heartbeat(Poco::Util::TimerTask& task);
};