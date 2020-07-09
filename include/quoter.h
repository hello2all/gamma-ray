#pragma once
#include <vector>
#include "models.h"

class Quoter
{
private:
  std::vector<Models::QuoteOrder> sent_quotes;
public:
  Quoter();
  ~Quoter();

  std::vector<Models::QuoteOrder> quote_sent();
};