#include "bitmex_gateway.h"

BitmexSymbolProvider::BitmexSymbolProvider(const std::string &c)
{
  if (c == "XBTUSD")
  {
    this->base = "XBt";
    this->quote = "USD";
    this->underlying = "XBt";
    this->symbol = "XBTUSD";
    this->symbol_with_type = "XBTUSD:Perpetual";
  }
  else if (c == "ETHUSD")
  {
    this->base = "ETH";
    this->quote = "USD";
    this->underlying = "XBt";
    this->symbol = "ETHUSD";
    this->symbol_with_type = "ETHUSD:Perpetual";
  }
  else if (c == "XRPUSD")
  {
    this->base = "XRP";
    this->quote = "USD";
    this->underlying = "XBt";
    this->symbol = "XRPUSD";
    this->symbol_with_type = "XRPUSD:Perpetual";
  }
  else if (c == "LTCUSD")
  {
    this->base = "LTC";
    this->quote = "USD";
    this->underlying = "XBt";
    this->symbol = "LTCUSD";
    this->symbol_with_type = "LTCUSD:Perpetual";
  }
  else if (c == "BCHUSD")
  {
    this->base = "BCH";
    this->quote = "USD";
    this->underlying = "XBt";
    this->symbol = "BCHUSD";
    this->symbol_with_type = "BCHUSD:Perpetual";
  }
  else
  {
    throw std::runtime_error("Invalid contract symbol");
  }
}

BitmexMarketDataGateway::BitmexMarketDataGateway(BitmexWebsocket &ws, BitmexSymbolProvider &symbol)
    : ws(ws), symbol(symbol)
{
  this->order_book_channel = "orderBook10:" + symbol.symbol;
  ws.set_handler(this->order_book_handle, [this](json order_book) {
    double bidPrice = order_book["data"].back()["bids"][0][0].get<double>();
    double bidSize = order_book["data"].back()["bids"][0][1].get<double>();
    double askPrice = order_book["data"].back()["asks"][0][0].get<double>();
    double askSize = order_book["data"].back()["asks"][0][1].get<double>();
    // Poco::DateTime time = Models::iso8601_to_datetime(order_book["data"].back()["timestamp"].get<std::string>());
    Models::MarketQuote mq(askPrice, askSize, bidPrice, bidSize, this->symbol.symbol, Poco::DateTime());
    this->market_quote(this, mq);
  });
  ws.connect_changed += Poco::delegate(this, &BitmexMarketDataGateway::subscribe_to_quote);
}

BitmexMarketDataGateway::~BitmexMarketDataGateway()
{
  ws.connect_changed -= Poco::delegate(this, &BitmexMarketDataGateway::subscribe_to_quote);
}

void BitmexMarketDataGateway::subscribe_to_quote(const void *, Models::ConnectivityStatus &cs)
{
  if (cs == Models::ConnectivityStatus::connected)
  {
    json j = {{"op", "subscribe"}, {"args", {this->order_book_channel}}};
    ws.send(j);
  }
}

BitmexOrderEntryGateway::BitmexOrderEntryGateway(BitmexHttp &http, BitmexWebsocket &ws, BitmexDeltaParser &parser, BitmexStore &store, BitmexSymbolProvider &symbol)
    : http(http), ws(ws), parser(parser), store(store), symbol(symbol)
{
  this->order_channel = "order:" + symbol.symbol;
  this->execution_channel = "execution:" + symbol.symbol;
  ws.set_handler(this->execution_handle, [this](json execution) {
    this->on_execution(this, execution);
  });
  ws.set_handler(this->order_handle, [this](json order) {
    this->on_order(this, order);
  });
  ws.connect_changed += Poco::delegate(this, &BitmexOrderEntryGateway::subscribe);
}

BitmexOrderEntryGateway::~BitmexOrderEntryGateway()
{
  ws.connect_changed -= Poco::delegate(this, &BitmexOrderEntryGateway::subscribe);
}

void BitmexOrderEntryGateway::subscribe(const void *, Models::ConnectivityStatus &cs)
{
  if (cs == Models::ConnectivityStatus::connected)
  {
    json j = {{"op", "subscribe"}, {"args", {this->order_channel, this->execution_channel}}};
    ws.send(j);
  }
}

std::string BitmexOrderEntryGateway::generate_client_id()
{
  return id_generator.createRandom().toString();
}

