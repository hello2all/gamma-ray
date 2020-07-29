#pragma once
#include <optional>
#include "json.hpp"
#include "models.h"
#include "interfaces.h"
#include "Poco/Delegate.h"

using json = nlohmann::json;
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
  Interfaces::IOrderEntryGateway &oe;
  Interfaces::IExchangeDetailsGateway &details;
  std::optional<Models::MarketQuote> latest;

  void filter_market(const void *, Models::MarketQuote &mq);

public:
  MarketFiltration(Interfaces::IMarketDataGateway &md, Interfaces::IOrderEntryGateway &oe,Interfaces::IExchangeDetailsGateway &details);
  ~MarketFiltration();

  Models::MarketQuote get_latest() override;
};