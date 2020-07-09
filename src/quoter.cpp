
#include "quoter.h"

Quoter::Quoter()
{
}

Quoter::~Quoter()
{
}


std::vector<Models::QuoteOrder> Quoter::quote_sent()
{
  return this->sent_quotes;
}