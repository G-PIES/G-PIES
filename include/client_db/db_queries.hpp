#ifndef DB_QUERIES_HPP
#define DB_QUERIES_HPP

#include <string>

namespace db_queries {

/*  SCHEMA
    in SQLite, AUTOINCREMENT on PRIMARY KEYs only prevents the reuse of ROWIDs
    which have not been used. The macro is omitted to avoid unnecessary
   overhead. https://www.sqlite.org/autoinc.html
*/
std::string init =
    "CREATE TABLE IF NOT EXISTS reactors"
    "("
    "id_reactor INTEGER PRIMARY KEY,"
    "creation_datetime TEXT,"
    "species text NOT NULL DEFAULT '',"
    "flux FLOAT DEFAULT 0.0,"
    "temperature FLOAT DEFAULT 0.0,"
    "recombination FLOAT DEFAULT 0.0,"
    "i_bi FLOAT DEFAULT 0.0,"
    "i_tri FLOAT DEFAULT 0.0,"
    "i_quad FLOAT DEFAULT 0.0,"
    "v_bi FLOAT DEFAULT 0.0,"
    "v_tri FLOAT DEFAULT 0.0,"
    "v_quad FLOAT DEFAULT 0.0,"
    "dislocation_density_evolution FLOAT DEFAULT 0.0,"
    "is_preset INTEGER"
    ");"

    "CREATE TABLE IF NOT EXISTS materials"
    "("
    "id_material INTEGER PRIMARY KEY,"
    "creation_datetime TEXT,"
    "species text NOT NULL DEFAULT '',"
    "i_migration FLOAT DEFAULT 0.0,"
    "v_migration FLOAT DEFAULT 0.0,"
    "i_diffusion_0 FLOAT DEFAULT 0.0,"
    "v_diffusion_0 FLOAT DEFAULT 0.0,"
    "i_formation FLOAT DEFAULT 0.0,"
    "v_formation FLOAT DEFAULT 0.0,"
    "i_binding FLOAT DEFAULT 0.0,"
    "v_binding FLOAT DEFAULT 0.0,"
    "recombination_radius FLOAT DEFAULT 0.0,"
    "i_loop_bias FLOAT DEFAULT 0.0,"
    "i_dislocation_bias FLOAT DEFAULT 0.0,"
    "i_dislocation_bias_param FLOAT DEFAULT 0.0,"
    "v_loop_bias FLOAT DEFAULT 0.0,"
    "v_dislocation_bias FLOAT DEFAULT 0.0,"
    "v_dislocation_bias_param FLOAT DEFAULT 0.0,"
    "dislocation_density_0 FLOAT DEFAULT 0.0,"
    "grain_size FLOAT DEFAULT 0.0,"
    "lattice_param FLOAT DEFAULT 0.0,"
    "burgers_vector FLOAT DEFAULT 0.0,"
    "atomic_volume FLOAT DEFAULT 0.0,"
    "is_preset INTEGER"
    ");"

