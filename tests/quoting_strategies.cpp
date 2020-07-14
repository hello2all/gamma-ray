#include "doctest.h"
#include <iostream>
#include <vector>
#include "models.h"
#include "quoting_strategies.h"
#include "Poco/DateTime.h"

TEST_CASE("Top quoting strategy")
{
  QuotingStrategies::Top top;
  std::vector<QuotingStrategies::QuotingStyle *> quoting_styles;
  quoting_styles.push_back(&top);
  QuotingStrategies::QuotingStyleRegistry quoting_style_registry(quoting_styles);
  std::string symbol = "XBTUSD";

  SUBCASE("size > 0")
  {
    Models::MarketQuote mq(9501.5, 1000, 9500, 2000, symbol, Poco::DateTime());
    Models::FairValue fv(9500.25, Poco::DateTime());
    Models::QuotingParameters qp(0.5, 25, 0, 100, 1, 5, 300);
    QuotingStrategies::QuoteInput input(mq, fv, qp, 0.5, 1);
    QuotingStrategies::GeneratedQuote quote = quoting_style_registry.get(Models::QuotingMode::Top)->generate_quote(input);

    CHECK(quote.askPrice == 9501.0);
    CHECK(quote.askSize == 25);
    CHECK(quote.bidPrice == 9500.0);
    CHECK(quote.bidSize == 25);
  }

  SUBCASE("size == 0")
  {
    Models::MarketQuote mq(9501.5, 0, 9500, 2000, symbol, Poco::DateTime());
    Models::FairValue fv(9500.25, Poco::DateTime());
    Models::QuotingParameters qp(0.5, 25, 0, 100, 1, 5, 300);
    QuotingStrategies::QuoteInput input(mq, fv, qp, 0.5, 1);
    QuotingStrategies::GeneratedQuote quote = quoting_style_registry.get(Models::QuotingMode::Top)->generate_quote(input);

    CHECK(quote.askPrice == 9501.5);
    CHECK(quote.askSize == 25);
    CHECK(quote.bidPrice == 9500.0);
    CHECK(quote.bidSize == 25);
  }
}
