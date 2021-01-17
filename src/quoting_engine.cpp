#include "quoting_engine.h"

QuotingEngine::QuotingEngine(QuotingStrategies::QuotingStyleRegistry &registry, FairValueBase &fv, QuotingParameters &qp, MarketFiltrationBase &mf, Skew &skew, Interfaces::IOrderEntryGateway &oe, Interfaces::IPositionGateway &pg, Interfaces::IExchangeDetailsGateway &details)
    : registry(registry), fv(fv), qp(qp), mf(mf), skew(skew), oe(oe), pg(pg), details(details)
{
  this->min_tick_increment = details.min_tick_increment;
  this->min_size_increment = details.min_size_increment;
  auto params = this->qp.get_latest();
  if (params.skew_factor > -1e6 && params.skew_factor < 1e-10)
  {
    this->enable_skew = false;
  }
  this->bids_cache.reserve(qp.get_latest().pairs * 2);
  this->asks_cache.reserve(qp.get_latest().pairs * 2);
  mf.filtered_quote += Poco::delegate(this, &QuotingEngine::on_filtered_quote);
  // fv.fair_value_changed += Poco::delegate(this, &QuotingEngine::on_fair_value);
  qp.parameters_changed += Poco::delegate(this, &QuotingEngine::on_quoting_parameters);
  oe.trade += Poco::delegate(this, &QuotingEngine::on_trade);
}

QuotingEngine::~QuotingEngine()
{
  mf.filtered_quote -= Poco::delegate(this, &QuotingEngine::on_filtered_quote);
  // fv.fair_value_changed -= Poco::delegate(this, &QuotingEngine::on_fair_value);
  qp.parameters_changed -= Poco::delegate(this, &QuotingEngine::on_quoting_parameters);
  oe.trade -= Poco::delegate(this, &QuotingEngine::on_trade);
}

void QuotingEngine::on_filtered_quote(const void *, Models::MarketQuote &filtered_quote)
{
  auto fair_value = this->fv.get_latest();
  auto params = this->qp.get_latest();

  double position_val;
  if (this->get_position_val(position_val) != true)
    return;

  double skew_val = 0.0;
  if (this->enable_skew && (this->get_skew_val(skew_val) != true))
    return;

  auto two_sided_quote = this->calc_quote(filtered_quote, fair_value, params, skew_val, position_val, this->min_tick_increment, this->min_size_increment, filtered_quote.time);
  this->set_latest(std::move(two_sided_quote));
}

// void QuotingEngine::on_fair_value(const void *, Models::FairValue &fair_value)
// {
// }

void QuotingEngine::on_quoting_parameters(const void *, Models::QuotingParameters &quoting_parameters)
{
  auto fair_value = this->fv.get_latest();
  auto filtered_quote = this->mf.get_latest();

  double position_val;
  if (this->get_position_val(position_val) != true)
    return;

  double skew_val = 0.0;
  if (this->enable_skew && (this->get_skew_val(skew_val) != true))
    return;

  auto two_sided_quote = this->calc_quote(filtered_quote, fair_value, quoting_parameters, skew_val, position_val, this->min_tick_increment, this->min_size_increment, filtered_quote.time);
  this->set_latest(std::move(two_sided_quote));
}

void QuotingEngine::on_trade(const void *, Models::Trade &)
{
  auto fair_value = this->fv.get_latest();
  auto filtered_quote = this->mf.get_latest();
  auto params = this->qp.get_latest();

  double position_val;
  if (this->get_position_val(position_val) != true)
    return;

  double skew_val = 0.0;
  if (this->enable_skew && (this->get_skew_val(skew_val) != true))
    return;

  auto two_sided_quote = this->calc_quote(filtered_quote, fair_value, params, skew_val, position_val, this->min_tick_increment, this->min_size_increment, filtered_quote.time);
  this->set_latest(std::move(two_sided_quote));
}

std::optional<Models::TwoSidedQuote> QuotingEngine::get_latest()
{
  return this->latest;
}

void QuotingEngine::set_latest(Models::TwoSidedQuote &&two_sided_quote)
{
  if (this->quotes_are_same(two_sided_quote))
  {
    return;
  }
  else
  {
    this->latest = std::move(two_sided_quote);
    this->new_quote(this, this->latest.value());
  }
}

Models::TwoSidedQuote QuotingEngine::calc_quote(Models::MarketQuote &filtered_quote, Models::FairValue &fair_value, Models::QuotingParameters &quoting_parameters, double skew_val, double position, double min_tick_increment, double min_size_increment, Poco::DateTime time)
{
  QuotingStrategies::QuoteInput input(filtered_quote, fair_value, quoting_parameters, position, min_tick_increment, min_size_increment);
  QuotingStrategies::GeneratedQuote unrounded = this->registry.get(quoting_parameters.mode)->generate_quote(input);
  // apply skew
  if (this->enable_skew)
  {
    unrounded.bidPrice = std::min(unrounded.bidPrice + skew_val, fair_value.price);
    unrounded.askPrice = std::max(unrounded.askPrice + skew_val, fair_value.price);
  }
  // rounding
  unrounded.bidPrice = util::round_side(unrounded.bidPrice, min_tick_increment, Models::Side::Bid);
  unrounded.askPrice = util::round_side(unrounded.askPrice, min_tick_increment, Models::Side::Ask);
  unrounded.bidSize = util::round_down(unrounded.bidSize, min_size_increment);
  unrounded.bidSize = std::max(unrounded.bidSize, min_size_increment);
  unrounded.askSize = util::round_down(unrounded.askSize, min_size_increment);
  unrounded.askSize = std::max(unrounded.askSize, min_size_increment);

  // create two sided quote
  this->bids_cache.clear();
  this->asks_cache.clear();
  for (unsigned int i = 0; i < quoting_parameters.pairs; i++)
  {
    Models::Quote bid(unrounded.bidPrice - quoting_parameters.price_interval * i, unrounded.bidSize + quoting_parameters.size_increment * i, Models::Side::Bid);
    Models::Quote ask(unrounded.askPrice + quoting_parameters.price_interval * i, unrounded.askSize + quoting_parameters.size_increment * i, Models::Side::Ask);
    this->bids_cache.emplace_back(std::move(bid));
    this->asks_cache.emplace_back(std::move(ask));
  }

  return Models::TwoSidedQuote(this->bids_cache, this->asks_cache, time);
}

bool QuotingEngine::quotes_are_same(Models::TwoSidedQuote &two_sided_quote)
{
  // if latest not exist
  if (!(this->latest))
    return false;

  auto latest_quotes = this->latest.value();

  double bid_delta = std::fabs(two_sided_quote.bids[0].price - latest_quotes.bids[0].price);
  double ask_delta = std::fabs(two_sided_quote.asks[0].price - latest_quotes.asks[0].price);
  // abs price diff less than min tick
  if ((bid_delta < this->min_tick_increment) && (ask_delta < this->min_tick_increment))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void QuotingEngine::delete_quotes()
{
  this->latest = std::nullopt;
}

bool QuotingEngine::get_skew_val(double &skew_val)
{
  auto skew = this->skew.get_latest();
  if (!skew)
  {
    this->delete_quotes();
    return false;
  }
  else
  {
    skew_val = skew.value().value;
    return true;
  }
}

bool QuotingEngine::get_position_val(double &position_val)
{
  auto pos = this->pg.get_latest_position();

  if (!pos)
  {
    this->delete_quotes();
    return false;
  }
  else if (pos.value().size() == 0)
  {
    position_val = 0.0;
    return true;
  }
  else
  {
    position_val = pos.value()[0]["currentQty"].get<double>();
    return true;
  }
}