void BitmexOrderEntryGateway::batch_send_order(std::vector<Models::NewOrder> &orders)
{
  const std::string path = "/api/v1/order/bulk";
  const std::string verb = "POST";
  json payload;
  payload["orders"] = json::array();
  for (const auto &order : orders)
  {
    payload["orders"].push_back(order.to_json());
  }

  this->http.call(path, verb, payload).then([](pplx::task<json> previous_task) {
    try
    {
      previous_task.get();
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }
  });
  Poco::DateTime now;
  Poco::Timespan diff = now - orders[0].time;
  int ms = diff.totalMicroseconds();
  std::cout << "tick to trade: " << ms << "us" << std::endl;
}

void BitmexOrderEntryGateway::batch_cancel_order(std::vector<Models::CancelOrder> &cancels)
{
  const std::string path = "/api/v1/order";
  const std::string verb = "DELETE";
  json payload;
  payload["clOrdID"] = json::array();
  for (const auto &cancel : cancels)
  {
    payload["clOrdID"].push_back(cancel.clOrdID);
  }

  this->http.call(path, verb, payload).then([](pplx::task<json> previous_task) {
    try
    {
      previous_task.get();
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }
  });
}

void BitmexOrderEntryGateway::batch_replace_order(std::vector<Models::ReplaceOrder> &replaces)
{
  const std::string path = "/api/v1/order/bulk";
  const std::string verb = "PUT";
  json payload;
  payload["orders"] = json::array();
  for (const auto &replace : replaces)
  {
    payload["orders"].push_back(replace.to_json());
  }

  this->http.call(path, verb, payload).then([](pplx::task<json> previous_task) {
    try
    {
      previous_task.get();
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }
  });
  Poco::DateTime now;
  Poco::Timespan diff = now - replaces[0].time;
  int ms = diff.totalMicroseconds();
  std::cout << "tick to trade: " << ms << "us" << std::endl;
}

unsigned int BitmexOrderEntryGateway::cancel_all()
{
  const std::string path = "/api/v1/order/all";
  const std::string verb = "DELETE";
  json payload = {
      {"symbol", this->symbol.symbol}};

  unsigned int opens = 0;
  this->http.call(path, verb, payload)
      .then([&opens](json res) {
        opens = static_cast<unsigned int>(res.size());
      })
      .wait();

  return opens;
}

void BitmexOrderEntryGateway::on_order(const void *, json &order)
{
  // update store
  this->parser.onAction(order["action"], order["table"], this->symbol.symbol, this->store, order);
  // keep only open orders
  auto begin = this->store.data["order"][this->symbol.symbol].begin();
  auto end = this->store.data["order"][this->symbol.symbol].end();
  json opens = json::array();
  std::copy_if(begin, end, std::back_inserter(opens), [](json o) { return o["leavesQty"].get<long>() > 0; });
  opens.swap(this->store.data["order"][this->symbol.symbol]);
  long number_of_orders = this->store.data["order"][this->symbol.symbol].size();
  this->n_orders(this, number_of_orders);
}

Models::Side BitmexOrderEntryGateway::decode_side(const std::string &s)
{
  if (s == "Buy")
  {
    return Models::Side::Bid;
  }
  // if (s == "Sell")
  else
  {
    return Models::Side::Ask;
  }
}

Models::Liquidity BitmexOrderEntryGateway::decode_liquidity(const std::string &l)
{
  if (l == "AddedLiquidity")
  {
    return Models::Liquidity::Maker;
  }
  // if (l == "RemovedLiquidity")
  else
  {
    return Models::Liquidity::Taker;
  }
}

void BitmexOrderEntryGateway::on_execution(const void *, json &execution)
{
  for (const auto &e : execution["data"])
  {
    if (e["execType"].get<std::string>() == "Trade")
    {
      Models::Trade t(
          e["execID"].get<std::string>(),
          Models::iso8601_to_datetime(e["timestamp"].get<std::string>()),
          BitmexOrderEntryGateway::decode_side(e["side"].get<std::string>()),
          e["lastQty"].get<double>(),
          e["lastPx"].get<double>(),
          BitmexOrderEntryGateway::decode_liquidity(e["lastLiquidityInd"].get<std::string>()),
          e["homeNotional"].get<double>(),
          e["foreignNotional"].get<double>());

      this->trade(this, t);
    }
  }
}

