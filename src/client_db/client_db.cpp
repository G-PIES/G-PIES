

#include "client_db/client_db.hpp"

#include <sqlite3.h>
#include <string>
#include <vector>

#include "client_db_impl.hpp"
#include "db_queries.hpp"
#include "entities/history_simulation.hpp"
#include "entities/material.hpp"
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
  return _impl->delete_one<NuclearReactorEntity>(reactor, sqlite_result_code);
}

bool ClientDb::create_material(Material &material, int *sqlite_result_code,
                               bool is_preset) {
  return _impl->create_one<MaterialEntity>(
    material,
    sqlite_result_code,
    std::forward<bool>(is_preset));
}

bool ClientDb::read_materials(std::vector<Material> &materials,
                              int *sqlite_result_code) {
  return _impl->read_all<MaterialEntity>(materials, sqlite_result_code);
}

bool ClientDb::read_material(const int sqlite_id, Material &material,
                             int *sqlite_result_code) {
  return _impl->read_one<MaterialEntity>(
    sqlite_id,
    material,
    sqlite_result_code);
}

bool ClientDb::update_material(const Material &material,
                               int *sqlite_result_code) {
  return _impl->update_one<MaterialEntity>(material, sqlite_result_code);
}

bool ClientDb::delete_material(const Material &material,
                               int *sqlite_result_code) {
  return _impl->delete_one<MaterialEntity>(material, sqlite_result_code);
}

bool ClientDb::create_simulation(HistorySimulation &simulation,
                                 int *sqlite_result_code) {
  // create non-preset copies of the reactor and material
  create_reactor(simulation.reactor, nullptr, false);
  create_material(simulation.material, nullptr, false);

  return _impl->create_one<HistorySimulationEntity>(
    simulation,
    sqlite_result_code);
}

bool ClientDb::read_simulations(std::vector<HistorySimulation> &simulations,
                                int *sqlite_result_code) {
  return _impl->read_all<HistorySimulationEntity>(
    simulations,
    sqlite_result_code);
}

bool ClientDb::read_simulation(const int sqlite_id,
                               HistorySimulation &simulation,
                               int *sqlite_result_code) {
  return _impl->read_one<HistorySimulationEntity>(
    sqlite_id,
    simulation,
    sqlite_result_code);
}

bool ClientDb::delete_simulation(const HistorySimulation &simulation,
                                 int *sqlite_result_code) {
  return _impl->delete_one<HistorySimulationEntity>(
    simulation,
    sqlite_result_code);
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
