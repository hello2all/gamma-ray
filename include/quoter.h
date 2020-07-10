#pragma once
#include <vector>
#include "models.h"

class QuoterBase
{
protected:
  std::vector<Models::QuoteOrder> sent_quotes;

public:
  virtual std::vector<Models::QuoteOrder> quote_sent() = 0;
};

class Quoter : public QuoterBase
{
private:
public:
  Quoter();
  ~Quoter();

  std::vector<Models::QuoteOrder> quote_sent() override;
};