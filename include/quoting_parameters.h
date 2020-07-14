#pragma once
#include "models.h"
#include "Poco/BasicEvent.h"

class QuotingParameters
{
private:
  Models::QuotingParameters latest;

public:
  QuotingParameters(Models::QuotingParameters &qp);
  ~QuotingParameters();

  Poco::BasicEvent<Models::QuotingParameters> parameters_changed;

  void update_parameters(Models::QuotingParameters &nqp);
  Models::QuotingParameters get_latest();
};