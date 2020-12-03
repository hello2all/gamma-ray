#include "doctest.h"
#include <iostream>
#include <vector>
#include "models.h"
#include "quoting_strategies.h"
#include "Poco/DateTime.h"

TEST_CASE("Top quoting strategy")
{
  QuotingStrategies::Top top(0.5);
  std::vector<QuotingStrategies::QuotingStyle *> quoting_styles;
  quoting_styles.push_back(&top);
  QuotingStrategies::QuotingStyleRegistry quoting_style_registry(quoting_styles);
  std::string symbol = "XBTUSD";

  SUBCASE("size > 0")
  {
    Models::MarketQuote mq(9501.5, 1000, 9500, 2000, symbol, Poco::DateTime());
    Models::FairValue fv(9500.25, Poco::DateTime());
    Models::QuotingParameters qp(Models::QuotingMode::Top, 25, 2, 0.5, 0, 0, 100, 1, 5, 300);
    QuotingStrategies::QuoteInput input(mq, fv, qp, 0, 0.5, 1);
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
    Models::QuotingParameters qp(Models::QuotingMode::Top, 25, 2, 0.5, 0, 0, 100, 1, 5, 300);
    QuotingStrategies::QuoteInput input(mq, fv, qp, 0, 0.5, 1);
    QuotingStrategies::GeneratedQuote quote = quoting_style_registry.get(Models::QuotingMode::Top)->generate_quote(input);

    CHECK(quote.askPrice == 9501.5);
    CHECK(quote.askSize == 25);
    CHECK(quote.bidPrice == 9500.0);
    CHECK(quote.bidSize == 25);
  }
}

TEST_CASE("Mid quoting strategy")
{
  QuotingStrategies::Mid mid(1);
  std::vector<QuotingStrategies::QuotingStyle *> quoting_styles;
  quoting_styles.push_back(&mid);
  QuotingStrategies::QuotingStyleRegistry quoting_style_registry(quoting_styles);
  std::string symbol = "XBTUSD";

  Models::MarketQuote mq(9501.5, 1000, 9500, 2000, symbol, Poco::DateTime());
  Models::FairValue fv(9500.25, Poco::DateTime());
  Models::QuotingParameters qp(Models::QuotingMode::Mid, 25, 2, 0.5, 0, 0, 100, 1, 5, 300);
  QuotingStrategies::QuoteInput input(mq, fv, qp, 0, 0.5, 1);
  QuotingStrategies::GeneratedQuote quote = quoting_style_registry.get(Models::QuotingMode::Mid)->generate_quote(input);

  CHECK(quote.askPrice == 9500.75);
  CHECK(quote.askSize == 25);
  CHECK(quote.bidPrice == 9499.75);
  CHECK(quote.bidSize == 25);
}

TEST_CASE("Avellaneda Stoikov strategy")
{
  double gamma = 0.4;
  double sigma = 7.0;
  double k = 8.5;

  QuotingStrategies::AvellanedaStoikov as(gamma, sigma, k);
  std::vector<QuotingStrategies::QuotingStyle *> quoting_styles;
  quoting_styles.push_back(&as);
  QuotingStrategies::QuotingStyleRegistry quoting_style_registry(quoting_styles);
  std::string symbol = "XBTUSD";

  Models::MarketQuote mq(9501.5, 1000, 9500, 2000, symbol, Poco::DateTime());
  Models::FairValue fv(9500.25, Poco::DateTime());
  Models::QuotingParameters qp(Models::QuotingMode::Mid, 25, 2, 0.5, 0, 0, 100, 1, 5, 300);
  QuotingStrategies::QuoteInput input(mq, fv, qp, 0, 0.5, 1);
  QuotingStrategies::GeneratedQuote quote = quoting_style_registry.get(Models::QuotingMode::AvellanedaStoikov)->generate_quote(input);

  CHECK((quote.askPrice - 9509.91) > 0.05);
  CHECK(quote.askSize == 25);
  CHECK((quote.bidPrice - 9490.08) > 0.05);
  CHECK(quote.bidSize == 25);
}
