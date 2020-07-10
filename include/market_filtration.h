#pragma once
#include "quoter.h"
#include "models.h"
#include "interfaces.h"
#include "Poco/Delegate.h"

class MarketFiltrationBase
{
public:
  Poco::BasicEvent<Models::MarketQuote> filtered_quote;
  virtual Models::MarketQuote get_latest() = 0;
};

class MarketFiltration : public MarketFiltrationBase
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

  Models::MarketQuote get_latest() override;
};