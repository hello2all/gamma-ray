#include "quoting_parameters.h"

QuotingParameters::QuotingParameters(Models::QuotingParameters &qp)
    : latest(qp)
{
}

QuotingParameters::~QuotingParameters()
{
}

void QuotingParameters::update_parameters(Models::QuotingParameters &nqp)
{
  this->latest = nqp;
  this->parameters_changed(this, this->latest);
}

Models::QuotingParameters QuotingParameters::get_latest()
{
  return this->latest;
}