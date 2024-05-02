#pragma once

#include <functional>
#include <sqlite3.h>
#include <string>
#include <vector>

#include "client_db/client_db.hpp"
#include "model/history_simulation.hpp"

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

  template <typename T>
  int execute_non_query(sqlite3_stmt *, T &, const std::function<void()> &);

  int execute_query(sqlite3_stmt *, const std::function<void()> &,
                    const std::function<void()> &);

  // CREATE
  template <typename T>
  int create_one(sqlite3_stmt *, void (*)(sqlite3_stmt *, const T &), T &);

  // READ
  template <typename T>
  int read_one(sqlite3_stmt *, void (*)(sqlite3_stmt *, T &, int),
               void (*)(sqlite3_stmt *, const int), const int, T &);
  template <typename T>
  int read_all(sqlite3_stmt *, void (*)(sqlite3_stmt *, T &, int),
               void (*)(sqlite3_stmt *), std::vector<T> &);

  // UPDATE / DELETE
  template <typename T>
  int update_one(sqlite3_stmt *, void (*)(sqlite3_stmt *, const T &),
                 const T &);

  // DELETE
  template <typename T>
  int delete_one(sqlite3_stmt *, void (*)(sqlite3_stmt *, const T &),
                 const T &);

  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  /*  COLUMN BINDING
      These are called by the public facing function to bind objects to SQLite
     statements.
  */

  static void bind_reactor(sqlite3_stmt *, const NuclearReactor &,
                           bool = false);
  static void bind_material(sqlite3_stmt *, const Material &, bool = false);
  static void bind_simulation(sqlite3_stmt *, const HistorySimulation &);

  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  /*  ROW CALLBACKS
      These callbacks are used as parameters in the read template functions.
     They handle the underlying logic of copying data from a retrieved SQLite
     row into an object.
  */

  static void row_read_reactor(sqlite3_stmt *, NuclearReactor &, int = 0);
  static void row_read_material(sqlite3_stmt *, Material &, int = 0);
  static void row_read_simulation(sqlite3_stmt *, HistorySimulation &, int = 0);

  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  /*  ERROR CALLBACKS
      These callbacks are used as parameters in all template functions to
     handle any errors that occur in the database queries.
  */

  static void err_create_reactor(sqlite3_stmt *, const NuclearReactor &);
  static void err_read_reactors(sqlite3_stmt *);
  static void err_read_reactor(sqlite3_stmt *, const int);
  static void err_update_reactor(sqlite3_stmt *, const NuclearReactor &);
  static void err_delete_reactor(sqlite3_stmt *, const NuclearReactor &);

  static void err_create_material(sqlite3_stmt *, const Material &);
  static void err_read_materials(sqlite3_stmt *);
  static void err_read_material(sqlite3_stmt *, const int);
  static void err_update_material(sqlite3_stmt *, const Material &);
  static void err_delete_material(sqlite3_stmt *, const Material &);

  static void err_create_simulation(sqlite3_stmt *, const HistorySimulation &);
  static void err_read_simulations(sqlite3_stmt *);
  static void err_read_simulation(sqlite3_stmt *, const int);
  static void err_delete_simulation(sqlite3_stmt *, const HistorySimulation &);

  // --------------------------------------------------------------------------------------------

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
