#include "material.hpp"

#include <string>

#include "../client_db_impl.hpp"
#include "../db_queries.hpp"

std::string MaterialEntity::get_create_one_query() {
  return db_queries::create_material;
}

std::string MaterialEntity::get_read_one_query() {
  return db_queries::read_material;
}

std::string MaterialEntity::get_read_all_query() {
  return db_queries::read_materials;
}

std::string MaterialEntity::get_update_one_query() {
  return db_queries::update_material;
}

std::string MaterialEntity::get_delete_one_query() {
  return db_queries::delete_material;
}

void MaterialEntity::bind_base(
    sqlite3_stmt *stmt,
    const Material &material) {
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
}

void MaterialEntity::bind_update_one(
    sqlite3_stmt *stmt,
    const Material &material) {
  bind_base(stmt, material);
  sqlite3_bind_int(stmt, 22, material.sqlite_id);
}

void MaterialEntity::bind_create_one(
    sqlite3_stmt *stmt,
    const Material &material,
    bool &&is_preset) {
  bind_base(stmt, material);
  sqlite3_bind_text(stmt, 22, material.creation_datetime.c_str(),
                    material.creation_datetime.length(), nullptr);
  sqlite3_bind_int(stmt, 23, static_cast<int>(is_preset));
}

void MaterialEntity::read_row(
    sqlite3_stmt *stmt,
    Material &material) {
  int col_offset = 0;
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

std::string MaterialEntity::get_entity_name() { return "material"; }
std::string MaterialEntity::get_entities_name() { return "maerials"; }
std::string MaterialEntity::get_entity_description(
    const Material &material) {
  return "\"" + material.species + "\"";
}
