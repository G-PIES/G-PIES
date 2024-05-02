#pragma once

#include <sqlite3.h>
#include <string>

#include "client_db/client_db.hpp"

template <typename T, class... Args>
class EntityDescriptor {
 public:
  virtual std::string get_create_one_query() = 0;
  virtual std::string get_read_one_query() = 0;

  virtual void bind_create_one(sqlite3_stmt *, T &, Args &&...) = 0;

  virtual void read_row(sqlite3_stmt *, T &) = 0;

  void handle_create_one_error(sqlite3_stmt *stmt, const T &object) {
    std::string errmsg = get_create_one_error_message(object);
    sqlite3 *db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                            sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
  }

  void handle_read_one_error(sqlite3_stmt *stmt, const int sqlite_id) {
    std::string errmsg = get_read_one_error_message(sqlite_id);
    sqlite3 *db = sqlite3_db_handle(stmt);

    throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                            sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
  }

 protected:
  virtual std::string get_create_one_error_message(const T &) = 0;
  virtual std::string get_read_one_error_message(const int) = 0;
};
