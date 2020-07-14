#include "fair_value.h"

FairValue::FairValue(MarketFiltrationBase &mf, Interfaces::IExchangeDetailsGateway &details)
    : mf(mf), details(details)
{
  this->mf.filtered_quote += Poco::delegate(this, &FairValue::calculate_fair_value);
}

FairValue::~FairValue()
{
  this->mf.filtered_quote -= Poco::delegate(this, &FairValue::calculate_fair_value);
}

void FairValue::calculate_fair_value(const void *, Models::MarketQuote &filtered_quote)
{
  // if bidSize or askSize equals 0, then price shall shift back by 1 tick, to prevent price competition with self
  double askPrice = filtered_quote.askSize > 0 ? filtered_quote.askPrice : (filtered_quote.askPrice + this->details.min_tick_increment);
  double bidPrice = filtered_quote.bidSize > 0 ? filtered_quote.bidPrice : (filtered_quote.bidPrice - this->details.min_tick_increment);
  double mid = (askPrice + bidPrice) / 2;
  this->latest = Models::FairValue(mid, filtered_quote.time);
  this->fair_value_changed(this, this->latest.value());
}

Models::FairValue FairValue::get_latest()
{
  return this->latest.value();
}