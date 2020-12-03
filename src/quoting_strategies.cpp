#include "quoting_strategies.h"

namespace QuotingStrategies
{
  QuoteInput::QuoteInput(Models::MarketQuote &mq, Models::FairValue &fv, Models::QuotingParameters &qp, double position, double min_tick_increment, double min_size_increment)
      : mq(mq), fv(fv), qp(qp), position(position), min_tick_increment(min_tick_increment), min_size_increment(min_size_increment)
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

  Top::Top(double spread)
      : spread(spread)
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
    double min_ask = input.fv.price + this->spread / 2.0;
    double min_bid = input.fv.price - this->spread / 2.0;
    askPrice = std::max(min_ask, askPrice);
    bidPrice = std::min(min_bid, bidPrice);

    return GeneratedQuote(askPrice, input.qp.size, bidPrice, input.qp.size);
  }

  Mid::Mid(double spread)
      : spread(spread)
  {
    this->mode = Models::QuotingMode::Mid;
  }

  Mid::~Mid()
  {
  }

  GeneratedQuote Mid::generate_quote(QuoteInput &input)
  {
    double askPrice = input.fv.price + this->spread / 2.0;
    double bidPrice = input.fv.price - this->spread / 2.0;

    return GeneratedQuote(askPrice, input.qp.size, bidPrice, input.qp.size);
  }

  AvellanedaStoikov::AvellanedaStoikov(double gamma, double sigma, double k)
      : gamma(gamma), sigma(sigma), k(k)
  {
    this->mode = Models::QuotingMode::AvellanedaStoikov;
    this->settle_time = util::get_milli_seconds_timestamp(util::current_time()).count() + this->ONE_DAY_IN_MS; // settle 24h from now
  }

  AvellanedaStoikov::~AvellanedaStoikov()
  {
  }

  // time horizen (T - t) is between [0-1], default rotate every 24h, meaning the algo will try to exit with 0 inventory everyday since start
  double AvellanedaStoikov::time_horizon()
  {
    // current time in milliseconds since epoch
    double now = util::get_milli_seconds_timestamp(util::current_time()).count();
    double time_remain = this->settle_time - now;
    double th;
    if (time_remain > 0)
    {
      th = time_remain / this->ONE_DAY_IN_MS; // 24 * 3600 * 1000 milliseconds
    }
    else
    {
      // postpone settlement time to 24h latter
      this->settle_time += this->ONE_DAY_IN_MS;
      double new_time_remain = this->settle_time - now;
      th = new_time_remain / this->ONE_DAY_IN_MS;
    }

    return th;
  }

  GeneratedQuote AvellanedaStoikov::generate_quote(QuoteInput &input)
  {
    // position (flip the inverted position)
    double p = input.position / input.fv.price;
    // time horizon [0,1]
    double th = this->time_horizon();
    // reservation/indifference price r(s,t)
    double r = input.fv.price - p * this->gamma * std::pow(this->sigma, 2.0) * th;
    // spread
    double spread = this->gamma * std::pow(this->sigma, 2.0) + 2 / this->gamma * std::log(1 + this->gamma / this->k);
    // ask price
    double ra = r + spread / 2;
    // bid price
    double rb = r - spread / 2;

    return GeneratedQuote(ra, input.qp.size, rb, input.qp.size);
  }
} // namespace QuotingStrategies