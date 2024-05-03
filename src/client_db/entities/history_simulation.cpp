#include "history_simulation.hpp"

#include <string>
#include <vector>

#include "utils/blob_converter.hpp"
#include "../client_db_impl.hpp"
#include "../db_queries.hpp"

std::string HistorySimulationEntity::get_create_one_query() {
  return db_queries::create_simulation;
}

std::string HistorySimulationEntity::get_read_one_query() {
  return db_queries::read_simulation;
}

std::string HistorySimulationEntity::get_read_all_query() {
  return db_queries::read_simulations;
}

std::string HistorySimulationEntity::get_update_one_query() {
  return "";
}

std::string HistorySimulationEntity::get_delete_one_query() {
  return db_queries::delete_simulation;
}

void HistorySimulationEntity::bind_base(
    sqlite3_stmt *stmt,
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

void HistorySimulationEntity::bind_update_one(
    sqlite3_stmt *stmt,
    const HistorySimulation &simulation) {
  bind_base(stmt, simulation);
}

void HistorySimulationEntity::bind_create_one(
    sqlite3_stmt *stmt,
    const HistorySimulation &simulation) {
  bind_base(stmt, simulation);
}

void HistorySimulationEntity::read_row(
    sqlite3_stmt *stmt,
    HistorySimulation &simulation) {
  int col_offset = 0;
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

  _material_entity.read_row(stmt, simulation.material, 11);
  _nuclear_reactor_entity.read_row(stmt, simulation.reactor, 25);
}

std::string HistorySimulationEntity::get_entity_name() { return "simulation"; }
std::string HistorySimulationEntity::get_entities_name() {
  return "simulations";
}
std::string HistorySimulationEntity::get_entity_description(
    const HistorySimulation &simulation) {
  return "at " + simulation.creation_datetime;
}
