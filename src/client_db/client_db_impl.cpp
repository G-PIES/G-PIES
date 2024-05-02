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
#include "model/history_simulation.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/blob_converter.hpp"
#include "utils/gpies_exception.hpp"

#define INSTANTIATE_TEMPALTES(T) \
template int ClientDbImpl::create_one(sqlite3_stmt *, \
                                      void (*)(sqlite3_stmt *, const T &), \
                                      T &);\
template int ClientDbImpl::read_one(sqlite3_stmt *, \
                                    void (*)(sqlite3_stmt *, T &, int), \
                                    void (*)(sqlite3_stmt *, const int), \
                                    const int, T &);\
template int ClientDbImpl::read_all(sqlite3_stmt *, \
                                    void (*)(sqlite3_stmt *, T &, int), \
                                    void (*)(sqlite3_stmt *), \
                                    std::vector<T> &);\
template int ClientDbImpl::update_one(sqlite3_stmt *, \
                                      void (*)(sqlite3_stmt *, const T &), \
                                      const T &);\
template int ClientDbImpl::delete_one(sqlite3_stmt *, \
                                      void (*)(sqlite3_stmt *, const T &), \
                                      const T &);

INSTANTIATE_TEMPALTES(HistorySimulation)
INSTANTIATE_TEMPALTES(Material)
INSTANTIATE_TEMPALTES(NuclearReactor)

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

template <typename T>
int ClientDbImpl::create_one(sqlite3_stmt *stmt,
                             void (*err_callback)(sqlite3_stmt *, const T &),
                             T &object) {
  int sqlite_code;

  do {
    sqlite_code = sqlite3_step(stmt);
    if (is_sqlite_error(sqlite_code)) err_callback(stmt, object);
  } while (SQLITE_DONE != sqlite_code);

  sqlite_code = last_insert_rowid(object.sqlite_id);
  if (is_sqlite_error(sqlite_code)) err_callback(stmt, object);

  sqlite3_finalize(stmt);
  return sqlite_code;
}

template <typename T>
int ClientDbImpl::read_one(sqlite3_stmt *stmt,
                       void (*row_callback)(sqlite3_stmt *, T &, int),
                       void (*err_callback)(sqlite3_stmt *, const int),
                       const int sqlite_id, T &object) {
  int sqlite_code;

  sqlite_code = sqlite3_bind_int(stmt, 1, sqlite_id);
  if (is_sqlite_error(sqlite_code)) err_callback(stmt, sqlite_id);

  do {
    if (SQLITE_ROW == (sqlite_code = sqlite3_step(stmt)))
      row_callback(stmt, object, 0);
    else if (is_sqlite_error(sqlite_code))
      err_callback(stmt, sqlite_id);
  } while (sqlite_code != SQLITE_DONE);

  sqlite3_finalize(stmt);
  return sqlite_code;
}

template <typename T>
int ClientDbImpl::read_all(sqlite3_stmt *stmt,
                       void (*row_callback)(sqlite3_stmt *, T &, int),
                       void (*err_callback)(sqlite3_stmt *),
                       std::vector<T> &objects) {
  int sqlite_code;

  do {
    if (SQLITE_ROW == (sqlite_code = sqlite3_step(stmt))) {
      T obj;
      row_callback(stmt, obj, 0);
      objects.push_back(obj);
    } else if (is_sqlite_error(sqlite_code)) {
      err_callback(stmt);
    }
  } while (sqlite_code != SQLITE_DONE);

  sqlite3_finalize(stmt);
  return sqlite_code;
}

template <typename T>
int ClientDbImpl::update_one(sqlite3_stmt *stmt,
                         void (*err_callback)(sqlite3_stmt *, const T &),
                         const T &object) {
  int sqlite_code;

  do {
    sqlite_code = sqlite3_step(stmt);
    if (is_sqlite_error(sqlite_code)) err_callback(stmt, object);
  } while (SQLITE_DONE != sqlite_code);

  sqlite3_finalize(stmt);
  return sqlite_code;
}

template <typename T>
int ClientDbImpl::delete_one(sqlite3_stmt *stmt,
                         void (*err_callback)(sqlite3_stmt *, const T &),
                         const T &object) {
  int sqlite_code;
  sqlite3_bind_int(stmt, 1, object.sqlite_id);

  do {
    sqlite_code = sqlite3_step(stmt);
    if (is_sqlite_error(sqlite_code)) err_callback(stmt, object);
  } while (SQLITE_DONE != sqlite_code);

  sqlite3_finalize(stmt);
  return sqlite_code;
}

// --------------------------------------------------------------------------------------------
// COLUMN BINDING
// --------------------------------------------------------------------------------------------

