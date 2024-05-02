

#include "client_db/client_db.hpp"

#include <sqlite3.h>
#include <string>
#include <vector>

#include "client_db_impl.hpp"
#include "db_queries.hpp"
#include "entities/nuclear_reactor.hpp"
#include "model/history_simulation.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/blob_converter.hpp"

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  PUBLIC INTERFACE FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

bool ClientDb::init(int *sqlite_result_code) {
  return _impl->init(sqlite_result_code);
}

bool ClientDb::clear(int *sqlite_result_code) {
  return _impl->clear(sqlite_result_code);
}

bool ClientDb::create_reactor(
    NuclearReactor &reactor,
    int *sqlite_result_code,
    bool is_preset) {
  return _impl->create_one<NuclearReactorEntity>(
    reactor,
    sqlite_result_code,
    std::forward<bool>(is_preset));
}

bool ClientDb::read_reactors(std::vector<NuclearReactor> &reactors,
                             int *sqlite_result_code) {
  return _impl->read_all<NuclearReactorEntity>(reactors, sqlite_result_code);
}

bool ClientDb::read_reactor(const int sqlite_id, NuclearReactor &reactor,
                            int *sqlite_result_code) {
  return _impl->read_one<NuclearReactorEntity>(
    sqlite_id,
    reactor,
    sqlite_result_code);
}

bool ClientDb::update_reactor(const NuclearReactor &reactor,
                              int *sqlite_result_code) {
  return _impl->update_one<NuclearReactorEntity>(reactor, sqlite_result_code);
}

