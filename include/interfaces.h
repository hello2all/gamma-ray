#pragma once
#include "Poco/BasicEvent.h"

namespace Interfaces
{
  class IMarketDataGateway
  {
  public:
    Poco::BasicEvent<Models::MarketQuote> market_quote;
  };

  class IExchangeDetailsGateway
  {
    public:
    double maker_fee;
    double taker_fee;
    double min_tick_increment;
    double min_size_increment;
    double face_value;
    unsigned int max_leverage;
  };

} // namespace interfaces