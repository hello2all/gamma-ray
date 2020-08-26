#pragma once
#include <map>
#include <algorithm>
#include "models.h"
#include "quoting_parameters.h"

namespace QuotingStrategies
{
  class QuoteInput
  {
  public:
    Models::MarketQuote &mq;
    Models::FairValue &fv;
    Models::QuotingParameters &qp;
    double min_tick_increment;
    double min_size_increment;

    QuoteInput(Models::MarketQuote &mq, Models::FairValue &fv, Models::QuotingParameters &qp, double min_tick_increment, double min_size_increment);
    ~QuoteInput();
  };

  class GeneratedQuote
  {
  public:
    double askPrice;
    double askSize;
    double bidPrice;
    double bidSize;

    GeneratedQuote(double askPrice, double askSize, double bidPrice, double bidSize);
    ~GeneratedQuote();
  };

  class QuotingStyle
  {
  public:
    Models::QuotingMode mode;
    virtual GeneratedQuote generate_quote(QuoteInput &input) = 0;
  };

  class QuotingStyleRegistry
  {
  private:
    std::map<Models::QuotingMode, QuotingStyle *> mappings;

  public:
    QuotingStyleRegistry(std::vector<QuotingStyle *>);
    ~QuotingStyleRegistry();

    QuotingStyle *get(Models::QuotingMode mode);
  };

  class Top : public QuotingStyle
  {
  public:
    Top();
    ~Top();

    GeneratedQuote generate_quote(QuoteInput &input) override;
  };

  class Mid : public QuotingStyle
  {
    public:
    Mid();
    ~Mid();

    GeneratedQuote generate_quote(QuoteInput &input) override;
  };
} // namespace QuotingStrategies