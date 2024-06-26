#include "nuclear_reactor.hpp"

#include <string>

#include "../client_db_impl.hpp"
#include "../db_queries.hpp"

std::string NuclearReactorEntity::get_create_one_query() {
  return db_queries::create_reactor;
}

std::string NuclearReactorEntity::get_read_one_query() {
  return db_queries::read_reactor;
}

std::string NuclearReactorEntity::get_read_all_query() {
  return db_queries::read_reactors;
}

std::string NuclearReactorEntity::get_update_one_query() {
  return db_queries::update_reactor;
}

std::string NuclearReactorEntity::get_delete_one_query() {
  return db_queries::delete_reactor;
}

void NuclearReactorEntity::bind_base(sqlite3_stmt *stmt,
                                     const NuclearReactor &reactor) {
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
}

void NuclearReactorEntity::bind_update_one(sqlite3_stmt *stmt,
                                           const NuclearReactor &reactor) {
  bind_base(stmt, reactor);
  sqlite3_bind_int(stmt, 12, reactor.sqlite_id);
}

void NuclearReactorEntity::bind_create_one(sqlite3_stmt *stmt,
                                           const NuclearReactor &reactor,
                                           bool &&is_preset) {
  bind_base(stmt, reactor);
  sqlite3_bind_text(stmt, 12, reactor.creation_datetime.c_str(),
                    reactor.creation_datetime.length(), nullptr);
  sqlite3_bind_int(stmt, 13, static_cast<int>(is_preset));
}

void NuclearReactorEntity::read_row(sqlite3_stmt *stmt,
                                    NuclearReactor &reactor) {
  read_row(stmt, reactor, 0);
}

void NuclearReactorEntity::read_row(sqlite3_stmt *stmt, NuclearReactor &reactor,
                                    const int col_offset) {
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

std::string NuclearReactorEntity::get_entity_name() { return "reactor"; }
std::string NuclearReactorEntity::get_entities_name() { return "reactors"; }
std::string NuclearReactorEntity::get_entity_description(
    const NuclearReactor &reactor) {
  return "\"" + reactor.species + "\"";
}
