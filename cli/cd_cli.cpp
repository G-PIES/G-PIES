#include <array>
#include <boost/program_options.hpp>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>

#include "client_db/client_db.hpp"
#include "cluster_dynamics/cluster_dynamics.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/timer.hpp"

namespace po = boost::program_options;

std::ofstream output_file;
std::ostream os(std::cout.rdbuf());

bool csv = false;
bool step_print = false;
bool data_validation_on = true;

size_t concentration_boundary;
bool sensitivity_analysis_mode = false;
std::string sensitivity_analysis_variable = "";
size_t num_of_simulation_loops = 0;
gp_float delta_sensitivity_analysis = 0;
gp_float simulation_time;
gp_float delta_time;
gp_float sample_interval;  // How often (in seconds) to record the state

void print_start_message() {
  std::cout << "\nSimulation Started\n"
            << "delta time: " << delta_time
            << " simulation time: " << simulation_time
            << " concentration boundary: "
            << static_cast<int>(concentration_boundary)
            << " data validation: " << (data_validation_on ? "on" : "off")
            << std::endl;
}

void print_state(const ClusterDynamicsState& state) {
  os << "\nTime=" << state.time;

  if (state.interstitials.size() != concentration_boundary ||
      state.vacancies.size() != concentration_boundary) {
    throw ClusterDynamicsException("Output data is incorrectly sized.", state);
  }

  os << "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n";
  for (uint64_t n = 1; n < concentration_boundary; ++n) {
    os << (long long unsigned int)n << "\t\t\t\t\t" << std::setprecision(13)
       << state.interstitials[n] << "\t\t\t" << std::setprecision(15)
       << state.vacancies[n] << std::endl;
  }

  os << "\nDislocation Network Density: " << state.dislocation_density
     << std::endl;
}

void print_csv(const ClusterDynamicsState& state) {
  os << state.dpa;
  for (uint64_t n = 1; n < concentration_boundary; ++n) {
    os << "," << state.interstitials[n] << "," << state.vacancies[n];
  }
  os << std::endl;
}

void step_print_prompt(const ClusterDynamicsState& state) {
  if (csv) {
    print_csv(state);
  } else {
    print_state(state);
  }

  std::cout << "ENTER/RETURN to continue > ";
  std::cin.get();
}

