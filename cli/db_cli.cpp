#include <iostream>
#include <vector>

#include "client_db/client_db.hpp"
#include "model/history_simulation.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/gpies_exception.hpp"
#include "utils/randomizer.hpp"

// used to turn of row deleting so we can inspect the database
#define DELETE_ON false

#define VEC_SIZE 4

void reactors_crud();
void reactor_cmp_print(const NuclearReactor&, const NuclearReactor&);
void materials_crud();
void material_cmp_print(const Material&, const Material&);
void simulations_crud();
void simulation_cmp_print(const HistorySimulation&, const HistorySimulation&);

ClientDb db;
Randomizer randomizer;

int main() {
  try {
    db.clear();
    db.init();

    std::cout << "\nREACTORS CRUD -----------------------------------\n";
    reactors_crud();

    std::cout << "\nMATERIALS CRUD ----------------------------------\n";
    materials_crud();

    std::cout << "\nSIMULATIONS CRUD --------------------------------\n";
    simulations_crud();
  } catch (const ClientDbException& e) {
    fprintf(stderr, "%s\n%s sqlite code = %4d\n\n** SQL QUERY **\n%s\n\n",
            e.message.c_str(), e.sqlite_errmsg.c_str(), e.sqlite_code,
            e.query.c_str());
  }

  return 0;
}

void reactors_crud() {
  int sqlite_code = -1;

  // generate a list of randomized reactors
  std::vector<NuclearReactor> reactors(VEC_SIZE, NuclearReactor());
  for (int i = 0; i < VEC_SIZE; ++i) {
    reactors[i].species = "G-PIES REACTOR " + std::to_string(i);
    randomizer.reactor_randomize(reactors[i]);

    db.create_reactor(reactors[i], &sqlite_code);

    fprintf(stdout, "* CREATE %s\t-\tid = %4d\t-\tsqlite code = %4d\n",
            reactors[i].species.c_str(), reactors[i].sqlite_id, sqlite_code);
  }

  fprintf(stdout, "\n");

  std::vector<NuclearReactor> read_reactors;
  db.read_reactors(read_reactors, &sqlite_code);
  fprintf(stdout,
          "* READ REACTORS - count = %4d - sqlite code = %4d\n"
          "* EXISTING\t\t-\tREAD RESULT\n\n",
          (int)read_reactors.size(), sqlite_code);

  for (int i = 0; i < VEC_SIZE; ++i) {
    reactor_cmp_print(reactors[i], read_reactors[i]);
    fprintf(stdout, "\n");
  }

  fprintf(stdout, "* UPDATE / READ REACTORS\n\n");

  for (int i = 0; i < VEC_SIZE; ++i) {
    reactors[i].species = "U+G-PIES REACTOR " + std::to_string(i);
    randomizer.reactor_randomize(reactors[i]);

    db.update_reactor(reactors[i], &sqlite_code);
    fprintf(stdout, "REACTOR UPDATED\n");

    db.read_reactor(reactors[i].sqlite_id, read_reactors[i], &sqlite_code);
    fprintf(stdout, "READ REACTOR\t-\tsqlite code = %4d\n", sqlite_code);

    reactor_cmp_print(reactors[i], read_reactors[i]);
    fprintf(stdout, "\n");
  }

#if DELETE_ON
  fprintf(stdout, "* DELETE REACTORS\n\n");

  for (int i = 0; i < VEC_SIZE; ++i) {
    db.delete_reactor(reactors[i], &sqlite_code);
    fprintf(stdout, "* REACTOR \"%s\" DELETED\t-\tid = %4d\n",
            reactors[i].species.c_str(), reactors[i].sqlite_id);
  }

  fprintf(stdout, "\n");
#endif
}

