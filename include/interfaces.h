#pragma once
#include <vector>
#include <optional>
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
    std::string pair;
    double maker_fee;
    double taker_fee;
    double min_tick_increment;
    double min_size_increment;
    double face_value;
    unsigned int max_leverage;
  };

  class IOrderEntryGateway
  {
  protected:
    virtual void on_order(const void *, json &order) = 0;
    virtual void on_execution(const void *, json &execution) = 0;

  public:
    virtual std::string generate_client_id() = 0;

    virtual void batch_send_order(std::vector<Models::NewOrder> &orders) = 0;
    virtual void batch_cancel_order(std::vector<Models::CancelOrder> &cancels) = 0;
    virtual void batch_replace_order(std::vector<Models::ReplaceOrder> &replaces) = 0;
    virtual unsigned int cancel_all() = 0;
    virtual std::optional<json> open_orders() = 0;

    Poco::BasicEvent<Models::Trade> trade;
    Poco::BasicEvent<long> n_orders;
  };

  class IPositionGateway
  {
  protected:
    virtual void on_position(const void *, json &position) = 0;
    virtual void on_margin(const void *, json &margin) = 0;

  public:
    virtual std::optional<json> get_latest_position() = 0;
    virtual std::optional<json> get_latest_margin() = 0;
  };

  class IRateLimitMonitor
  {
  public:
    virtual void update_rate_limit(int limit, int remain, Poco::DateTime next_reset) = 0;
    virtual bool is_rate_limited() = 0;
  };
} // namespace Interfaces