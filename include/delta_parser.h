#include <iostream>
#include <vector>
#include "json.hpp"

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

  void onAction(const std::string &action, const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg);

private:
  bool is_initialized(const std::string &table_name, const std::string &symbol, BitmexStore store);
  void replace(const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg);
  void insert(const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg);
  void update(const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg);
  void remove(const std::string &table_name, const std::string &symbol, BitmexStore &store, json msg);

  bool item_keys_match(std::vector<std::string> keys, json store_item, json msg_item);
};