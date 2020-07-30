#include "doctest.h"
#include <vector>
#include "json.hpp"
#include "quoting_engine.h"
#include "fair_value.h"
#include "market_filtration.h"
#include "quoting_parameters.h"
#include "quoting_strategies.h"
#include "skew.h"
#include "bitmex_gateway.h"
#include "models.h"

using json = nlohmann::json;

class TestMarketFiltration : public MarketFiltrationBase
{
private:
  std::optional<Models::MarketQuote> latest;

public:
  void set_latest(Models::MarketQuote fq)
  {
    this->latest = fq;
  }

  Models::MarketQuote get_latest() override
  {
    this->filtered_quote(this, this->latest.value());
    return latest.value();
  }
};

class TestFairValue : public FairValueBase
{
public:
  Models::FairValue get_latest() override
  {
    return Models::FairValue(9500.25, Poco::DateTime());
  }
};

class TestPositionGateway : public Interfaces::IPositionGateway
{
private:
  bool enable_pos = true;
  int pos_qty = 0;

  void on_position(const void *, json &position) override
  {
    std::cout << position.dump() << std::endl;
  }

  void on_margin(const void *, json &margin) override
  {
    std::cout << margin.dump() << std::endl;
  }

public:
  std::optional<json> get_latest_position() override
  {
    if (this->enable_pos)
    {
      json pos = {{{"currentQty", this->pos_qty}}};
      return pos;
    }
    else
    {
      return std::nullopt;
    }
  }

  std::optional<json> get_latest_margin() override
  {
    json margin = {{"availableMargin", 12054123}};
    return margin;
  }

  void disable_position()
  {
    this->enable_pos = false;
  }

  void set_position(int qty)
  {
    this->pos_qty = qty;
  }
};

class TestOrderEntryGateway : public Interfaces::IOrderEntryGateway
{
private:
  void on_order(const void *, json &) override {}
  void on_execution(const void *, json &) override {}

public:
  std::string generate_client_id() override
  {
    return "id";
  }

  void batch_send_order(std::vector<Models::NewOrder>) override {}
  void batch_cancel_order(std::vector<Models::CancelOrder>) override {}
  void batch_replace_order(std::vector<Models::ReplaceOrder>) override {}
  unsigned int cancel_all() override
  {
    return 0;
  }

  json open_orders() override
  {
    return json::array();
  };

  void trigger_trade()
  {
    Models::Trade t(this->generate_client_id(), Poco::DateTime(), Models::Side::Ask, 10.0, 9500, Models::Liquidity::Maker, 123456, 10);
    this->trade(this, t);
  }
};

class TestQuotingEngineListener
{
private:
  QuotingEngine &engine;
  void on_new_quote(const void *, Models::TwoSidedQuote &two_sided_quote)
  {
    this->call_counter++;
  }

public:
  int call_counter = 0;
  TestQuotingEngineListener(QuotingEngine &engine)
      : engine(engine)
  {
    engine.new_quote += Poco::delegate(this, &TestQuotingEngineListener::on_new_quote);
  }

  ~TestQuotingEngineListener()
  {
    engine.new_quote -= Poco::delegate(this, &TestQuotingEngineListener::on_new_quote);
  }
};