void ClientDbImpl::bind_reactor(sqlite3_stmt *stmt,
                                const NuclearReactor &reactor,
                                bool is_preset) {
  sqlite3_bind_text(stmt, 1, reactor.species.c_str(), reactor.species.length(),
                    nullptr);
  sqlite3_bind_double(stmt, 2, reactor.get_flux());
  sqlite3_bind_double(stmt, 3, reactor.get_temperature());
  sqlite3_bind_double(stmt, 4, reactor.get_recombination());
  sqlite3_bind_double(stmt, 5, reactor.get_i_bi());
  sqlite3_bind_double(stmt, 6, reactor.get_i_tri());
  sqlite3_bind_double(stmt, 7, reactor.get_i_quad());
  sqlite3_bind_double(stmt, 8, reactor.get_v_bi());
  sqlite3_bind_double(stmt, 9, reactor.get_v_tri());
  sqlite3_bind_double(stmt, 10, reactor.get_v_quad());
  sqlite3_bind_double(stmt, 11, reactor.get_dislocation_density_evolution());

  if (is_valid_sqlite_id(reactor.sqlite_id)) {
    // update
    sqlite3_bind_int(stmt, 12, reactor.sqlite_id);
  } else {
    // create
    sqlite3_bind_text(stmt, 12, reactor.creation_datetime.c_str(),
                      reactor.creation_datetime.length(), nullptr);
    sqlite3_bind_int(stmt, 13, static_cast<int>(is_preset));
  }
}

void ClientDbImpl::bind_material(sqlite3_stmt *stmt, const Material &material,
                             bool is_preset) {
  sqlite3_bind_text(stmt, 1, material.species.c_str(),
                    material.species.length(), nullptr);
  sqlite3_bind_double(stmt, 2, material.get_i_migration());
  sqlite3_bind_double(stmt, 3, material.get_v_migration());
  sqlite3_bind_double(stmt, 4, material.get_i_diffusion_0());
  sqlite3_bind_double(stmt, 5, material.get_v_diffusion_0());
  sqlite3_bind_double(stmt, 6, material.get_i_formation());
  sqlite3_bind_double(stmt, 7, material.get_v_formation());
  sqlite3_bind_double(stmt, 8, material.get_i_binding());
  sqlite3_bind_double(stmt, 9, material.get_v_binding());
  sqlite3_bind_double(stmt, 10, material.get_recombination_radius());
  sqlite3_bind_double(stmt, 11, material.get_i_loop_bias());
  sqlite3_bind_double(stmt, 12, material.get_i_dislocation_bias());
  sqlite3_bind_double(stmt, 13, material.get_i_dislocation_bias_param());
  sqlite3_bind_double(stmt, 14, material.get_v_loop_bias());
  sqlite3_bind_double(stmt, 15, material.get_v_dislocation_bias());
  sqlite3_bind_double(stmt, 16, material.get_v_dislocation_bias_param());
  sqlite3_bind_double(stmt, 17, material.get_dislocation_density_0());
  sqlite3_bind_double(stmt, 18, material.get_grain_size());
  sqlite3_bind_double(stmt, 19, material.get_lattice_param());
  sqlite3_bind_double(stmt, 20, material.get_burgers_vector());
  sqlite3_bind_double(stmt, 21, material.get_atomic_volume());

  if (is_valid_sqlite_id(material.sqlite_id)) {
    // update
    sqlite3_bind_int(stmt, 22, material.sqlite_id);
  } else {
    // create
    sqlite3_bind_text(stmt, 22, material.creation_datetime.c_str(),
                      material.creation_datetime.length(), nullptr);
    sqlite3_bind_int(stmt, 23, static_cast<int>(is_preset));
  }
}

void ClientDbImpl::bind_simulation(sqlite3_stmt *stmt,
                               const HistorySimulation &simulation) {
  std::vector<char> interstitials_blob =
      BlobConverter::to_blob(simulation.cd_state.interstitials);
  std::vector<char> vacancies_blob =
      BlobConverter::to_blob(simulation.cd_state.vacancies);

  sqlite3_bind_int(stmt, 1,
                   static_cast<int>(simulation.max_cluster_size));
  sqlite3_bind_double(stmt, 2, static_cast<double>(simulation.simulation_time));
  sqlite3_bind_double(stmt, 3, static_cast<double>(simulation.time_delta));
  sqlite3_bind_int(stmt, 4, simulation.reactor.sqlite_id);
  sqlite3_bind_int(stmt, 5, simulation.material.sqlite_id);
  sqlite3_bind_blob(stmt, 6, interstitials_blob.data(),
                    interstitials_blob.size(), SQLITE_TRANSIENT);
  sqlite3_bind_blob(stmt, 7, vacancies_blob.data(), vacancies_blob.size(),
                    SQLITE_TRANSIENT);
  sqlite3_bind_double(
      stmt, 8, static_cast<double>(simulation.cd_state.dislocation_density));
  sqlite3_bind_double(stmt, 9, static_cast<double>(simulation.cd_state.dpa));
  sqlite3_bind_text(stmt, 10, simulation.creation_datetime.c_str(),
                    simulation.creation_datetime.length(), nullptr);
}

