#include "client_db_impl.hpp"

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
  #include <direct.h>
#else
  #include <sys/stat.h>
#endif

#include <utility>

#include "client_db/client_db.hpp"
#include "db_queries.hpp"
#include "entities/template_instantiation.hpp"
#include "utils/gpies_exception.hpp"

bool ClientDbImpl::init(int *sqlite_result_code) {
  int sqlite_code;
  char *sqlite_errmsg;
  sqlite_code = sqlite3_initialize();
  if (is_sqlite_error(sqlite_code))
    throw ClientDbException("Failed to initialize database.",
                            sqlite3_errmsg(db), sqlite_code);

  if (!db) open();

  sqlite_code = sqlite3_exec(db, db_queries::init.c_str(), nullptr, nullptr,
                             &sqlite_errmsg);
  if (is_sqlite_error(sqlite_code))
    throw ClientDbException("Failed to initialize database.", sqlite_errmsg,
                            sqlite_code, db_queries::init);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDbImpl::clear(int *sqlite_result_code) {
  if (!db) open();

  int sqlite_code;
  char *sqlite_errmsg = nullptr;

  sqlite_code =
      sqlite3_exec(db, db_queries::clear.c_str(), nullptr, nullptr, nullptr);
  if (is_sqlite_error(sqlite_code))
    throw ClientDbException("Failed to clear database.", sqlite_errmsg,
                            sqlite_code, db_queries::clear);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

// --------------------------------------------------------------------------------------------
// TEMPLATE FUNCTIONS
// --------------------------------------------------------------------------------------------
template <typename TEntityDescriptor, typename T, class... Args>
bool ClientDbImpl::create_one(
    T &object,
    int *sqlite_result_code,
    Args&&... args) {

  TEntityDescriptor descriptor = TEntityDescriptor();

  if (is_valid_sqlite_id(object.sqlite_id))
    throw_error(nullptr, "Failed to create", "It already exists",
                descriptor.get_entity_name(),
                descriptor.get_entity_description(object));

  if (!db) open();

  int result;
  sqlite3_stmt *stmt;

  std::basic_string<char> query = descriptor.get_create_one_query();
  result = sqlite3_prepare_v2(db, query.c_str(), query.size(), &stmt, nullptr);
  if (is_sqlite_error(result))
    throw_error(stmt, "Failed to create", "",
                descriptor.get_entity_name(),
                descriptor.get_entity_description(object));

  descriptor.bind_create_one(stmt, object, std::forward<Args>(args)...);

  result = execute_non_query(stmt,
    [stmt, &descriptor, &object, this]() {
      throw_error(stmt, "Failed to create", "",
                  descriptor.get_entity_name(),
                  descriptor.get_entity_description(object));
  });

  int last_insert_result = last_insert_rowid(object.sqlite_id);
  if (is_sqlite_error(last_insert_result))
    throw_error(stmt, "Failed to create", "",
                descriptor.get_entity_name(),
                descriptor.get_entity_description(object));

  if (sqlite_result_code) *sqlite_result_code = result;
  return is_sqlite_success(result);
}

template <typename TEntityDescriptor, typename T>
bool ClientDbImpl::read_one(
    const int sqlite_id,
    T &object,
    int *sqlite_result_code) {

  TEntityDescriptor descriptor = TEntityDescriptor();

  if (!is_valid_sqlite_id(sqlite_id))
    throw_error(nullptr, "Failed to read", "Invalid id.",
                descriptor.get_entity_name(),
                "with id " + std::to_string(sqlite_id));

  if (!db) open();

  int result;
  sqlite3_stmt *stmt;

  std::basic_string<char> query = descriptor.get_read_one_query();
  result = sqlite3_prepare_v2(db, query.c_str(), query.size(), &stmt, nullptr);
  if (is_sqlite_error(result))
    throw_error(stmt, "Failed to read", "",
                descriptor.get_entity_name(),
                "with id " + std::to_string(sqlite_id));

  result = sqlite3_bind_int(stmt, 1, sqlite_id);
  if (is_sqlite_error(result))
    throw_error(stmt, "Failed to read", "",
                descriptor.get_entity_name(),
                "with id " + std::to_string(sqlite_id));

  result = execute_query(stmt,
    [stmt, &descriptor, &object]() {
      descriptor.read_row(stmt, object);
    },
    [stmt, &descriptor, sqlite_id, this]() {
      throw_error(stmt, "Failed to read", "",
                  descriptor.get_entity_name(),
                  "with id " + std::to_string(sqlite_id));
    });

  if (sqlite_result_code) *sqlite_result_code = result;
  return is_sqlite_success(result) && is_valid_sqlite_id(object.sqlite_id);
}

template <typename TEntityDescriptor, typename T>
bool ClientDbImpl::read_all(std::vector<T> &objects, int *sqlite_result_code) {
  TEntityDescriptor descriptor = TEntityDescriptor();
  std::basic_string<char> query = descriptor.get_read_all_query();

  if (!db) open();

  int result;
  sqlite3_stmt *stmt;

  result = sqlite3_prepare_v2(db, query.c_str(), query.size(), &stmt, nullptr);
  if (is_sqlite_error(result))
    throw_error(stmt, "Failed to read", "",
                descriptor.get_entities_name());

  result = execute_query(stmt,
    [stmt, &descriptor, &objects]() {
      T object;
      descriptor.read_row(stmt, object);
      objects.push_back(object);
    },
    [stmt, &descriptor, this]() {
      throw_error(stmt, "Failed to read", "",
                  descriptor.get_entities_name());
    });

  if (sqlite_result_code) *sqlite_result_code = result;
  return is_sqlite_success(result);
}

template <typename TEntityDescriptor, typename T>
bool ClientDbImpl::update_one(
    const T &object,
    int *sqlite_result_code) {

  TEntityDescriptor descriptor = TEntityDescriptor();

  if (!is_valid_sqlite_id(object.sqlite_id))
    throw_error(nullptr, "Failed to update", "Invalid id.",
                descriptor.get_entity_name(),
                "with id " + std::to_string(object.sqlite_id));

  if (!db) open();

  int result;
  sqlite3_stmt *stmt;

  std::basic_string<char> query = descriptor.get_update_one_query();
  result = sqlite3_prepare_v2(db, query.c_str(), query.size(), &stmt, nullptr);
  if (is_sqlite_error(result))
    throw_error(stmt, "Failed to update", "",
                descriptor.get_entity_name(),
                "with id " + std::to_string(object.sqlite_id));

  descriptor.bind_update_one(stmt, object);

  result = execute_non_query(stmt,
    [stmt, &descriptor, &object, this]() {
      throw_error(stmt, "Failed to update", "",
                  descriptor.get_entity_name(),
                  "with id " + std::to_string(object.sqlite_id));
    });

  if (sqlite_result_code) *sqlite_result_code = result;
  return is_sqlite_success(result);
}

template <typename TEntityDescriptor, typename T>
bool ClientDbImpl::delete_one(
    const T &object,
    int *sqlite_result_code) {

  TEntityDescriptor descriptor = TEntityDescriptor();

  if (!is_valid_sqlite_id(object.sqlite_id))
    throw_error(nullptr, "Failed to delete", "Invalid id.",
                descriptor.get_entity_name(),
                descriptor.get_entity_description(object),
                "with id " + std::to_string(object.sqlite_id));

  if (!db) open();

  int result;
  sqlite3_stmt *stmt;

  std::basic_string<char> query = descriptor.get_delete_one_query();
  result = sqlite3_prepare_v2(db, query.c_str(), query.size(), &stmt, nullptr);
  if (is_sqlite_error(result))
    throw_error(stmt, "Failed to delete", "",
                descriptor.get_entity_name(),
                descriptor.get_entity_description(object),
                "with id " + std::to_string(object.sqlite_id));

  result = sqlite3_bind_int(stmt, 1, object.sqlite_id);
  if (is_sqlite_error(result))
    throw_error(stmt, "Failed to delete", "",
                descriptor.get_entity_name(),
                descriptor.get_entity_description(object),
                "with id " + std::to_string(object.sqlite_id));

  result = execute_non_query(stmt,
    [stmt, &descriptor, &object, this]() {
      throw_error(stmt, "Failed to delete", "",
                  descriptor.get_entity_name(),
                  descriptor.get_entity_description(object),
                  "with id " + std::to_string(object.sqlite_id));
    });

  if (sqlite_result_code) *sqlite_result_code = result;
  return is_sqlite_success(result);
}

void ClientDbImpl::throw_error(
    sqlite3_stmt *stmt, const std::string &error, const std::string &reason,
    const std::string &entity_name,
    const std::string &entity_description1,
    const std::string &entity_description2) {

  std::string message = error + " " + entity_name;
  if (entity_description1.length() > 0) {
    message += " " + entity_description1;
  }
  if (entity_description2.length() > 0) {
    message += " " + entity_description2;
  }
  if (reason.length() > 0) {
    message += ". " + reason;
  }
  message += ".";

  if (stmt) {
    throw ClientDbException(message.c_str(), sqlite3_errmsg(db),
                            sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
  } else {
    throw ClientDbException(message.c_str());
  }
}

int ClientDbImpl::execute_non_query(
    sqlite3_stmt *stmt,
    const std::function<void()> &error_callback) {
  int sqlite_code;

  do {
    sqlite_code = sqlite3_step(stmt);
    if (is_sqlite_error(sqlite_code)) error_callback();
  } while (SQLITE_DONE != sqlite_code);

  sqlite3_finalize(stmt);
  return sqlite_code;
}

int ClientDbImpl::execute_query(
    sqlite3_stmt *stmt,
    const std::function<void()> &row_callback,
    const std::function<void()> &error_callback) {
  int sqlite_code;

  do {
    if (SQLITE_ROW == (sqlite_code = sqlite3_step(stmt)))
      row_callback();
    else if (is_sqlite_error(sqlite_code))
      error_callback();
  } while (sqlite_code != SQLITE_DONE);

  sqlite3_finalize(stmt);
  return sqlite_code;
}

// --------------------------------------------------------------------------------------------
// UTILITIES
// --------------------------------------------------------------------------------------------

bool ClientDbImpl::open(int *sqlite_result_code) {
  int sqlite_code;

  make_db_dir();

  std::string full_db_path = path + "/" + DB_NAME;

  sqlite_code = sqlite3_open(full_db_path.c_str(), &db);
  if (is_sqlite_error(sqlite_code))
    throw ClientDbException("Failed to open local database.",
                            sqlite3_errmsg(db), sqlite_code);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDbImpl::close(int *sqlite_result_code) {
  if (!db) return 0;

  int sqlite_code;

  sqlite_code = sqlite3_close(db);
  if (is_sqlite_error(sqlite_code))
    throw ClientDbException("Failed to close database.", sqlite3_errmsg(db),
                            sqlite_code);
  else
    db = nullptr;

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDbImpl::is_open() { return db; }

void ClientDbImpl::make_db_dir() {
  int err = 0;

#if defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)
  err = _mkdir(path.c_str());  // can be used on Windows
#else
  const mode_t n_mode = 0733;
  err = mkdir(path.c_str(), n_mode);  // can be used on non-Windows
#endif

  if (err) {
    if (EEXIST != errno) {
      std::string err_msg =
          "Failed to provision database with directory: " + path +
          "\nError code: " + std::to_string(err);
      throw ClientDbException(err_msg.c_str());
    }
  }
}

int ClientDbImpl::changes() { return sqlite3_changes(db); }

int ClientDbImpl::last_insert_rowid(int &sqlite_id) {
  if (!db) return -1;

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(db, db_queries::last_insert_rowid.c_str(),
                         db_queries::last_insert_rowid.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code))
    throw ClientDbException("Failed to retrieve database row id.",
                            sqlite3_errmsg(db), sqlite3_errcode(db));

  sqlite_code = sqlite3_step(stmt);
  if (is_sqlite_error(sqlite_code))
    throw ClientDbException("Failed to retrieve database row id.",
                            sqlite3_errmsg(db), sqlite3_errcode(db));

  sqlite_id = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);
  return sqlite_code;
}

bool ClientDbImpl::is_sqlite_success(const int sqlite_code) {
  // sqlite3 error handling: https://www.sqlite.org/rescode.html
  return SQLITE_OK == sqlite_code || SQLITE_ROW == sqlite_code ||
         SQLITE_DONE == sqlite_code;
}

bool ClientDbImpl::is_sqlite_error(const int sqlite_code) {
  // sqlite3 error handling: https://www.sqlite.org/rescode.html
  return SQLITE_OK != sqlite_code && SQLITE_ROW != sqlite_code &&
         SQLITE_DONE != sqlite_code;
}

bool ClientDbImpl::is_valid_sqlite_id(const int sqlite_id) {
  return 0 < sqlite_id;
}

ClientDbImpl::ClientDbImpl(const std::string &db_path, const bool lazy)
    : path(db_path) {
  if (lazy)
    db = nullptr;
  else
    open();
}

ClientDbImpl::~ClientDbImpl() {
  close();
  sqlite3_shutdown();
}
