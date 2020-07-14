#include "quoting_strategies.h"

namespace QuotingStrategies
{
  QuoteInput::QuoteInput(Models::MarketQuote &mq, Models::FairValue &fv, Models::QuotingParameters &qp, double min_tick_increment, double min_size_increment)
      : mq(mq), fv(fv), qp(qp), min_tick_increment(min_tick_increment), min_size_increment(min_size_increment)
  {
  }

  QuoteInput::~QuoteInput()
  {
  }

  GeneratedQuote::GeneratedQuote(double askPrice, double askSize, double bidPrice, double bidSize)
      : askPrice(askPrice), askSize(askSize), bidPrice(bidPrice), bidSize(bidSize)
  {
  }

  GeneratedQuote::~GeneratedQuote()
  {
  }

  QuotingStyleRegistry::QuotingStyleRegistry(std::vector<QuotingStyle *> styles)
  {
    for (const auto style : styles)
    {
      this->mappings.emplace(style->mode, style);
    }
  }

  QuotingStyleRegistry::~QuotingStyleRegistry()
  {
  }

  QuotingStyle *QuotingStyleRegistry::get(Models::QuotingMode mode)
  {
    return this->mappings.at(mode);
  }

  Top::Top()
  {
    this->mode = Models::QuotingMode::Top;
  }

  Top::~Top()
  {
  }

  GeneratedQuote Top::generate_quote(QuoteInput &input)
  {
    double askPrice = input.mq.askSize > 0 ? (input.mq.askPrice - input.min_tick_increment) : input.mq.askPrice;
    double bidPrice = input.mq.bidSize > 0 ? (input.mq.bidPrice + input.min_tick_increment) : input.mq.bidPrice;
    double min_ask = input.fv.price + input.qp.width / 2.0;
    double min_bid = input.fv.price - input.qp.width / 2.0;
    askPrice = std::max(min_ask, askPrice);
    bidPrice = std::min(min_bid, bidPrice);

    return GeneratedQuote(askPrice, input.qp.size, bidPrice, input.qp.size);
  }
} // namespace QuotingStrategies