bool ClientDb::delete_reactor(const NuclearReactor &reactor,
                              int *sqlite_result_code) {
  if (!is_valid_sqlite_id(reactor.sqlite_id))
    throw ClientDbException("Failed to delete reactor. Invalid id.");

  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::delete_reactor.c_str(),
                         db_queries::delete_reactor.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_delete_reactor(stmt, reactor);

  sqlite_code = _impl->delete_one<NuclearReactor>(stmt,
                                                  _impl->err_delete_reactor,
                                                  reactor);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::create_material(Material &material, int *sqlite_result_code,
                               bool is_preset) {
  if (is_valid_sqlite_id(material.sqlite_id))
    throw ClientDbException("Failed to create material, it already exists.");
  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::create_material.c_str(),
                         db_queries::create_material.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_create_material(stmt, material);

  ClientDbImpl::bind_material(stmt, material, is_preset);

  sqlite_code = _impl->create_one<Material>(stmt,
                                            _impl->err_create_material,
                                            material);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::read_materials(std::vector<Material> &materials,
                              int *sqlite_result_code) {
  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::read_materials.c_str(),
                         db_queries::read_materials.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_read_materials(stmt);

  sqlite_code = _impl->read_all<Material>(stmt, _impl->row_read_material,
                                          _impl->err_read_materials, materials);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::read_material(const int sqlite_id, Material &material,
                             int *sqlite_result_code) {
  if (!is_valid_sqlite_id(sqlite_id))
    throw ClientDbException("Failed to read material. Invalid id.");

  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::read_material.c_str(),
                         db_queries::read_material.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_read_material(stmt, sqlite_id);

  sqlite_code = _impl->read_one<Material>(stmt, _impl->row_read_material,
                                          _impl->err_read_material, sqlite_id,
                                          material);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code) &&
         is_valid_sqlite_id(material.sqlite_id);
}

bool ClientDb::update_material(const Material &material,
                               int *sqlite_result_code) {
  if (!is_valid_sqlite_id(material.sqlite_id))
    throw ClientDbException("Failed to update material. Invalid id.");

  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::update_material.c_str(),
                         db_queries::update_material.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_update_material(stmt, material);

  ClientDbImpl::bind_material(stmt, material);

  sqlite_code = _impl->update_one<Material>(stmt, _impl->err_update_material,
                                            material);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::delete_material(const Material &material,
                               int *sqlite_result_code) {
  if (!is_valid_sqlite_id(material.sqlite_id))
    throw ClientDbException("Failed to delete material. Invalid id.");

  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::delete_material.c_str(),
                         db_queries::delete_material.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_delete_material(stmt, material);

  sqlite_code = _impl->delete_one<Material>(stmt, _impl->err_delete_material,
                                            material);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::create_simulation(HistorySimulation &simulation,
                                 int *sqlite_result_code) {
  if (is_valid_sqlite_id(simulation.sqlite_id))
    throw ClientDbException("Failed to create simulation, it already exists.");
  if (!_impl->db) open();

  // create non-preset copies of the reactor and material
  create_reactor(simulation.reactor, nullptr, false);
  create_material(simulation.material, nullptr, false);

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::create_simulation.c_str(),
                         db_queries::create_simulation.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_create_simulation(stmt,
                                                                 simulation);

  ClientDbImpl::bind_simulation(stmt, simulation);

  sqlite_code =
      _impl->create_one<HistorySimulation>(stmt, _impl->err_create_simulation,
                                           simulation);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::read_simulations(std::vector<HistorySimulation> &simulations,
                                int *sqlite_result_code) {
  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::read_simulations.c_str(),
                         db_queries::read_simulations.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_read_simulations(stmt);

  sqlite_code = _impl->read_all<HistorySimulation>(stmt,
                                                  _impl->row_read_simulation,
                                                  _impl->err_read_simulations,
                                                  simulations);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::read_simulation(const int sqlite_id,
                               HistorySimulation &simulation,
                               int *sqlite_result_code) {
  if (!is_valid_sqlite_id(sqlite_id))
    throw ClientDbException("Failed to read simulation. Invalid id.");

  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::read_simulation.c_str(),
                         db_queries::read_simulation.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_read_simulation(stmt, sqlite_id);

  sqlite_code = _impl->read_one<HistorySimulation>(
      stmt, _impl->row_read_simulation, _impl->err_read_simulation, sqlite_id,
      simulation);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code) &&
         is_valid_sqlite_id(simulation.sqlite_id);
}

bool ClientDb::delete_simulation(const HistorySimulation &simulation,
                                 int *sqlite_result_code) {
  if (!is_valid_sqlite_id(simulation.sqlite_id))
    throw ClientDbException("Failed to delete simulation. Invalid id.");

  if (!_impl->db) open();

  int sqlite_code;
  sqlite3_stmt *stmt;

  sqlite_code =
      sqlite3_prepare_v2(_impl->db, db_queries::delete_simulation.c_str(),
                         db_queries::delete_simulation.size(), &stmt, nullptr);
  if (is_sqlite_error(sqlite_code)) _impl->err_delete_simulation(stmt,
                                                                 simulation);

  sqlite_code =
      _impl->delete_one<HistorySimulation>(stmt, _impl->err_delete_simulation,
                                           simulation);

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::delete_simulations(int *sqlite_result_code) {
  if (!_impl->db) open();

  int sqlite_code;
  char *sqlite_errmsg;

  sqlite_code = sqlite3_exec(_impl->db, db_queries::delete_simulations.c_str(),
                             nullptr, nullptr, &sqlite_errmsg);

  if (is_sqlite_error(sqlite_code)) {
    throw ClientDbException("Failed to delete simulations.", sqlite_errmsg,
                            sqlite_code, db_queries::delete_simulations);
  }

  if (sqlite_result_code) *sqlite_result_code = sqlite_code;
  return is_sqlite_success(sqlite_code);
}

bool ClientDb::open(int *sqlite_result_code) {
  return _impl->open(sqlite_result_code);
}

bool ClientDb::close(int *sqlite_result_code) {
  return _impl->close(sqlite_result_code);
}

bool ClientDb::is_open() {
  return _impl->is_open();
}

bool ClientDb::is_sqlite_success(const int sqlite_code) {
  return ClientDbImpl::is_sqlite_success(sqlite_code);
}

bool ClientDb::is_sqlite_error(const int sqlite_code) {
  return ClientDbImpl::is_sqlite_error(sqlite_code);
}

bool ClientDb::is_valid_sqlite_id(const int sqlite_id) {
  return ClientDbImpl::is_valid_sqlite_id(sqlite_id);
}

int ClientDb::changes() { return _impl->changes(); }

ClientDb::ClientDb(const std::string &db_path, const bool lazy) {
  _impl = std::make_unique<ClientDbImpl>(db_path, lazy);
}

ClientDb::~ClientDb() {
  _impl.reset();
}
