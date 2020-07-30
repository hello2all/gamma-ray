#include "market_filtration.h"

MarketFiltration::MarketFiltration(Interfaces::IMarketDataGateway &md, Interfaces::IOrderEntryGateway &oe, Interfaces::IExchangeDetailsGateway &details)
    : md(md), oe(oe), details(details)
{
  md.market_quote += Poco::delegate(this, &MarketFiltration::filter_market);
}

MarketFiltration::~MarketFiltration()
{
  md.market_quote -= Poco::delegate(this, &MarketFiltration::filter_market);
}

void MarketFiltration::filter_market(const void *, Models::MarketQuote &mq)
{
  // get quotes sent
  auto open_orders = this->oe.open_orders();
  if (!open_orders)
    return;

  // init filtered market filtration
  this->latest = mq;

  // loop open orders
  for (auto const &o : open_orders.value())
  {
    if (o["side"].get<std::string>() == "Buy")
    {
      // if market quote diff to quotes sent: deduct size
      if (o["price"].get<double>() - mq.bidPrice < this->details.min_tick_increment)
        this->latest.value().bidSize = this->latest.value().bidSize - o["orderQty"].get<double>();
    }
    else
    {
      // if market quote diff to quotes sent: deduct size
      if (o["price"].get<double>() - mq.askPrice < this->details.min_tick_increment)
        this->latest.value().askSize = this->latest.value().askSize - o["orderQty"].get<double>();
    }
  }
  this->filtered_quote(this, this->latest.value());
}

Models::MarketQuote MarketFiltration::get_latest()
{
  return this->latest.value();
}