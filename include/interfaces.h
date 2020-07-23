#pragma once
#include <vector>
#include "models.h"
#include "Poco/BasicEvent.h"

using json = nlohmann::json;

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

  class IOrderEntryGateway
  {
  public:
    virtual std::string generate_client_id() = 0;

    virtual void batch_send_order(std::vector<Models::NewOrder> orders) = 0;
    virtual void batch_cancel_order(std::vector<Models::CancelOrder> cancels) = 0;
    virtual void batch_replace_order(std::vector<Models::ReplaceOrder> replaces) = 0;
    virtual unsigned int cancel_all() = 0;
    virtual void on_order(const void *, json &order) = 0;
    virtual void on_execution(const void *, json &execution) = 0;

    Poco::BasicEvent<Models::Trade> trade;
  };
} // namespace Interfaces