// --------------------------------------------------------------------------------------------
// ROW CALLBACKS
// --------------------------------------------------------------------------------------------

void ClientDbImpl::row_read_reactor(sqlite3_stmt *stmt, NuclearReactor &reactor,
                                int col_offset) {
  reactor.sqlite_id =
      static_cast<int>(sqlite3_column_int(stmt, col_offset + 0));
  reactor.creation_datetime =
      reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_offset + 1));
  reactor.species =
      reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_offset + 2));
  reactor.set_flux((gp_float)sqlite3_column_double(stmt, col_offset + 3));
  reactor.set_temperature(
      (gp_float)sqlite3_column_double(stmt, col_offset + 4));
  reactor.set_recombination(
      (gp_float)sqlite3_column_double(stmt, col_offset + 5));
  reactor.set_i_bi((gp_float)sqlite3_column_double(stmt, col_offset + 6));
  reactor.set_i_tri((gp_float)sqlite3_column_double(stmt, col_offset + 7));
  reactor.set_i_quad((gp_float)sqlite3_column_double(stmt, col_offset + 8));
  reactor.set_v_bi((gp_float)sqlite3_column_double(stmt, col_offset + 9));
  reactor.set_v_tri((gp_float)sqlite3_column_double(stmt, col_offset + 10));
  reactor.set_v_quad((gp_float)sqlite3_column_double(stmt, col_offset + 11));
  reactor.set_dislocation_density_evolution(
      (gp_float)sqlite3_column_double(stmt, col_offset + 12));
}

void ClientDbImpl::row_read_material(sqlite3_stmt *stmt, Material &material,
                                 int col_offset) {
  material.sqlite_id =
      static_cast<int>(sqlite3_column_int(stmt, col_offset + 0));
  material.creation_datetime =
      reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_offset + 1));
  material.species =
      reinterpret_cast<const char *>(sqlite3_column_text(stmt, col_offset + 2));
  material.set_i_migration(
      (gp_float)sqlite3_column_double(stmt, col_offset + 3));
  material.set_v_migration(
      (gp_float)sqlite3_column_double(stmt, col_offset + 4));
  material.set_i_diffusion_0(
      (gp_float)sqlite3_column_double(stmt, col_offset + 5));
  material.set_v_diffusion_0(
      (gp_float)sqlite3_column_double(stmt, col_offset + 6));
  material.set_i_formation(
      (gp_float)sqlite3_column_double(stmt, col_offset + 7));
  material.set_v_formation(
      (gp_float)sqlite3_column_double(stmt, col_offset + 8));
  material.set_i_binding((gp_float)sqlite3_column_double(stmt, col_offset + 9));
  material.set_v_binding(
      (gp_float)sqlite3_column_double(stmt, col_offset + 10));
  material.set_recombination_radius(
      (gp_float)sqlite3_column_double(stmt, col_offset + 11));
  material.set_i_loop_bias(
      (gp_float)sqlite3_column_double(stmt, col_offset + 12));
  material.set_i_dislocation_bias(
      (gp_float)sqlite3_column_double(stmt, col_offset + 13));
  material.set_i_dislocation_bias_param(
      (gp_float)sqlite3_column_double(stmt, col_offset + 14));
  material.set_v_loop_bias(
      (gp_float)sqlite3_column_double(stmt, col_offset + 15));
  material.set_v_dislocation_bias(
      (gp_float)sqlite3_column_double(stmt, col_offset + 16));
  material.set_v_dislocation_bias_param(
      (gp_float)sqlite3_column_double(stmt, col_offset + 17));
  material.set_dislocation_density_0(
      (gp_float)sqlite3_column_double(stmt, col_offset + 18));
  material.set_grain_size(
      (gp_float)sqlite3_column_double(stmt, col_offset + 19));
  material.set_lattice_param(
      (gp_float)sqlite3_column_double(stmt, col_offset + 20));
  material.set_burgers_vector(
      (gp_float)sqlite3_column_double(stmt, col_offset + 21));
  material.set_atomic_volume(
      (gp_float)sqlite3_column_double(stmt, col_offset + 22));
}

