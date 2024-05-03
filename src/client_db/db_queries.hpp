#pragma once

#include <string>

namespace db_queries {

/*  SCHEMA
    in SQLite, AUTOINCREMENT on PRIMARY KEYs only prevents the reuse of ROWIDs
    which have not been used. The macro is omitted to avoid unnecessary
   overhead. https://www.sqlite.org/autoinc.html
*/
extern std::string init;

extern std::string clear;

extern std::string last_insert_rowid;

// reactors CRUD

extern std::string create_reactor;

extern std::string read_reactors;

extern std::string read_reactor;

extern std::string update_reactor;

extern std::string delete_reactor;

// materials CRUD

extern std::string create_material;

extern std::string read_materials;

extern std::string read_material;

extern std::string update_material;

extern std::string delete_material;

// history_simulations CRUD

extern std::string create_simulation;

extern std::string read_simulations;

extern std::string read_simulation;

extern std::string delete_simulation;

extern std::string delete_simulations;

// simulation_materials CRUD

extern std::string create_simulation_material;

extern std::string read_simulation_materials;

extern std::string read_simulation_material_by_id_simulation;

extern std::string read_simulation_material_by_id_material;

extern std::string update_simulation_material;

extern std::string delete_simulation_material;
}  // namespace db_queries
