#include "market_filtration.h"

MarketFiltration::MarketFiltration(Interfaces::IMarketDataGateway &md, Interfaces::IExchangeDetailsGateway &details, QuoterBase &quoter)
    : md(md), details(details), quoter(quoter)
{
  md.market_quote += Poco::delegate(this, &MarketFiltration::filter_market);
}

MarketFiltration::~MarketFiltration()
{
  md.market_quote -= Poco::delegate(this, &MarketFiltration::filter_market);
}

void MarketFiltration::filter_market(const void *, Models::MarketQuote &mq)
{
  // get quotes sent (qs)
  auto quote_sent = this->quoter.quote_sent();
  // init filtered market filtration (fmq)
  this->latest = mq;

  // loop qs
  for (auto const &q : quote_sent)
  {
    switch (q.quote.side)
    {
    case Models::Side::Bid:
      // if market quote diff to quotes sent: deduct size
      if (q.quote.price - mq.bidPrice < this->details.min_tick_increment)
        this->latest.value().bidSize = this->latest.value().bidSize - q.quote.size;
      break;
    case Models::Side::Ask:
      // if market quote diff to quotes sent: deduct size
      if (q.quote.price - mq.askPrice < this->details.min_tick_increment)
        this->latest.value().askSize = this->latest.value().askSize - q.quote.size;
      break;
    }
  }
  this->filtered_quote(this, this->latest.value());
}

Models::MarketQuote MarketFiltration::get_latest()
{
  return this->latest.value();
}