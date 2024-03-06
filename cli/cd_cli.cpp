#include <array>
#include <boost/program_options.hpp>
#include <cmath>
#include <cstring>
#include <iostream>

#include "client_db/client_db.hpp"
#include "cluster_dynamics/cluster_dynamics.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/timer.hpp"

namespace po = boost::program_options;

#ifndef VPRINT
#define VPRINT false
#endif

#ifndef VBREAK
#define VBREAK false
#endif

#ifndef CSV
#define CSV false
#endif

size_t concentration_boundary;
bool sensitivity_analysis_mode = false;
std::string sensitivity_analysis_variable = "";
size_t num_of_simulation_loops = 0;
gp_float delta_sensitivity_analysis = 0;
gp_float simulation_time;
gp_float delta_time;
gp_float sample_interval;  // How often (in seconds) to record the state

void print_start_message() {
  fprintf(stderr, "\nSimulation Started: ");
  fprintf(stderr, "delta_time: %g, ", delta_time);
  fprintf(stderr, "simulation_time: %g, ", simulation_time);
  fprintf(stderr, "concentration_boundary: %4d\n",
          static_cast<int>(concentration_boundary));
}

void print_state(const ClusterDynamicsState& state) {
  if (!state.valid)
    fprintf(stdout, "\nINVALID SIM @ Time=%g", state.time);
  else
    fprintf(stdout, "\nTime=%g", state.time);

  if (state.interstitials.size() != concentration_boundary ||
      state.vacancies.size() != concentration_boundary) {
    fprintf(stderr, "\nError: Output data is incorrect size.\n");
    return;
  }

  fprintf(stdout, "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n");
  for (uint64_t n = 1; n < concentration_boundary; ++n) {
    fprintf(stdout, "%llu\t\t\t\t\t%13g\t\t\t  %15g\n\n",
            (long long unsigned int)n, state.interstitials[n],
            state.vacancies[n]);
  }

  fprintf(stderr, "\nDislocation Network Density: %g\n\n",
          state.dislocation_density);
}

void print_csv(ClusterDynamicsState& state) {
  fprintf(stdout, "%g", state.dpa);
  for (uint64_t n = 1; n < concentration_boundary; ++n) {
    fprintf(stdout, ",%g,%g", state.interstitials[n], state.vacancies[n]);
  }
  fprintf(stdout, "\n");
}

void print_simulation_history(ClientDb& db, bool print_details) {
  std::vector<HistorySimulation> simulations;
  db.read_simulations(simulations);

  fprintf(stdout, "\nSimulation History\tCount: %llu\n",
          static_cast<long long unsigned int>(simulations.size()));

  if (!simulations.empty()) {
    fprintf(stdout,
            "ID ~ Concentration Boundary ~ Simulation Time ~ Delta "
            "Time ~ Reactor ~ Material ~ Creation Datetime\n\n");

    for (HistorySimulation s : simulations) {
      fprintf(stdout, "%d ~ %llu ~ %g ~ %g ~ %s ~ %s ~ %s\n", s.sqlite_id,
              static_cast<unsigned long long>(s.concentration_boundary),
              s.simulation_time, s.delta_time, s.reactor.species.c_str(),
              s.material.species.c_str(), s.creation_datetime.c_str());

      // Print the state(s) of the simulation
      if (print_details) {
        print_state(s.cd_state);
        fprintf(stdout, "\n\n");
      }
    }

    fprintf(stdout, "\n");
  }
}

void profile() {
  std::vector<gp_float> times;
  Timer timer;

  ClusterDynamicsState state;

  NuclearReactor reactor;
  nuclear_reactors::OSIRIS(reactor);

  Material material;
  materials::SA304(material);

  ClusterDynamics cd(10, reactor, material);
  cd.run(1e-5, 1e-5);

  for (int n = 100; n < 400000; n += 10000) {
    fprintf(stderr, "N=%d\n", n);
    ClusterDynamics cd(n, reactor, material);

    timer.Start();
    state = cd.run(1e-5, 1e-5);
    gp_float time = timer.Stop();

    fprintf(stdout, "\n%g", time);
  }
}

enum var_code {
  e_noMatch,
  e_iMigration,
  e_vMigration,
  e_iFormation,
  e_vFormation,
  e_iBinding,
  e_vBinding,
  e_dislocationDensity0,
  e_flux,
  e_temperature,
  e_dislocationDensityEvolution
};