// NOTE: col_offset is not really needed here, it is merely added to have
// matching arg signatures for the other row reading functions. It maybe be
// better to implement row reading by token, to completely avoid column
// offsetting.
void ClientDbImpl::row_read_simulation(sqlite3_stmt *stmt,
                                   HistorySimulation &simulation,
                                   int col_offset) {
  // satisfy the compiler by pretending to "use" the argument
  col_offset = 0;
  simulation.sqlite_id = sqlite3_column_int(stmt, col_offset + 0);

  simulation.creation_datetime = (char *)sqlite3_column_text(stmt, 1);
  simulation.max_cluster_size =
      static_cast<size_t>(sqlite3_column_int(stmt, 2));
  simulation.simulation_time =
      static_cast<gp_float>(sqlite3_column_double(stmt, 3));
  simulation.time_delta = static_cast<gp_float>(sqlite3_column_double(stmt, 4));

  // columns 5 & 6 are for reactor & material foreign keys

  const void *interstitials_blob = sqlite3_column_blob(stmt, 7);
  int interstitials_blob_size = sqlite3_column_bytes(stmt, 7);
  std::vector<char> interstitials_vec(
      static_cast<const char *>(interstitials_blob),
      static_cast<const char *>(interstitials_blob) + interstitials_blob_size);
  simulation.cd_state.interstitials =
      BlobConverter::from_blob(interstitials_vec);

  const void *vacancies_blob = sqlite3_column_blob(stmt, 8);
  int vacancies_blob_size = sqlite3_column_bytes(stmt, 8);
  std::vector<char> vacancies_vec(
      static_cast<const char *>(vacancies_blob),
      static_cast<const char *>(vacancies_blob) + vacancies_blob_size);
  simulation.cd_state.vacancies = BlobConverter::from_blob(vacancies_vec);

  simulation.cd_state.dislocation_density = sqlite3_column_double(stmt, 9);

  simulation.cd_state.dpa = sqlite3_column_double(stmt, 10);

  row_read_reactor(stmt, simulation.reactor, 11);
  row_read_material(stmt, simulation.material, 25);
}

// --------------------------------------------------------------------------------------------
// ERROR CALLBACKS
// --------------------------------------------------------------------------------------------

void ClientDbImpl::err_create_reactor(sqlite3_stmt *stmt,
                                  const NuclearReactor &reactor) {
  std::string errmsg = "Failed to create reactor \"" + reactor.species + "\".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_read_reactors(sqlite3_stmt *stmt) {
  sqlite3 *db = sqlite3_db_handle(stmt);
  throw ClientDbException("Failed to read reactors.", sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_read_reactor(sqlite3_stmt *stmt, const int sqlite_id) {
  std::string errmsg =
      "Failed to read reactor w/ id " + std::to_string(sqlite_id) + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_update_reactor(sqlite3_stmt *stmt,
                                  const NuclearReactor &reactor) {
  std::string errmsg = "Failed to update reactor \"" + reactor.species +
                       "\" w/ id " + std::to_string(reactor.sqlite_id) + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_delete_reactor(sqlite3_stmt *stmt,
                                  const NuclearReactor &reactor) {
  std::string errmsg = "Failed to delete reactor \"" + reactor.species +
                       "\" w/ id " + std::to_string(reactor.sqlite_id) + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_create_material(sqlite3_stmt *stmt,
                                   const Material &material) {
  std::string errmsg =
      "Failed to create material \"" + material.species + "\".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_read_materials(sqlite3_stmt *stmt) {
  sqlite3 *db = sqlite3_db_handle(stmt);
  throw ClientDbException("Failed to read materials.", sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_read_material(sqlite3_stmt *stmt, const int sqlite_id) {
  std::string errmsg =
      "Failed to read material w/ id " + std::to_string(sqlite_id) + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_update_material(sqlite3_stmt *stmt,
                                   const Material &material) {
  std::string errmsg = "Failed to update material \"" + material.species +
                       "\" w/ id " + std::to_string(material.sqlite_id) + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_delete_material(sqlite3_stmt *stmt,
                                   const Material &material) {
  std::string errmsg = "Failed to delete material \"" + material.species +
                       "\" w/ id " + std::to_string(material.sqlite_id) + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_create_simulation(sqlite3_stmt *stmt,
                                     const HistorySimulation &simulation) {
  std::string errmsg =
      "Failed to create simulation @ " + simulation.creation_datetime + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_read_simulations(sqlite3_stmt *stmt) {
  sqlite3 *db = sqlite3_db_handle(stmt);
  throw ClientDbException("Failed to read simulations.", sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_read_simulation(sqlite3_stmt *stmt,
                                       const int sqlite_id) {
  std::string errmsg =
      "Failed to read simulation w/ id " + std::to_string(sqlite_id) + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
}

void ClientDbImpl::err_delete_simulation(sqlite3_stmt *stmt,
                                     const HistorySimulation &simulation) {
  std::string errmsg = "Failed to delete simulation w/ id " +
                       std::to_string(simulation.sqlite_id) + ".";
  sqlite3 *db = sqlite3_db_handle(stmt);

  throw ClientDbException(errmsg.c_str(), sqlite3_errmsg(db),
                          sqlite3_errcode(db), sqlite3_expanded_sql(stmt));
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
