#include "doctest.h"
#include "util.h"
#include "models.h"

TEST_CASE("round up")
{
  double result = util::round_up(9500.23, 0.5);
  CHECK(result == 9500.5);
}

TEST_CASE("round down")
{
  double result = util::round_down(9500.26, 0.5);
  CHECK(result == 9500);
}

TEST_CASE("round nearest")
{
  double result1 = util::round_nearest(9500.3, 0.5);
  double result2 = util::round_nearest(9500.2, 0.5);
  CHECK(result1 == 9500.5);
  CHECK(result2 == 9500.0);
}

TEST_CASE("round side")
{
  double bid = util::round_side(9500.3, 0.5, Models::Side::Bid);
  double ask = util::round_side(9500.3, 0.5, Models::Side::Ask);
  CHECK(bid == 9500);
  CHECK(ask == 9500.5);
}