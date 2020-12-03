#pragma once
#include <optional>
#include <cmath>
#include <algorithm>
#include "fair_value.h"
#include "market_filtration.h"
#include "quoting_strategies.h"
#include "quoting_parameters.h"
#include "interfaces.h"
#include "skew.h"
#include "util.h"
#include "Poco/DateTime.h"
#include "Poco/Delegate.h"
#include "Poco/BasicEvent.h"

class QuotingEngine
{
private:
  QuotingStrategies::QuotingStyleRegistry &registry;
  FairValueBase &fv;
  QuotingParameters &qp;
  MarketFiltrationBase &mf;
  Skew &skew;
  Interfaces::IOrderEntryGateway &oe;
  Interfaces::IPositionGateway &pg;
  Interfaces::IExchangeDetailsGateway &details;
  std::vector<Models::Quote> bids_cache;
  std::vector<Models::Quote> asks_cache;
  double min_tick_increment;
  double min_size_increment;
  bool enable_skew = true;

  std::optional<Models::TwoSidedQuote> latest;

  void on_filtered_quote(const void *, Models::MarketQuote &filtered_quote);
  // void on_fair_value(const void *, Models::FairValue);
  void on_quoting_parameters(const void *, Models::QuotingParameters &quoting_parameters);
  void on_trade(const void *, Models::Trade &);
  bool quotes_are_same(Models::TwoSidedQuote &two_sided_quote);
  void set_latest(Models::TwoSidedQuote &&two_sided_quote);
  void delete_quotes();
  bool get_skew_val(double &skew_val);
  bool get_position_val(double &position_val);

public:
  QuotingEngine(QuotingStrategies::QuotingStyleRegistry &registry, FairValueBase &fv, QuotingParameters &qp, MarketFiltrationBase &mf, Skew &skew, Interfaces::IOrderEntryGateway &oe, Interfaces::IPositionGateway &pg, Interfaces::IExchangeDetailsGateway &details);
  ~QuotingEngine();
  std::optional<Models::TwoSidedQuote> get_latest();

  Models::TwoSidedQuote calc_quote(Models::MarketQuote &market_quote, Models::FairValue &fair_value, Models::QuotingParameters &quoting_parameters, double skew_val, double position, double min_tick_increment, double min_size_increment, Poco::DateTime time);

  Poco::BasicEvent<Models::TwoSidedQuote> new_quote;
};