void materials_crud() {
  int sqlite_code = -1;

  // generate a list of randomized materials
  std::vector<Material> materials(VEC_SIZE, Material());
  for (int i = 0; i < VEC_SIZE; ++i) {
    materials[i].species = "G-PIES MATERIAL " + std::to_string(i);
    randomizer.material_randomize(materials[i]);

    db.create_material(materials[i], &sqlite_code);

    fprintf(stdout, "* CREATE %s\t-\tid = %4d\t-\tsqlite code = %4d\n",
            materials[i].species.c_str(), materials[i].sqlite_id, sqlite_code);
  }

  fprintf(stdout, "\n");

  std::vector<Material> read_materials;
  db.read_materials(read_materials, &sqlite_code);
  fprintf(stdout,
          "* READ MATERIALS - count = %4d - sqlite code = %4d\n"
          "* EXISTING\t\t-\tREAD RESULT\n\n",
          (int)read_materials.size(), sqlite_code);

  for (int i = 0; i < VEC_SIZE; ++i) {
    material_cmp_print(materials[i], read_materials[i]);
    fprintf(stdout, "\n");
  }

  fprintf(stdout, "* UPDATE / READ MATERIALS\n\n");

  for (int i = 0; i < VEC_SIZE; ++i) {
    materials[i].species = "U+G-PIES MATERIAL " + std::to_string(i);
    randomizer.material_randomize(materials[i]);

    db.update_material(materials[i], &sqlite_code);
    fprintf(stdout, "MATERIAL UPDATED\n");

    db.read_material(materials[i].sqlite_id, read_materials[i], &sqlite_code);
    fprintf(stdout, "READ MATERIAL\t-\tsqlite code = %4d\n", sqlite_code);

    material_cmp_print(materials[i], read_materials[i]);
    fprintf(stdout, "\n");
  }

#if DELETE_ON
  fprintf(stdout, "* DELETE MATERIALS\n\n");

  for (int i = 0; i < VEC_SIZE; ++i) {
    db.delete_material(materials[i], &sqlite_code);
    fprintf(stdout, "* MATERIAL \"%s\" DELETED\t-\tid = %4d\n",
            materials[i].species.c_str(), materials[i].sqlite_id);
  }

  fprintf(stdout, "\n");
#endif
}

void simulations_crud() {
  int sqlite_code = -1;

  std::vector<HistorySimulation> simulations(VEC_SIZE, HistorySimulation());
  for (int i = 0; i < VEC_SIZE; ++i) {
    simulations[i].material.species =
        "SIMULATION MATERIAL " + std::to_string(i);
    simulations[i].reactor.species = "SIMULATION REACTOR " + std::to_string(i);
    randomizer.simulation_randomize(simulations[i]);
    db.create_simulation(simulations[i], &sqlite_code);

    fprintf(stdout, "* CREATE id = %4d\t-\tsqlite code = %4d\n",
            simulations[i].sqlite_id, sqlite_code);
  }

  fprintf(stdout, "\n");
  std::vector<HistorySimulation> read_simulations;
  db.read_simulations(read_simulations, &sqlite_code);
  fprintf(stdout,
          "* READ SIMULATIONS - count = %4d - sqlite code = %4d\n"
          "* EXISTING\t\t-\tREAD RESULT\n\n",
          (int)read_simulations.size(), sqlite_code);

  for (int i = 0; i < VEC_SIZE; ++i) {
    simulation_cmp_print(simulations[i], read_simulations[i]);

    fprintf(stdout, "\n");
    reactor_cmp_print(simulations[i].reactor, read_simulations[i].reactor);

    fprintf(stdout, "\n");
    material_cmp_print(simulations[i].material, read_simulations[i].material);

    fprintf(stdout, "\n");
  }

  // NOTE: HistorySimulation update is not currently supported
  // TODO - decide if HistorySimulation update is useful
  /*
  fprintf(stdout, "* UPDATE / READ SIMULATIONS\n\n");

  for (int i = 0; i < VEC_SIZE; ++i) {
      randomizer.simulation_randomize(simulations[i]);

      db.update_simulation(simulations[i], &sqlite_code);
      fprintf(stdout, "SIMULATION UPDATED\n");

      db.read_simulation(simulations[i].sqlite_id, read_simulations[i],
                         &sqlite_code);
      fprintf(stdout, "READ SIMULATION\t-\tsqlite code = %4d\n", sqlite_code);

      simulation_cmp_print(simulations[i], read_simulations[i]);

      fprintf(stdout, "\n");
      reactor_cmp_print(simulations[i].reactor, read_simulations[i].reactor);

      fprintf(stdout, "\n");
      material_cmp_print(simulations[i].material,
                         read_simulations[i].material);

      fprintf(stdout, "\n");
  }
  */

#if DELETE_ON
  fprintf(stdout, "* DELETE SIMULATIONS\n\n");

  for (int i = 0; i < VEC_SIZE; ++i) {
    db.delete_simulation(simulations[i], &sqlite_code);
    fprintf(stdout, "* SIMULATION DELETED\t-\tid = %4d\n",
            simulations[i].sqlite_id);
  }

  fprintf(stdout, "\n");
#endif
}

