#include <iostream>
#include "quoter.h"
#include "doctest.h"
#include "bitmex_gateway.h"
#include "bitmexws.h"
#include "interfaces.h"
#include "market_filtration.h"
#include "Poco/Delegate.h"

class TestQuoter : public QuoterBase
{
public:
  std::vector<Models::QuoteOrder> quote_sent() override
  {
    Models::Quote q2(9500.5, 50, Models::Side::Ask);
    Models::Quote q1(9500, 50, Models::Side::Bid);
    std::string orderId1 = "orderId1";
    std::string orderId2 = "orderId2";
    Models::QuoteOrder qo1(q1, orderId1);
    Models::QuoteOrder qo2(q2, orderId2);
    std::vector<Models::QuoteOrder> sent_quotes = {qo1, qo2};
    return sent_quotes;
  }
};

class TestMarketDataGateway : public Interfaces::IMarketDataGateway
{
public:
  void trigger_market_quote()
  {
    std::string symbol = "XBTUSD";
    Poco::DateTime time;
    Models::MarketQuote mq(9500.5, 50.0, 9500.0, 100.0, symbol, time);

    this->market_quote(this, mq);
  }
};

class MarketFiltrationListener
{
private:
  MarketFiltration &mf;

public:
  MarketFiltrationListener(MarketFiltration &mf)
      : mf(mf)
  {
    mf.filtered_quote += Poco::delegate(this, &MarketFiltrationListener::on_filtered_market);
  }

  ~MarketFiltrationListener()
  {
    mf.filtered_quote -= Poco::delegate(this, &MarketFiltrationListener::on_filtered_market);
  }

  void on_filtered_market(const void *, Models::MarketQuote &mq)
  {
    CHECK(mq.askPrice == 9500.5);
    CHECK(mq.bidPrice == 9500);
    CHECK(mq.askSize == 0);
    CHECK(mq.bidSize == 50);
  }
};

TEST_CASE("market filtration")
{
  TestQuoter quoter;
  TestMarketDataGateway md;
  BitmexDetailsGateway details;
  MarketFiltration mf(md, details, quoter);
  MarketFiltrationListener mfl(mf);
  md.trigger_market_quote();
}