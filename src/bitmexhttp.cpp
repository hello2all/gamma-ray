#include "bitmexhttp.h"

BitmexHttp::BitmexHttp(Interfaces::IRateLimitMonitor &monitor)
    : client(web::http::client::http_client(web::uri(uri))), monitor(monitor)
{
}

BitmexHttp::BitmexHttp(const std::string &uri, Interfaces::IRateLimitMonitor &monitor)
    : client(web::http::client::http_client(web::uri(uri))), uri(uri), monitor(monitor)
{
}

BitmexHttp::BitmexHttp(const std::string &uri, const std::string &api_key, const std::string &api_secret, Interfaces::IRateLimitMonitor &monitor)
    : client(web::http::client::http_client(web::uri(uri))), uri(uri), api_key(api_key), api_secret(api_secret), monitor(monitor)
{
}

BitmexHttp::~BitmexHttp()
{
}

http_request BitmexHttp::build_request(const std::string &path, const std::string &verb, const std::string &body)
{
  std::string expires = std::to_string(util::get_seconds_timestamp(util::current_time()).count() + 60);
  std::string data = verb + path + expires + body;
  std::string sign = util::encoding::hmac(api_secret, data);

  http_request req(this->method_map.at(verb));

  req.set_request_uri(this->uri + path);

  req.headers().add("Content-Type", "application/json");
  req.headers().add("Accept", "application/json");
  req.headers().add("X-Requested-With", "XMLHttpRequest");
  req.headers().add("api-expires", expires);
  req.headers().add("api-key", this->api_key);
  req.headers().add("api-signature", sign);
  req.set_body(body);

  return req;
}

pplx::task<json> BitmexHttp::call(const std::string &path, const std::string &verb)
{
  std::string body = "";

  http_request req = this->build_request(path, verb, body);
  return this->client.request(req)
      .then([this](http_response response) {
        int limit = std::stoi(response.headers()["x-ratelimit-limit"]);
        int remain = std::stoi(response.headers()["x-ratelimit-remaining"]);
        Poco::DateTime next_reset(Poco::Timestamp::fromEpochTime(std::stoi(response.headers()["x-ratelimit-reset"])));
        this->monitor.update_rate_limit(limit, remain, next_reset);
        return response.extract_string();
      })
      .then([](std::string str_res) {
        return pplx::task_from_result(json::parse(str_res));
      });
}

pplx::task<json> BitmexHttp::call(const std::string &path, const std::string &verb, const json &payload)
{
  std::string body = payload.dump();

  http_request req = this->build_request(path, verb, body);
  return this->client.request(req)
      .then([this](http_response response) {
        int limit = std::stoi(response.headers()["x-ratelimit-limit"]);
        int remain = std::stoi(response.headers()["x-ratelimit-remaining"]);
        Poco::DateTime next_reset(Poco::Timestamp::fromEpochTime(std::stoi(response.headers()["x-ratelimit-reset"])));
        this->monitor.update_rate_limit(limit, remain, next_reset);
        return response.extract_string();
      })
      .then([](std::string str_res) {
        return pplx::task_from_result(json::parse(str_res));
      });
}
