#pragma once
#include "quoter.h"
#include "models.h"
#include "interfaces.h"
#include "Poco/Delegate.h"

class MarketFiltration
{
private:
  Interfaces::IMarketDataGateway &md;
  Interfaces::IExchangeDetailsGateway &details;
  QuoterBase &quoter;
  Models::MarketQuote latest;

  void filter_market(const void *, Models::MarketQuote &mq);

public:
  MarketFiltration(Interfaces::IMarketDataGateway &md, Interfaces::IExchangeDetailsGateway &details, QuoterBase &quoter);
  ~MarketFiltration();

  Poco::BasicEvent<Models::MarketQuote> filtered_quote;

  Models::MarketQuote get_latest();
};