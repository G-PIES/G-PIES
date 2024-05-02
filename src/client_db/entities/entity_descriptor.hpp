#pragma once

#include <sqlite3.h>
#include <string>

#include "client_db/client_db.hpp"

template <typename T, class... Args>
class EntityDescriptor {
 public:
  virtual std::string get_create_one_query() = 0;
  virtual std::string get_read_one_query() = 0;
  virtual std::string get_read_all_query() = 0;
  virtual std::string get_update_one_query() = 0;

  virtual void bind_create_one(sqlite3_stmt *, const T &, Args &&...) = 0;
  virtual void bind_update_one(sqlite3_stmt *, const T &) = 0;

  virtual void read_row(sqlite3_stmt *, T &) = 0;

  virtual std::string get_entity_name() = 0;
  virtual std::string get_entities_name() = 0;
  virtual std::string get_entity_description(const T &) { return ""; }
};
