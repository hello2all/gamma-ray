#pragma once

#include <iostream>
#include <map>
#include "json.hpp"
#include <cpprest/http_client.h>
#include "util.h"

using namespace web::http;         // Common HTTP functionality
using namespace web::http::client; // HTTP client features
using json = nlohmann::json;

class BitmexHttp
{
public:
  BitmexHttp();
  BitmexHttp(const std::string &uri);
  BitmexHttp(const std::string &uri, const std::string &api_key, const std::string &api_secret);
  ~BitmexHttp();
  pplx::task<json> call(const std::string &path, const std::string &verb);
  pplx::task<json> call(const std::string &path, const std::string &verb, const json &payload);

private:
  http_client client;
  std::string uri = "https://www.bitmex.com/api/v1";
  std::string api_key = "";
  std::string api_secret = "";
  const std::map<std::string, method> method_map = {
      {"GET", methods::GET},
      {"PUT", methods::PUT},
      {"POST", methods::POST},
      {"DELETE", methods::DEL},
  };
  http_request build_request(const std::string &path, const std::string &verb, const std::string &body);
};