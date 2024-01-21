#ifndef DB_QUERIES_HPP
#define DB_QUERIES_HPP

#include <string>

namespace db_queries {

    // reactors CRUD

    const std::string create_reactor = 
        "INSERT INTO reactors ("
        "species, flux, temperature, recombination, i_bi, i_tri, i_quad, v_bi, v_tri, v_quad, dislocation_density_evolution"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    const std::string read_reactors = 
        "SELECT * FROM reactors;";

    const std::string read_reactor = 
        "SELECT * FROM reactors WHERE id_reactor = ?;";

    const std::string update_reactor = 
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

    const std::string delete_reactor = 
        "DELETE FROM reactors WHERE id_reactor = ?;";

    // materials CRUD

    const std::string create_material = 
        "INSERT INTO materials ("
        "species, i_migration, v_migration, i_diffusion_0, v_diffusion_0, i_formation, v_formation, i_binding, v_binding, recombination_radius, i_loop_bias, i_dislocation_bias, i_dislocation_bias_param, v_loop_bias, v_dislocation_bias, v_dislocation_bias_param, dislocation_density_0, grain_size, lattice_param, burgers_vector, atomic_volume"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    const std::string read_materials = 
        "SELECT * FROM materials;";

    const std::string read_material = 
        "SELECT * FROM materials WHERE id_material = ?;";

    const std::string update_material = 
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

    const std::string delete_material = 
        "DELETE FROM materials WHERE id_material = ?;";

    // simulations CRUD

    const std::string create_simulation = 
        "INSERT INTO simulations ("
        "id_reactor, simulation_time, time_delta, data"
        ") VALUES (?, ?, ?, ?);";

    const std::string read_simulations = 
        "SELECT * FROM simulations;";

    const std::string read_simulation = 
        "SELECT * FROM simulations WHERE id_simulation = ?;";

    const std::string update_simulation = 
        "UPDATE simulations SET "
        "id_reactor = ?, "
        "simulation_time = ?, "
        "time_delta = ?, "
        "data = ? "
        "WHERE id_simulation = ?;";

    const std::string delete_simulation = 
        "DELETE FROM simulations WHERE id_simulation = ?;";

    // simulation_materials CRUD

    const std::string create_simulation_material = 
        "INSERT INTO simulation_materials ("
        "id_simulation, id_material"
        ") VALUES (?, ?);";

    const std::string read_simulation_materials = 
        "SELECT * FROM simulation_materials;";

    const std::string read_simulation_material_by_id_simulation = 
        "SELECT * FROM simulation_materials WHERE id_simulation = ?;";

    const std::string read_simulation_material_by_id_material = 
        "SELECT * FROM simulation_materials WHERE id_material = ?;";

    const std::string update_simulation_material = 
        "UPDATE simulation_materials SET "
        "id_simulation = ?, "
        "id_material = ? "
        "WHERE id_simulation = ? "
        "AND id_material = ?;";

    const std::string delete_simulation_material = 
        "DELETE FROM simulation_materials WHERE id_simulation = ? AND id_material = ?;";
}

#endif