    "CREATE TABLE IF NOT EXISTS history_simulations"
    "("
    "id_simulation INTEGER PRIMARY KEY,"
    "creation_datetime TEXT,"
    "max_cluster_size INTEGER DEFAULT 0,"
    "simulation_time FLOAT DEFAULT 0.0,"
    "delta_time FLOAT DEFAULT 0.0,"
    "id_reactor INTEGER,"
    "id_material INTEGER,"
    "interstitials BLOB,"
    "vacancies BLOB,"
    "dislocation_density FLOAT DEFAULT 0.0,"
    "density_per_atom FLOAT DEFAULT 0.0"
    ");";

std::string clear =
    "DROP TABLE IF EXISTS history_simulations;"
    "DROP TABLE IF EXISTS reactors;"
    "DROP TABLE IF EXISTS materials;"
    "DROP TABLE IF EXISTS simulation_materials;";

std::string last_insert_rowid = "SELECT last_insert_rowid();";

// reactors CRUD

std::string create_reactor =
    "INSERT INTO reactors ("
    "species, flux, temperature, recombination, i_bi, i_tri, i_quad, v_bi, "
    "v_tri, v_quad, dislocation_density_evolution, creation_datetime, is_preset"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

std::string read_reactors = "SELECT * FROM reactors;";

std::string read_reactor = "SELECT * FROM reactors WHERE id_reactor = ?;";

std::string update_reactor =
    "UPDATE reactors SET "
    "species = ?, "
    "flux = ?, "
    "temperature = ?, "
    "recombination = ?, "
    "i_bi = ?, "
    "i_tri = ?, "
    "i_quad = ?, "
    "v_bi = ?, "
    "v_tri = ?, "
    "v_quad = ?, "
    "dislocation_density_evolution = ? "
    "WHERE id_reactor = ?;";

std::string delete_reactor = "DELETE FROM reactors WHERE id_reactor = ?;";

// materials CRUD

std::string create_material =
    "INSERT INTO materials ("
    "species, i_migration, v_migration, i_diffusion_0, v_diffusion_0, "
    "i_formation, v_formation, i_binding, v_binding, recombination_radius, "
    "i_loop_bias, i_dislocation_bias, i_dislocation_bias_param, v_loop_bias, "
    "v_dislocation_bias, v_dislocation_bias_param, dislocation_density_0, "
    "grain_size, lattice_param, burgers_vector, atomic_volume, "
    "creation_datetime, is_preset"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
    "?, ?);";

std::string read_materials = "SELECT * FROM materials;";

std::string read_material = "SELECT * FROM materials WHERE id_material = ?;";

std::string update_material =
    "UPDATE materials SET "
    "species = ?, "
    "i_migration = ?, "
    "v_migration = ?, "
    "i_diffusion_0 = ?, "
    "v_diffusion_0 = ?, "
    "i_formation = ?, "
    "v_formation = ?, "
    "i_binding = ?, "
    "v_binding = ?, "
    "recombination_radius = ?, "
    "i_loop_bias = ?, "
    "i_dislocation_bias = ?, "
    "i_dislocation_bias_param = ?, "
    "v_loop_bias = ?, "
    "v_dislocation_bias = ?, "
    "v_dislocation_bias_param = ?, "
    "dislocation_density_0 = ?, "
    "grain_size = ?, "
    "lattice_param = ?, "
    "burgers_vector = ?, "
    "atomic_volume = ? "
    "WHERE id_material = ?;";

std::string delete_material = "DELETE FROM materials WHERE id_material = ?;";

// history_simulations CRUD

std::string create_simulation =
    "INSERT INTO history_simulations ("
    "max_cluster_size, simulation_time, delta_time, id_reactor, "
    "id_material, interstitials, vacancies, "
    "dislocation_density, density_per_atom, creation_datetime"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

std::string read_simulations =
    "SELECT * FROM history_simulations "
    "INNER JOIN reactors ON reactors.id_reactor = "
    "history_simulations.id_reactor "
    "INNER JOIN materials ON materials.id_material = "
    "history_simulations.id_material;";

std::string read_simulation =
    "SELECT * FROM history_simulations "
    "INNER JOIN reactors ON reactors.id_reactor = "
    "history_simulations.id_reactor "
    "INNER JOIN materials ON materials.id_material = "
    "history_simulations.id_material "
    "WHERE history_simulations.id_simulation = ?;";

std::string delete_simulation =
    "DELETE FROM history_simulations WHERE id_simulation = ?;";

std::string delete_simulations = "DELETE FROM history_simulations;";

// simulation_materials CRUD

std::string create_simulation_material =
    "INSERT INTO simulation_materials ("
    "id_simulation, id_material"
    ") VALUES (?, ?);";

std::string read_simulation_materials = "SELECT * FROM simulation_materials;";

std::string read_simulation_material_by_id_simulation =
    "SELECT * FROM simulation_materials WHERE id_simulation = ?;";

std::string read_simulation_material_by_id_material =
    "SELECT * FROM simulation_materials WHERE id_material = ?;";

std::string update_simulation_material =
    "UPDATE simulation_materials SET "
    "id_simulation = ?, "
    "id_material = ? "
    "WHERE id_simulation = ? "
    "AND id_material = ?;";

std::string delete_simulation_material =
    "DELETE FROM simulation_materials WHERE id_simulation = ? AND id_material "
    "= ?;";
}  // namespace db_queries

#endif  // DB_QUERIES_HPP
