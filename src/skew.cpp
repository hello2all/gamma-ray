#include "skew.h"

Skew::Skew(QuotingParameters &qp, Interfaces::IPositionGateway &pg, Interfaces::IExchangeDetailsGateway &details)
    : qp(qp), pg(pg), details(details)
{
}

std::optional<Models::Skew> Skew::get_latest()
{
  auto pos = this->pg.get_latest_position();
  if (pos)
  {
    auto params = this->qp.get_latest();
    double skew_factor = params.skew_factor;
    double size = params.size;
    double tick = this->details.min_tick_increment;
    double qty = pos.value().size() > 0 ? pos.value()[0]["currentQty"].get<double>() : 0;
    double skew_value = qty >= 0 ? -(std::floor(qty / size) * skew_factor * tick) : -(std::ceil(qty / size) * skew_factor * tick);
    return Models::Skew(skew_value, Poco::DateTime());
  }
  else
  {
    return std::nullopt;
  }
}