var_code hashit(std::string const& varString) {
  if (varString == "i_migration") return e_iMigration;
  if (varString == "v_migration") return e_vMigration;
  if (varString == "i_formation") return e_iFormation;
  if (varString == "v_formation") return e_vFormation;
  if (varString == "i_binding") return e_iBinding;
  if (varString == "v_binding") return e_vBinding;
  if (varString == "dislocation_density_0") return e_dislocationDensity0;
  if (varString == "flux") return e_flux;
  if (varString == "temperature") return e_temperature;
  if (varString == "dislocation_density_evolution")
    return e_dislocationDensityEvolution;
  return e_noMatch;
}

void update_for_sensitivity_analysis(ClusterDynamics& cd,
                                     NuclearReactor& reactor,
                                     Material& material, const gp_float delta) {
  switch (hashit(sensitivity_analysis_variable)) {
    case e_iMigration:
      material.set_i_migration(material.get_i_migration() + delta);
      cd.set_material(material);
      break;
    case e_vMigration:
      material.set_v_migration(material.get_v_migration() + delta);
      cd.set_material(material);
      break;
    case e_iFormation:
      material.set_i_formation(material.get_i_formation() + delta);
      cd.set_material(material);
      break;
    case e_vFormation:
      material.set_v_formation(material.get_v_formation() + delta);
      cd.set_material(material);
      break;
    case e_iBinding:
      material.set_i_binding(material.get_i_binding() + delta);
      cd.set_material(material);
      break;
    case e_vBinding:
      material.set_v_binding(material.get_v_binding() + delta);
      cd.set_material(material);
      break;
    case e_dislocationDensity0:
      material.set_dislocation_density_0(material.get_dislocation_density_0() +
                                         delta);
      cd.set_material(material);
      break;
    case e_flux:
      reactor.set_flux(reactor.get_flux() + delta);
      cd.set_reactor(reactor);
      break;
    case e_temperature:
      reactor.set_temperature(reactor.get_temperature() + delta);
      cd.set_reactor(reactor);
      break;
    case e_dislocationDensityEvolution:
      reactor.set_dislocation_density_evolution(
          reactor.get_dislocation_density_evolution() + delta);
      cd.set_reactor(reactor);
      break;
    default:
      break;
  }
}

ClusterDynamicsState run_simulation(const NuclearReactor& reactor,
                                    const Material& material) {
  ClusterDynamics cd(concentration_boundary, reactor, material);

  print_start_message();

#if CSV
  fprintf(stdout,
          "Time (s),Cluster Size,Interstitials / cm^3,Vacancies / cm^3\n");
#endif

  ClusterDynamicsState state;
  // --------------------------------------------------------------------------------------------
  // main simulation loop
  for (gp_float t = 0; t < simulation_time; t = state.time) {
    // run simulation for this time slice
    state = cd.run(delta_time, sample_interval);

#if VPRINT
    print_state(state);
#elif CSV
    print_csv(state);
#endif

    if (!state.valid) {
      break;
    }

#if VBREAK
    fgetc(stdin);
#endif
  }
  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  // print results
#if !VPRINT && !CSV
  print_state(state);
#endif
  // --------------------------------------------------------------------------------------------

  return state;
}

// --------------------------------------------------------------------------------------------
// Boost argument parsing
bool has_option(const std::vector<std::string_view>& args,
                const std::string_view& option_name) {
  for (auto it = args.begin(), end = args.end(); it != end; ++it) {
    if (*it == option_name) return true;
  }

  return false;
}