void print_simulation_history(ClientDb& db, bool print_details) {
  std::vector<HistorySimulation> simulations;
  db.read_simulations(simulations);

  os << "\nSimulation History\tCount: "
     << static_cast<long long unsigned int>(simulations.size()) << std::endl;

  if (!simulations.empty()) {
    os << "ID ~ Concentration Boundary ~ Simulation Time ~ Delta "
          "Time ~ Reactor ~ Material ~ Creation Datetime\n\n";

    for (HistorySimulation s : simulations) {
      os << s.sqlite_id << " ~ "
         << static_cast<unsigned long long>(s.concentration_boundary) << " ~ "
         << s.simulation_time << " ~ " << s.delta_time << " ~ "
         << s.reactor.species << " ~ " << s.material.species << " ~ "
         << s.creation_datetime << std::endl;

      // Print the state(s) of the simulation
      if (print_details) {
        print_state(s.cd_state);
        os << "\n\n";
      }
    }

    os << std::endl;
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
    os << "N=" << n << std::endl;
    ClusterDynamics cd(n, reactor, material);

    timer.Start();
    state = cd.run(1e-5, 1e-5);
    gp_float time = timer.Stop();

    os << std::endl << time;
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
  cd.set_data_validation(data_validation_on);

  print_start_message();

  if (csv) {
    os << "Time (s),Cluster Size,Interstitials / cm^3,Vacancies / cm^3\n";
  }

  // TODO - support sample interval
  sample_interval = delta_time;

  ClusterDynamicsState state;
  // --------------------------------------------------------------------------------------------
  // main simulation loop
  for (gp_float t = 0; t < simulation_time; t = state.time) {
    // run simulation for this time slice
    state = cd.run(delta_time, sample_interval);

    if (step_print) {
      step_print_prompt(state);
    } else if (csv) {
      print_csv(state);
    }
  }
  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  // print results
  if (!csv && !step_print) {
    print_state(state);
  }
  // --------------------------------------------------------------------------------------------

  return state;
}

int main(int argc, char* argv[]) {
  try {
    // Declare the supported options
    po::options_description all_options("General options");
    all_options.add_options()("help", "display help message")(
        "csv", "csv output formatting")("step-print",
                                        "print state at every time step")(
        "output-file", po::value<std::string>()->value_name("filename"),
        "write output to a file")("db", "database options")(
        "data-validation",
        po::value<std::string>()->value_name("toggle")->implicit_value("on"),
        "turn on/off data validation (on by default)")(
        "sensitivity,s", "sensitivity analysis mode")(
        "sensitivity-var,v", po::value<std::string>(),
        "variable to do sensitivity analysis mode on (required sensitivity "
        "analysis)")("number-of-loops,n", po::value<int>()->implicit_value(2),
                     "number of loops you want to run in sensititivy analysis "
                     "mode (required sensitivty analysis)")(
        "delta-sensitivty-analysis,d", po::value<int>()->implicit_value(0),
        "amount to change the sensitivity-var by for each loop (required "
        "sensitivity analysis)");

    po::options_description db_options("Database options [--db]");
    db_options.add_options()("history,h", "display simulation history")(
        "history-detail", "display detailed simulation history")(
        "run,r", po::value<int>()->value_name("id"),
        "run a simulation from the history by [id]")(
        "clear,c", "clear simulation history");

    all_options.add(db_options);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, all_options), vm);
    po::notify(vm);

    // Help message
    if (vm.count("help")) {
      std::cout << all_options << "\n";
      return 1;
    }

    // Redirect output to file
    if (vm.count("output-file")) {
      std::string filename = vm["output-file"].as<std::string>();
      output_file.open(filename);
      if (output_file.is_open()) os.rdbuf(output_file.rdbuf());
    }

    // Output formatting
    csv = static_cast<bool>(vm.count("csv"));
    step_print = static_cast<bool>(vm.count("step-print"));

    // Toggle data validation
    if (vm.count("data-validation")) {
      data_validation_on =
          0 == vm["data-validation"].as<std::string>().compare("on");
    }

    ClientDb db(DEV_DEFAULT_CLIENT_DB_PATH, false);
    // Open SQLite connection and create database
    db.init();

    // Default values
    NuclearReactor reactor;
    Material material;
    nuclear_reactors::OSIRIS(reactor);
    materials::SA304(material);
    concentration_boundary = 10;
    simulation_time = 1.;
    delta_time = 1e-5;

    // --------------------------------------------------------------------------------------------
    // arg parsing
    if (vm.count("db")) {  // DATABASE
      if (vm.count("history")) {
        // print simulation history
        print_simulation_history(db, false);
      } else if (vm.count("history-detail")) {
        // print detailed simulation history
        print_simulation_history(db, true);
      } else if (vm.count("clear")) {
        // clear history
        if (db.delete_simulations()) {
          std::cout << "Simulation History Cleared. " << db.changes()
                    << " Simulation(s) Deleted.\n";
        }
      } else if (vm.count("run")) {
        // rerun a previous simulation by database id
        int sim_sqlite_id = vm["run"].as<int>();
        HistorySimulation sim;
        if (db.read_simulation(sim_sqlite_id, sim)) {
          // TODO - support storing sensitivity analysis
          std::cout << "Running simulation " << sim_sqlite_id << std::endl;
          concentration_boundary = sim.concentration_boundary;
          simulation_time = sim.simulation_time;

          // TODO - Support sample interval and set a max resolution to
          // avoid bloating the database. For this to work we will need a
          // list of ClusterDynamicState objects and a SQLite intersection
          // table.
          delta_time = sample_interval = sim.delta_time;

          run_simulation(sim.reactor, sim.material);
        } else {
          std::cerr << "Could not find simulation " << sim_sqlite_id
                    << std::endl;
        }
      }
    } else if (vm.count("sensitivity")) {  // SENSITIVITY ANALYSIS
      // Set sensitivity analysis mode to true
      sensitivity_analysis_mode = true;
      if (vm.count("sensitivity-var") && vm.count("number-of-loops") &&
          vm.count("delta-sensitivty-analysis")) {
        sensitivity_analysis_variable = vm["sensitivity-var"].as<std::string>();
        num_of_simulation_loops = vm["number-of-loops"].as<int>();
        delta_sensitivity_analysis = vm["delta-sensitivty-analysis"].as<int>();
      } else {
        std::cerr << "Missing required arguments for sensitivity "
                     "analysis, running "
                     "normal simulation";
      }

      // TODO - support sample interval
      sample_interval = delta_time;

      // --------------------------------------------------------------------------------------------
      // sensitivity analysis simulation loop
      for (size_t n = 0; n < num_of_simulation_loops; n++) {
        ClusterDynamics cd(concentration_boundary, reactor, material);
        cd.set_data_validation(data_validation_on);

        print_start_message();

        if (csv) {
          os << "Time (s),Cluster Size,"
                "Interstitials / cm^3,Vacancies / cm^3\n";
        }

        ClusterDynamicsState state;
        update_for_sensitivity_analysis(cd, reactor, material,
                                        n * delta_sensitivity_analysis);

        for (gp_float t = 0; t < simulation_time; t = state.time) {
          // run simulation for this time slice
          state = cd.run(delta_time, sample_interval);

          if (step_print) {
            step_print_prompt(state);
          } else if (csv) {
            print_csv(state);
          }
        }

        // ----------------------------------------------------------------
        // print results
        if (!step_print && !csv) {
          print_state(state);
        }
        // ----------------------------------------------------------------
      }
      // --------------------------------------------------------------------
    } else {  // CLUSTER DYNAMICS OPTIONS
      ClusterDynamicsState state = run_simulation(reactor, material);

      // --------------------------------------------------------------------------------------------
      // Write simulation result to the database
      HistorySimulation history_simulation(concentration_boundary,
                                           simulation_time, delta_time, reactor,
                                           material, state);

      db.create_simulation(history_simulation);
      // --------------------------------------------------------------------------------------------
      // --------------------------------------------------------------------
    }
  } catch (const ClusterDynamicsException& e) {
    std::cerr << "A simulation error occured.\n"
              << "Details: " << e.message << "\n"
              << "Last Simulation State:\n";
    print_state(e.err_state);
    std::exit(EXIT_FAILURE);
  } catch (const ClientDbException& e) {
    std::cerr << "A database error occured.\n"
              << e.message << "\n"
              << "SQLite Code: " << e.sqlite_code << "\n"
              << "SQLite Message: " << e.sqlite_errmsg << "\n";
    std::exit(EXIT_FAILURE);
  } catch (const GpiesException& e) {
    std::cerr << "An error occured.\n"
              << "Details: " << e.message << "\n";
    std::exit(EXIT_FAILURE);
  } catch (const std::exception& e) {
    std::cerr << "An unexpected error occured.\n"
              << "Details: " << e.what() << "\n";
    std::exit(EXIT_FAILURE);
  } catch (...) {
    std::cerr << "An unexpected error occured.\n";
    std::exit(EXIT_FAILURE);
  }

  if (output_file.is_open()) output_file.close();

  return 0;
}
