#include <iostream>
#include "doctest.h"
#include "json.hpp"
#include "fair_value.h"
#include "models.h"
#include "market_filtration.h"
#include "bitmex_gateway.h"
#include "Poco/Delegate.h"

class TestMarketFiltration : public MarketFiltrationBase
{
public:
  Models::MarketQuote get_latest() override
  {
    std::string symbol = "XBTUSD";
    Poco::DateTime time;
    Models::MarketQuote mq(9500.5, 0.0, 9500.0, 100.0, symbol, time);

    this->filtered_quote(this, mq);
    return mq;
  }
};

class FairValueListener
{
private:
  FairValue &fv;

public:
  FairValueListener(FairValue &fv)
      : fv(fv)
  {
    fv.fair_value_changed += Poco::delegate(this, &FairValueListener::on_fair_value);
  }
  ~FairValueListener()
  {
    fv.fair_value_changed -= Poco::delegate(this, &FairValueListener::on_fair_value);
  }

  void on_fair_value(const void *, Models::FairValue &fv)
  {
    CHECK(fv.price == 9500.5);
  }
};

TEST_CASE("fair value")
{
  TestMarketFiltration mf;
  BitmexDetailsGateway details;
  FairValue fv(mf, details);
  FairValueListener fvl(fv);
  mf.get_latest();
}