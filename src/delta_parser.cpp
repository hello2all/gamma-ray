#include "delta_parser.h"

BitmexDeltaParser::BitmexDeltaParser()
{
}

BitmexDeltaParser::~BitmexDeltaParser()
{
}

void BitmexDeltaParser::onAction(const std::string &action, const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg)
{
  // discard all deltas before getSymbol() call returns
  if (action != "partial" && !this->is_initialized(table_name, symbol, store))
    return;

  // handle partial action
  if (action == "partial")
  {
    this->replace(table_name, symbol, store, msg);
  }

  if ((action == "update" || action == "delete") && (store.keys[table_name].size() == 0))
  {
    throw std::runtime_error("The data in the store " + table_name + " is not keyed for " + action + "s.");
  }

  // handle insert action
  if (action == "insert")
  {
    this->insert(table_name, symbol, store, msg);
  }

  // handle update action
  if (action == "update")
  {
    this->update(table_name, symbol, store, msg);
  }

  // handle delete action
  if (action == "delete")
  {
    this->remove(table_name, symbol, store, msg);
  }
}

void BitmexDeltaParser::replace(const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg)
{
  if (!store.data.contains(table_name))
    store.data[table_name] = json::object();

  if ((!store.data[table_name].contains(symbol)) || msg["data"].size() == 0)
  {
    store.data[table_name][symbol] = msg["data"].size() == 0 ? json::array() : msg["data"];
  }

  store.keys[table_name][symbol] = msg["keys"];
}

void BitmexDeltaParser::insert(const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg)
{
  store.data[table_name][symbol].insert(store.data[table_name][symbol].end(), msg["data"].begin(), msg["data"].end());

  // TODO: trim data for none orderbook or order data to prevent excessive memory usage
}

void BitmexDeltaParser::update(const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg)
{
  const auto keys = store.keys[table_name][symbol].get<std::vector<std::string>>();

  // loop msg.data
  for (auto &msg_item : msg["data"])
  {
    // loop store.data
    for (auto &store_item : store.data[table_name][symbol])
    {
      // if keys match
      if (this->item_keys_match(keys, store_item, msg_item))
      {
        // assign msg_item values to store_item
        for (auto [key, value] : msg_item.items())
        {
          store_item[key] = value;
        }
      }
    }
  }
}

void BitmexDeltaParser::remove(const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg)
{
  std::vector<unsigned int> remove_queue;
  const auto keys = store.keys[table_name][symbol].get<std::vector<std::string>>();
  // loop msg.data
  for (auto &msg_item : msg["data"])
  {
    // loop store.data
    for (unsigned int i = 0; i < store.data[table_name][symbol].size(); i++)
    {
      // if keys match
      if (this->item_keys_match(keys, store.data[table_name][symbol][i], msg_item))
      {
        // note down index
        remove_queue.push_back(i);
        continue;
      }
    }
  }

  // erase noted indicies
  for (auto idx : remove_queue)
  {
    store.data[table_name][symbol].erase(idx);
  }
}

bool BitmexDeltaParser::is_initialized(const std::string &table_name, const std::string &symbol, BitmexStore store)
{
  return (store.data.contains(table_name) && store.data[table_name].contains(symbol));
}

bool BitmexDeltaParser::item_keys_match(std::vector<std::string> keys, json store_item, json msg_item)
{
  for (std::string &key : keys)
  {
    if (store_item[key] != msg_item[key])
      return false;
  }
  return true;
}