void reactor_cmp_print(const NuclearReactor& r1, const NuclearReactor& r2) {
  fprintf(stdout, "%s\t-\t%s\n", r1.species.c_str(), r2.species.c_str());
  fprintf(stdout, "%s\t-\t%s\n", r1.creation_datetime.c_str(),
          r2.creation_datetime.c_str());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_flux(), r2.get_flux());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_temperature(),
          r2.get_temperature());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_recombination(),
          r2.get_recombination());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_bi(), r2.get_i_bi());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_tri(), r2.get_i_tri());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_quad(), r2.get_i_quad());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_bi(), r2.get_v_bi());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_tri(), r2.get_v_tri());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_quad(), r2.get_v_quad());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_dislocation_density_evolution(),
          r2.get_dislocation_density_evolution());
}

void material_cmp_print(const Material& r1, const Material& r2) {
  fprintf(stdout, "%s\t-\t%s\n", r1.species.c_str(), r2.species.c_str());
  fprintf(stdout, "%s\t-\t%s\n", r1.creation_datetime.c_str(),
          r2.creation_datetime.c_str());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_migration(),
          r2.get_i_migration());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_migration(),
          r2.get_v_migration());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_diffusion_0(),
          r2.get_i_diffusion_0());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_diffusion_0(),
          r2.get_v_diffusion_0());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_formation(),
          r2.get_i_formation());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_formation(),
          r2.get_v_formation());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_binding(), r2.get_i_binding());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_binding(), r2.get_v_binding());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_recombination_radius(),
          r2.get_recombination_radius());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_loop_bias(),
          r2.get_i_loop_bias());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_dislocation_bias(),
          r2.get_i_dislocation_bias());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_i_dislocation_bias_param(),
          r2.get_i_dislocation_bias_param());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_loop_bias(),
          r2.get_v_loop_bias());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_dislocation_bias(),
          r2.get_v_dislocation_bias());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_v_dislocation_bias_param(),
          r2.get_v_dislocation_bias_param());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_dislocation_density_0(),
          r2.get_dislocation_density_0());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_grain_size(), r2.get_grain_size());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_lattice_param(),
          r2.get_lattice_param());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_burgers_vector(),
          r2.get_burgers_vector());
  fprintf(stdout, "%g\t\t\t-\t%g\n", r1.get_atomic_volume(),
          r2.get_atomic_volume());
}

void simulation_cmp_print(const HistorySimulation& r1,
                          const HistorySimulation& r2) {
  fprintf(stdout, "%s\t-\t%s\n", r1.creation_datetime.c_str(),
          r2.creation_datetime.c_str());
  fprintf(stdout, "%d\t\t\t\t\t\t-\t%d\n", r1.reactor.sqlite_id,
          r2.reactor.sqlite_id);
  fprintf(stdout, "%d\t\t\t\t\t\t-\t%d\n", r1.material.sqlite_id,
          r2.material.sqlite_id);
  fprintf(stdout, "%g\t\t\t\t\t\t-\t%g\n", r1.cd_state.time, r2.cd_state.time);
  fprintf(stdout, "%zu [%g, %g, %g, %g,...]\t-\t%zu [%g, %g, %g, %g,...]\n",
          r1.cd_state.interstitials.size(), r1.cd_state.interstitials[0],
          r1.cd_state.interstitials[1], r1.cd_state.interstitials[2],
          r1.cd_state.interstitials[3], r2.cd_state.interstitials.size(),
          r2.cd_state.interstitials[0], r2.cd_state.interstitials[1],
          r2.cd_state.interstitials[2], r2.cd_state.interstitials[3]);
  fprintf(stdout, "%zu [%g, %g, %g, %g,...]\t-\t%zu [%g, %g, %g, %g,...]\n",
          r1.cd_state.vacancies.size(), r1.cd_state.vacancies[0],
          r1.cd_state.vacancies[1], r1.cd_state.vacancies[2],
          r1.cd_state.vacancies[3], r2.cd_state.vacancies.size(),
          r2.cd_state.vacancies[0], r2.cd_state.vacancies[1],
          r2.cd_state.vacancies[2], r2.cd_state.vacancies[3]);
  fprintf(stdout, "%g\t\t\t\t\t\t-\t%g\n", r1.cd_state.dislocation_density,
          r2.cd_state.dislocation_density);
}
