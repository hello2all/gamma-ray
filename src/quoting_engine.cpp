#include "quoting_engine.h"

QuotingEngine::QuotingEngine(QuotingStrategies::QuotingStyleRegistry &registry, FairValueBase &fv, QuotingParameters &qp, MarketFiltrationBase &mf, Skew &skew, Interfaces::IOrderEntryGateway &oe, Interfaces::IExchangeDetailsGateway &details)
    : registry(registry), fv(fv), qp(qp), mf(mf), skew(skew), oe(oe), details(details)
{
  this->min_tick_increment = details.min_tick_increment;
  this->min_size_increment = details.min_size_increment;
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
  auto skew = this->skew.get_latest();
  if (!skew)
  {
    this->delete_quotes();
    return;
  }

  auto two_sided_quote = this->calc_quote(filtered_quote, fair_value, params, skew.value(), this->min_tick_increment, this->min_size_increment, filtered_quote.time);
  // bool valid = this->validate_quotes(two_sided_quote);
  this->set_latest(std::move(two_sided_quote));
}

// void QuotingEngine::on_fair_value(const void *, Models::FairValue &fair_value)
// {
// }

void QuotingEngine::on_quoting_parameters(const void *, Models::QuotingParameters &quoting_parameters)
{
  auto fair_value = this->fv.get_latest();
  auto filtered_quote = this->mf.get_latest();
  auto skew = this->skew.get_latest();
  if (!skew)
  {
    this->delete_quotes();
    return;
  }
  auto two_sided_quote = this->calc_quote(filtered_quote, fair_value, quoting_parameters, skew.value(), this->min_tick_increment, this->min_size_increment, Poco::DateTime());
  // bool valid = this->validate_quotes(two_sided_quote);
  this->set_latest(std::move(two_sided_quote));
}

void QuotingEngine::on_trade(const void *, Models::Trade &)
{
  auto fair_value = this->fv.get_latest();
  auto filtered_quote = this->mf.get_latest();
  auto params = this->qp.get_latest();
  auto skew = this->skew.get_latest();
  if (!skew)
  {
    this->delete_quotes();
    return;
  }
  auto two_sided_quote = this->calc_quote(filtered_quote, fair_value, params, skew.value(), this->min_tick_increment, this->min_size_increment, Poco::DateTime());
  // bool valid = this->validate_quotes(two_sided_quote);
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

Models::TwoSidedQuote QuotingEngine::calc_quote(Models::MarketQuote &filtered_quote, Models::FairValue &fair_value, Models::QuotingParameters &quoting_parameters, Models::Skew &skew, double min_tick_increment, double min_size_increment, Poco::DateTime time)
{
  QuotingStrategies::QuoteInput input(filtered_quote, fair_value, quoting_parameters, min_tick_increment, min_size_increment);
  QuotingStrategies::GeneratedQuote unrounded = this->registry.get(quoting_parameters.mode)->generate_quote(input);
  // apply skew
  unrounded.bidPrice = std::min(unrounded.bidPrice + skew.value, fair_value.price);
  unrounded.askPrice = std::max(unrounded.askPrice + skew.value, fair_value.price);
  // rounding
  unrounded.bidPrice = util::round_side(unrounded.bidPrice, min_tick_increment, Models::Side::Bid);
  unrounded.askPrice = util::round_side(unrounded.askPrice, min_tick_increment, Models::Side::Ask);
  unrounded.bidSize = util::round_down(unrounded.bidSize, min_size_increment);
  unrounded.bidSize = std::max(unrounded.bidSize, min_size_increment);
  unrounded.askSize = util::round_down(unrounded.askSize, min_size_increment);
  unrounded.askSize = std::max(unrounded.askSize, min_size_increment);

  // create two sided quote
  Models::Quote bid(unrounded.bidPrice, unrounded.bidSize, Models::Side::Bid);
  Models::Quote ask(unrounded.askPrice, unrounded.askSize, Models::Side::Ask);
  return Models::TwoSidedQuote(std::move(bid), std::move(ask), time);
}

// bool QuotingEngine::validate_quotes(Models::TwoSidedQuote &two_sided_quote)
// {
// }

bool QuotingEngine::quotes_are_same(Models::TwoSidedQuote &two_sided_quote)
{
  // if latest not exist
  if (!(this->latest))
    return false;

  auto latest_quotes = this->latest.value();
  // abs price diff less than min tick
  if (std::abs(two_sided_quote.ask.price - latest_quotes.ask.price) >= this->min_tick_increment)
    return false;
  if (std::abs(two_sided_quote.bid.price - latest_quotes.bid.price) >= this->min_tick_increment)
    return false;
  // abs size diff less tahn min size
  if (std::abs(two_sided_quote.ask.size - latest_quotes.ask.size) >= this->min_size_increment)
    return false;
  if (std::abs(two_sided_quote.bid.size - latest_quotes.bid.size) >= this->min_size_increment)
    return false;

  return true;
}

void QuotingEngine::delete_quotes()
{
  this->latest = std::nullopt;
}