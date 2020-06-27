#pragma once

#include <iostream>
#include <vector>
#include "json.hpp"

#define MAX_STORE_LENGTH 1000 

using json = nlohmann::json;

struct BitmexStore
{
  json data;
  json keys;
};

class BitmexDeltaParser
{
public:
  BitmexDeltaParser();
  ~BitmexDeltaParser();

  unsigned int MAX_LEN = MAX_STORE_LENGTH;

  void onAction(const std::string &action, const std::string &table_name, const std::string &symbol, BitmexStore &store, const json &msg);

private:
  bool is_initialized(const std::string &table_name, const std::string &symbol, const BitmexStore &store);
  void replace(const std::string &table_name, const std::string &symbol, BitmexStore &store, const json &msg);
  void insert(const std::string &table_name, const std::string &symbol, BitmexStore &store, const json &msg);
  void update(const std::string &table_name, const std::string &symbol, BitmexStore &store, const json &msg);
  void remove(const std::string &table_name, const std::string &symbol, BitmexStore &store, const json &msg);

  bool item_keys_match(const std::vector<std::string> &keys, const json &store_item, const json &msg_item);
};