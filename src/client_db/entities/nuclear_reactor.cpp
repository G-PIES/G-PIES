#include "nuclear_reactor.hpp"

#include <string>

#include "../client_db_impl.hpp"
#include "../db_queries.hpp"

std::string NuclearReactorEntity::get_create_query() {
  return "INSERT INTO reactors ("
         "species, flux, temperature, recombination, i_bi, i_tri, i_quad, "
         "v_bi, v_tri, v_quad, dislocation_density_evolution, "
         "creation_datetime, is_preset) "
         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
}

void NuclearReactorEntity::bind(sqlite3_stmt *stmt, NuclearReactor &reactor,
                                bool &&is_preset) {
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

  if (ClientDbImpl::is_valid_sqlite_id(reactor.sqlite_id)) {
    // update
    sqlite3_bind_int(stmt, 12, reactor.sqlite_id);
  } else {
    // create
    sqlite3_bind_text(stmt, 12, reactor.creation_datetime.c_str(),
                      reactor.creation_datetime.length(), nullptr);
    sqlite3_bind_int(stmt, 13, static_cast<int>(is_preset));
  }
}

std::string NuclearReactorEntity::get_create_error_message(
    const NuclearReactor &reactor) {
  return "Failed to create reactor \"" + reactor.species + "\".";
}
