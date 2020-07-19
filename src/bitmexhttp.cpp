#include "bitmexhttp.h"

BitmexHttp::BitmexHttp()
    : client(web::http::client::http_client(web::uri(uri)))
{
}

BitmexHttp::BitmexHttp(const std::string &uri)
    : client(web::http::client::http_client(web::uri(uri))), uri(uri)
{
}

BitmexHttp::BitmexHttp(const std::string &uri, const std::string &api_key, const std::string &api_secret)
    : client(web::http::client::http_client(web::uri(uri))), uri(uri), api_key(api_key), api_secret(api_secret)
{
}

BitmexHttp::~BitmexHttp()
{
}

http_request BitmexHttp::build_request(const std::string &path, const std::string &verb, const std::string &body)
{
  std::string expires = std::to_string(util::get_seconds_timestamp(util::current_time()).count() + 60);
  std::string data = verb + path + expires + body;
  std::string sign = util::encoding::hmac(std::string(api_secret), data);

  http_request req(this->method_map.at(verb));

  req.set_request_uri(this->uri + path);

  req.headers().add("content-type", "application/json");
  req.headers().add("Accept", "application/json");
  req.headers().add("X-Requested-With", "XMLHttpRequest");
  req.headers().add("api-expires", expires);
  req.headers().add("api-key", this->api_key);
  req.headers().add("api-signature", sign);

  return req;
}

pplx::task<json> BitmexHttp::call(const std::string &path, const std::string &verb)
{
  std::string body = "";

  http_request req = this->build_request(path, verb, body);
  return this->client.request(req)
      .then([](http_response response) {
        if (response.status_code() != 200)
        {
          throw std::runtime_error("Returned " + std::to_string(response.status_code()));
        }
        return response.extract_string();
      })
      .then([](std::string str_res) {
        return json::parse(str_res);
      });
}

pplx::task<json> BitmexHttp::call(const std::string &path, const std::string &verb, const json &payload)
{
  std::string body = payload.dump();

  http_request req = this->build_request(path, verb, body);
  return this->client.request(req)
      .then([](http_response response) {
        if (response.status_code() != 200)
        {
          throw std::runtime_error("Returned " + std::to_string(response.status_code()));
        }
        return response.extract_string();
      })
      .then([](std::string str_res) {
        return json::parse(str_res);
      });
}
