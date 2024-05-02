#pragma once

#include <functional>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "client_db/client_db.hpp"

class ClientDbImpl {
 public:
  // The SQLite database reference.
  sqlite3 *db;

  // The path to the SQLite database which |db| will reference.
  std::string path;

  // Initializes and opens the SQLite database, followed by any needed schema
  // maintenance. |sqlite_code| can optionally be retrieved. Returns true on
  // success.
  bool init(int *sqlite_code = nullptr);

  // Drops all tables in the SQLite database.
  // *** WARNING *** All data will be lost.
  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool clear(int *sqlite_code = nullptr);

  // --------------------------------------------------------------------------------------------
  /*  TEMPLATE FUNCTIONS
      These are called by the public facing functions, implementing the basic
     logic of statement execution and exception handling for create, read,
     update and delete. All functions return the resulting SQLite status code.
  */

  template <typename TEntityDescriptor, typename T, class... Args>
  bool create_one(T &, int *, Args&&...);

  template <typename TEntityDescriptor, typename T>
  bool read_one(const int, T &, int *);

  template <typename TEntityDescriptor, typename T>
  bool read_all(std::vector<T> &, int *);

  template <typename TEntityDescriptor, typename T>
  bool update_one(const T &, int *);

  template <typename TEntityDescriptor, typename T>
  bool delete_one(const T &, int *);

  void throw_error(sqlite3_stmt *,
                   const std::string &, const std::string &,
                   const std::string &,
                   const std::string & = "", const std::string & = "");

  int execute_non_query(sqlite3_stmt *, const std::function<void()> &);

  int execute_query(sqlite3_stmt *, const std::function<void()> &,
                    const std::function<void()> &);

  // --------------------------------------------------------------------------------------------
  // UTILITIES

  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool open(int *sqlite_code = nullptr);

  // |sqlite_code| can optionally be retrieved.
  // Returns true on success.
  bool close(int *sqlite_code = nullptr);

  // Returns true if the SQLite database is currently open.
  bool is_open();

  // Makes a directory for managing the database file
  void make_db_dir();

  // Returns the number of rows effected by the latest query.
  int changes();

  // Returns the last inserted row id, regardless of the table.
  int last_insert_rowid(int &);

  // Returns true if |sqlite_code| reprents a successful result.
  // https://www.sqlite.org/rescode.html
  static bool is_sqlite_success(const int sqlite_code);

  // Returns true if |sqlite_code| reprents a successful result.
  // https://www.sqlite.org/rescode.html
  static bool is_sqlite_error(const int sqlite_code);

  // Returns true if |sqlite_id| is a value primary key value.
  // NOTE: This does NOT check if |sqlite_id| exists within the database.
  static bool is_valid_sqlite_id(const int sqlite_id);

  // --------------------------------------------------------------------------------------------

  // Specify the |db_path| where the SQLite database resides
  // |lazy| will wait to open the database until |init| is called.
  ClientDbImpl(const std::string &db_path = DEV_DEFAULT_CLIENT_DB_PATH,
               const bool lazy = true);
  ~ClientDbImpl();
};
