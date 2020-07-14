#pragma once
#include <optional>
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "market_filtration.h"
#include "models.h"
#include "interfaces.h"

class FairValueBase
{
public:
  Poco::BasicEvent<Models::FairValue> fair_value_changed;
  virtual Models::FairValue get_latest() = 0;
};

class FairValue : public FairValueBase
{
private:
  MarketFiltrationBase &mf;
  Interfaces::IExchangeDetailsGateway &details;
  std::optional<Models::FairValue> latest;
  void calculate_fair_value(const void *, Models::MarketQuote &mkt_quote);

public:
  FairValue(MarketFiltrationBase &mf, Interfaces::IExchangeDetailsGateway &details);
  ~FairValue();

  Models::FairValue get_latest() override;
};