TEST_CASE("quoting engine")
{
  QuotingStrategies::Top top;
  std::vector<QuotingStrategies::QuotingStyle *> quoting_styles;
  quoting_styles.push_back(&top);
  QuotingStrategies::QuotingStyleRegistry quoting_style_registry(quoting_styles);

  TestFairValue fv;
  TestMarketFiltration mf;
  BitmexDetailsGateway details;
  Models::QuotingParameters params(Models::QuotingMode::Top, 0.5, 25.0, 0, 100.0, 1.0, 5.0, 300.0);
  QuotingParameters qp(params);
  TestPositionGateway pg;
  TestOrderEntryGateway oe;
  Skew skew(qp, pg, details);

  QuotingEngine engine(quoting_style_registry, fv, qp, mf, skew, oe, details);
  TestQuotingEngineListener tqel(engine);

  std::string symbol = "XBTUSD";
  Poco::DateTime time;

  // filtered_quote triggered
  SUBCASE("filtered quote triggered: spread == 0.5")
  {
    Models::MarketQuote mq(9500.5, 0.0, 9500.0, 100.0, symbol, time);
    mf.set_latest(mq);
    mf.get_latest();

    Models::TwoSidedQuote q = engine.get_latest().value();
    CHECK(q.bid.price == 9500);
    CHECK(q.ask.price == 9500.5);
    CHECK(q.bid.size == 25);
    CHECK(q.ask.size == 25);
    CHECK(tqel.call_counter == 1);
  }

  SUBCASE("filtered quote triggered: spread > 0.5")
  {
    Models::MarketQuote mq(9600.5, 1.0, 9402.0, 100.0, symbol, time);
    mf.set_latest(mq);
    mf.get_latest();

    Models::TwoSidedQuote q = engine.get_latest().value();
    CHECK(q.bid.price == 9402.5);
    CHECK(q.ask.price == 9600);
    CHECK(q.bid.size == 25);
    CHECK(q.ask.size == 25);
    CHECK(tqel.call_counter == 1);
  }

  // trade triggered
  SUBCASE("trade triggered: filtered quote size == 0")
  {
    Models::MarketQuote mq(9600.5, 0.0, 9402.0, 100.0, symbol, time);
    mf.set_latest(mq);
    oe.trigger_trade();
    Models::TwoSidedQuote q = engine.get_latest().value();
    CHECK(q.bid.price == 9402.5);
    CHECK(q.ask.price == 9600.5);
    CHECK(q.bid.size == 25);
    CHECK(q.ask.size == 25);
    CHECK(tqel.call_counter == 1);
  }

  // quoting parameters triggered
  SUBCASE("quoting parameter change triggered")
  {
    Models::MarketQuote mq(9600.5, 0.0, 9402.0, 100.0, symbol, time);
    mf.set_latest(mq);
    Models::QuotingParameters params2(Models::QuotingMode::Top, 0.5, 26.0, 0, 100.0, 1.0, 5.0, 300.0);
    qp.update_parameters(params2);
    Models::TwoSidedQuote q = engine.get_latest().value();
    CHECK(q.bid.price == 9402.5);
    CHECK(q.ask.price == 9600.5);
    CHECK(q.bid.size == 26);
    CHECK(q.ask.size == 26);
    CHECK(tqel.call_counter == 1);
  }

  // quotes are same
  SUBCASE("quotes are same")
  {
    Models::MarketQuote mq(9500.5, 0.0, 9500.0, 100.0, symbol, time);
    mf.set_latest(mq);
    mf.get_latest();

    Models::TwoSidedQuote q = engine.get_latest().value();
    CHECK(q.bid.price == 9500);
    CHECK(q.ask.price == 9500.5);
    CHECK(q.bid.size == 25);
    CHECK(q.ask.size == 25);
    CHECK(tqel.call_counter == 1);

    Models::MarketQuote mq1(9500.5, 0.0, 9500.0, 200.0, symbol, time);
    mf.set_latest(mq1);
    mf.get_latest();

    Models::TwoSidedQuote q1 = engine.get_latest().value();
    CHECK(q1.bid.price == 9500);
    CHECK(q1.ask.price == 9500.5);
    CHECK(q1.bid.size == 25);
    CHECK(q1.ask.size == 25);
    CHECK(tqel.call_counter == 1);
  }

  // delete quotes
  SUBCASE("delete quotes")
  {
    pg.disable_position();
    Models::MarketQuote mq(9500.5, 0.0, 9500.0, 100.0, symbol, time);
    mf.set_latest(mq);
    mf.get_latest();

    auto q = engine.get_latest();
    CHECK(q == std::nullopt);
    CHECK(tqel.call_counter == 0);
  }

  // skew
  SUBCASE("skew")
  {
    pg.set_position(25);
    Models::MarketQuote mq(9500.5, 0.0, 9500.0, 100.0, symbol, time);
    mf.set_latest(mq);
    mf.get_latest();

    Models::TwoSidedQuote q = engine.get_latest().value();
    CHECK(q.bid.price == 9499.5);
    CHECK(q.ask.price == 9500.5);
    CHECK(q.bid.size == 25);
    CHECK(q.ask.size == 25);
    CHECK(tqel.call_counter == 1);

    pg.set_position(-25);
    mf.set_latest(mq);
    mf.get_latest();

    Models::TwoSidedQuote q1 = engine.get_latest().value();
    CHECK(q1.bid.price == 9500);
    CHECK(q1.ask.price == 9501);
    CHECK(q1.bid.size == 25);
    CHECK(q1.ask.size == 25);
    CHECK(tqel.call_counter == 2);
  }
}