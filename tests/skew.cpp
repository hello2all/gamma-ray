#include "doctest.h"
#include "json.hpp"
#include "models.h"
#include "skew.h"
#include "quoting_parameters.h"
#include "bitmex_gateway.h"

using json = nlohmann::json;

class TestPositionGateway : public Interfaces::IPositionGateway
{
private:
  int counter = 0;
  void on_position(const void *, json &) override
  {
  }
  void on_margin(const void *, json &) override
  {
  }

public:
  std::optional<json> get_latest_position() override
  {
    json pos;
    if (this->counter == 0)
      pos["currentQty"] = 0;
    if (this->counter == 1)
      pos["currentQty"] = -25;
    if (this->counter == 2)
      pos["currentQty"] = 50;

    this->counter++;

    return pos;
  }
  std::optional<json> get_latest_margin() override
  {
    json margin;
    return margin;
  }
};

TEST_CASE("skew")
{
  Models::QuotingParameters param(0.5, 25, 0, 100, 1, 5, 300);
  QuotingParameters qp(param);
  TestPositionGateway pg;
  BitmexDetailsGateway details;
  Skew skew(qp, pg, details);
  auto s1 = skew.get_latest();
  CHECK(s1.value().value == 0);
  auto s2 = skew.get_latest();
  CHECK(s2.value().value == 0.5);
  auto s3 = skew.get_latest();
  CHECK(s3.value().value == -1);
}