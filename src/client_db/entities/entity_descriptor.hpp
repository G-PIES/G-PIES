#pragma once

#include <sqlite3.h>
#include <string>

#include "client_db/client_db.hpp"

template <typename T, class... Args>
class EntityDescriptor {
 public:
  virtual std::string get_create_query() = 0;

  virtual void bind(sqlite3_stmt *, T &, Args &&...) = 0;

  void handle_create_error(sqlite3_stmt *stmt, const T &object) {
    std::string errmsg = get_create_error_message(object);
    sqlite3 *db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                            sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
  }

 protected:
  virtual std::string get_create_error_message(const T &) = 0;
};
