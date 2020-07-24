#pragma once
#include <optional>
#include <cmath>
#include "models.h"
#include "interfaces.h"
#include "quoting_parameters.h"

class Skew
{
private:
  QuotingParameters &qp;
  Interfaces::IPositionGateway &pg;
  Interfaces::IExchangeDetailsGateway &details;

public:
  std::optional<Models::Skew> get_latest();
  Skew(QuotingParameters &qp, Interfaces::IPositionGateway &pg, Interfaces::IExchangeDetailsGateway &details);
};