std::string_view get_option(const std::vector<std::string_view>& args,
                            const std::string_view& option_name) {
  for (auto it = args.begin(), end = args.end(); it != end; ++it) {
    if (*it == option_name)
      if (it + 1 != end) return *(it + 1);
  }

  return "";
}
// --------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  const std::vector<std::string_view> args(argv, argv + argc);
  po::variables_map vm;

  ClientDb db(DEFAULT_CLIENT_DB_PATH, false);
  // Open SQLite connection and create database
  db.init();

  // Default values
  concentration_boundary = 10;
  simulation_time = 1.;
  delta_time = 1e-5;

  // --------------------------------------------------------------------------------------------
  if (has_option(args, "-db")) {
    // DATABASE
    if (has_option(args, "history")) {
      // Clear history
      if (has_option(args, "--clear")) {
        if (db.delete_simulations()) {
          fprintf(stdout, "Simulation History Cleared.\n");
        }
      } else {
      }
    } else if (has_option(args, "run")) {
    }
  } else if (has_option(args, "-s")) {
    // SENSITIVITY ANALYSIS
  } else {
    // CLUSTER DYNAMICS
  }
  // --------------------------------------------------------------------------------------------

  if (argc > 2 && !strcmp("-db", argv[1])) {
    if (!strcmp("history", argv[2])) {
      bool print_details = argc > 3 && !strcmp("--detail", argv[3]);

      // print simulation history
      print_simulation_history(db, print_details);
    } else if (!strcmp("run", argv[2]) && argc > 3) {
      // rerun a previous simulation
      int sim_sqlite_id = strtod(argv[3], NULL);
      HistorySimulation sim;
      if (db.read_simulation(sim_sqlite_id, sim)) {
        // TODO - support storing sensitivity analysis
        fprintf(stdout, "Running simulation %d\n", sim_sqlite_id);
        concentration_boundary = sim.concentration_boundary;
        simulation_time = sim.simulation_time;

        // TODO - Support sample interval and set a max resolution to avoid
        // bloating the database. For this to work we will need a list of
        // ClusterDynamicState objects and a SQLite intersection table.
        delta_time = sample_interval = sim.delta_time;

        run_simulation(sim.reactor, sim.material);
      } else {
        fprintf(stderr, "Could not find simulation %d\n", sim_sqlite_id);
      }
    } else if (!strcmp("clear", argv[2])) {
      if (db.delete_simulations()) {
        fprintf(stdout, "Simulation History Cleared.\n");
      }
    }

    return 0;
  }
  // --------------------------------------------------------------------------------------------

  NuclearReactor reactor;
  nuclear_reactors::OSIRIS(reactor);

  Material material;
  materials::SA304(material);

  // Override default values with CLI arguments
  switch (argc) {
    case 8:
      delta_sensitivity_analysis = strtod(argv[7], NULL);
      num_of_simulation_loops = strtod(argv[6], NULL);
      sensitivity_analysis_variable = argv[5];
      sensitivity_analysis_mode = true;  // argv[4] should be -s
                                         // fall through
    case 4:
      concentration_boundary = strtod(argv[3], NULL);
      // fall through
    case 3:
      simulation_time = strtod(argv[2], NULL);
      // fall through
    case 2:
      delta_time = strtod(argv[1], NULL);
    default:
      break;
  }

  sample_interval = delta_time;

  if (sensitivity_analysis_mode) {
    // --------------------------------------------------------------------------------------------
    // sensitivity analysis simulation loop
    for (size_t n = 0; n < num_of_simulation_loops; n++) {
      ClusterDynamics cd(concentration_boundary, reactor, material);

      print_start_message();

#if CSV
      fprintf(stdout, "Time (s),Cluster Size," +
                          "Interstitials / cm^3,Vacancies / cm^3\n");
#endif

      ClusterDynamicsState state;
      update_for_sensitivity_analysis(cd, reactor, material,
                                      n * delta_sensitivity_analysis);

      for (gp_float t = 0; t < simulation_time; t = state.time) {
        // run simulation for this time slice
        state = cd.run(delta_time, sample_interval);

#if VPRINT
        print_state(state);
#elif CSV
        print_csv(state);
#endif

        if (!state.valid) {
          break;
        }

#if VBREAK
        fgetc(stdin);
#endif
      }

// ----------------------------------------------------------------
// print results
#if !VPRINT && !CSV
      print_state(state);
#endif
      // ----------------------------------------------------------------
    }
    // --------------------------------------------------------------------
  } else {
    ClusterDynamicsState state = run_simulation(reactor, material);

    // --------------------------------------------------------------------------------------------
    // Write simulation result to the database
    HistorySimulation history_simulation(concentration_boundary,
                                         simulation_time, delta_time, reactor,
                                         material, state);

    db.create_simulation(history_simulation);
    // --------------------------------------------------------------------------------------------
  }

  return 0;
}