std::optional<json> BitmexOrderEntryGateway::open_orders()
{
  if (this->store.data.contains("order"))
  {
    return this->store.data["order"][this->symbol.symbol];
  }
  else
  {
    return std::nullopt;
  }
}

BitmexPositionGateway::BitmexPositionGateway(BitmexWebsocket &ws, BitmexDeltaParser &parser, BitmexStore &store, BitmexSymbolProvider &symbol)
    : ws(ws), parser(parser), store(store), symbol(symbol)
{
  this->position_channel = "position:" + symbol.symbol;
  ws.set_handler(this->position_handle, [this](json position) {
    this->on_position(this, position);
  });
  ws.set_handler(this->margin_handle, [this](json margin) {
    this->on_margin(this, margin);
  });
  ws.connect_changed += Poco::delegate(this, &BitmexPositionGateway::subscribe);
}

BitmexPositionGateway::~BitmexPositionGateway()
{
  ws.connect_changed -= Poco::delegate(this, &BitmexPositionGateway::subscribe);
}

void BitmexPositionGateway::subscribe(const void *, Models::ConnectivityStatus &cs)
{
  if (cs == Models::ConnectivityStatus::connected)
  {
    json j = {{"op", "subscribe"}, {"args", {this->position_channel, this->margin_channel}}};
    ws.send(j);
  }
}

void BitmexPositionGateway::on_position(const void *, json &position)
{
  this->parser.onAction(position["action"], position["table"], this->symbol.symbol, this->store, position);
}

void BitmexPositionGateway::on_margin(const void *, json &margin)
{
  this->parser.onAction(margin["action"], margin["table"], this->symbol.symbol, this->store, margin);
}

std::optional<json> BitmexPositionGateway::get_latest_position()
{
  if (this->store.data.contains("position"))
  {
    return this->store.data["position"][this->symbol.symbol];
  }
  else
  {
    return std::nullopt;
  }
}

std::optional<json> BitmexPositionGateway::get_latest_margin()
{
  if (this->store.data.contains("margin") && (this->store.data["margin"][this->symbol.symbol].size() > 0))
  {
    auto end = this->store.data["margin"][this->symbol.symbol].end();
    return *(--end);
  }
  else
  {
    return std::nullopt;
  }
}

BitmexRateLimit::BitmexRateLimit()
{
}

BitmexRateLimit::~BitmexRateLimit()
{
}

bool BitmexRateLimit::is_rate_limited()
{
  if (this->is_limited && (Poco::DateTime() < this->next_reset))
  {
    return true;
  }
  else if (this->is_limited == true)
  {
    this->is_limited = false;
    return false;
  }
  else
  {
    return false;
  }
}

void BitmexRateLimit::update_rate_limit(int limit, int remain, Poco::DateTime next_reset)
{
  this->limit = limit;
  this->remain = remain;
  this->next_reset = next_reset;

  if (this->threshold > (static_cast<float>(this->remain) / static_cast<float>(this->limit)))
    this->is_limited = true;
}

BitmexDetailsGateway::BitmexDetailsGateway()
{
  this->pair = "XBTUSD";
  this->maker_fee = -0.00025;
  this->taker_fee = 0.00075;
  this->min_tick_increment = 0.5;
  this->min_size_increment = 1;
  this->face_value = 1.0;
  this->max_leverage = 100;
}

BitmexDetailsGateway::BitmexDetailsGateway(const std::string &c)
{
  if (c == "XBTUSD")
  {
    this->pair = c;
    this->min_tick_increment = 0.5;
  }
  else if (c == "ETHUSD")
  {
    this->pair = c;
    this->min_tick_increment = 0.05;
  }
  else if (c == "XRPUSD")
  {
    this->pair = c;
    this->min_tick_increment = 0.0001;
  }
  else if (c == "LTCUSD")
  {
    this->pair = c;
    this->min_tick_increment = 0.01;
  }
  else if (c == "BCHUSD")
  {
    this->pair = c;
    this->min_tick_increment = 0.05;
  }

  this->maker_fee = -0.00025;
  this->taker_fee = 0.00075;
  this->min_size_increment = 1;
  this->face_value = 1.0;
  this->max_leverage = 100;
}

BitmexCombinedGateway::BitmexCombinedGateway(BitmexDetailsGateway &base, BitmexMarketDataGateway &md, BitmexOrderEntryGateway &oe, BitmexPositionGateway &pg, BitmexRateLimit &rl)
    : base(base), md(md), oe(oe), pg(pg), rl(rl)
{
}