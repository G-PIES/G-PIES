#include <array>
#include <boost/program_options.hpp>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "client_db/client_db.hpp"
#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/timer.hpp"

namespace po = boost::program_options;

std::string filename;
std::ofstream output_file;
std::ostream os(std::cout.rdbuf());

bool csv = false;
bool step_print = false;

size_t sa_num_simulations = 0;
std::string sa_var = "";
gp_float sa_var_delta = 0.;

gp_float simulation_time = 1e8;
gp_float time_delta = 1e6;
gp_float sample_interval =
    time_delta;  // How often (in seconds) to record the state

ClusterDynamicsConfig config;

void print_start_message() {
  std::cout << "\nSimulation Started\n"
            << "time delta: " << time_delta
            << "  simulation time: " << simulation_time
            << "  max cluster size: " << static_cast<int>(config.max_cluster_size)
            << "  data validation: " << (config.data_validation_on ? "on" : "off")
            << std::endl
            << "Integration Settings\n"
            << "  relative tolerance: " << config.relative_tolerance
            << "  absolute tolerance: " << config.absolute_tolerance
            << "  max num integration steps: " << config.max_num_integration_steps
            << "  min integration step: " << config.min_integration_step
            << "  max integration step: " << config.max_integration_step
            << std::endl;
}

void print_state(const ClusterDynamicsState& state) {
  os << "\nTime=" << state.time;

  size_t size = state.interstitials.size();

  os << "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n";
  for (size_t n = 1; n < size; ++n) {
    os << (long long unsigned int)n << "\t\t\t\t\t" << std::setprecision(13)
       << state.interstitials[n] << "\t\t\t" << std::setprecision(15)
       << state.vacancies[n] << std::endl;
  }

  os << "\nDislocation Network Density: " << state.dislocation_density
     << std::endl;
}

void print_csv(const ClusterDynamicsState& state) {
  os << state.time;
  for (uint64_t n = 1; n < config.max_cluster_size; ++n) {
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
         << static_cast<unsigned long long>(s.max_cluster_size) << " ~ "
         << s.simulation_time << " ~ " << s.time_delta << " ~ "
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

  nuclear_reactors::OSIRIS(config.reactor);

  materials::SA304(config.material);

  ClusterDynamics cd(config);
  cd.run(1e-5, 1e-5);

  for (int n = 100; n < 400000; n += 10000) {
    os << "N=" << n << std::endl;
    ClusterDynamics cd(config);

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
  switch (hashit(sa_var)) {
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

ClusterDynamicsState run_simulation() {
  ClusterDynamics cd(config);

  print_start_message();

  if (csv) {
    os << "Time (s),";
    for (size_t i = 1; i < config.max_cluster_size; ++i)
    {
      os << "i" << i << ",v" << i << ",";
    }
    os << "\n";
  }

  // TODO - support sample interval
  sample_interval = time_delta;

  ClusterDynamicsState state;
  // --------------------------------------------------------------------------------------------
  // main simulation loop
  for (gp_float t = 0; t < simulation_time; t = state.time) {
    // run simulation for this time slice
    state = cd.run(time_delta, sample_interval);

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

/* TODO - Remove
void valid_integration_search() {
  Timer timer;

  ClusterDynamicsState state;

  NuclearReactor reactor;
  nuclear_reactors::OSIRIS(reactor);

  Material material;
  materials::SA304(material);


  for (size_t n = 100; n < 1000; n += 100) {
    max_cluster_size = n;
    gp_float td = 10.;
    gp_float t = 100.;
    gp_float rt = relative_tolerance;
    gp_float at = absolute_tolerance;
    gp_float minis = 1e-5;
    gp_float maxis = max_integration_step;

    for (size_t i = 0; i < 21; ++i) {
      //time_delta = td;
      //simulation_time = t;
      relative_tolerance = rt;
      absolute_tolerance = at;
      min_integration_step = minis;
      max_integration_step = maxis;

      try {
        timer.Start();
        state = run_simulation(reactor, material);
        gp_float time = timer.Stop();
        os << "\nValid Simulation\n"
          << "time delta: " << time_delta
          << "  simulation time: " << simulation_time
          << "  max cluster size: " << static_cast<int>(max_cluster_size)
          << std::endl
          << "Integration Settings\n"
          << "  relative tolerance: " << relative_tolerance
          << "  absolute tolerance: " << absolute_tolerance
          << "  max num integration steps: " << max_num_integration_steps
          << "  min integration step: " << min_integration_step
          << "  max integration step: " << max_integration_step 
          << "\nTime Elapsed: " << time
          << std::endl;
          print_state(state);
      } catch (...) {
        timer.Stop();
      }

      //td *= 10.;
      //t *= 10.;
      //rt *= 10.;
      //at *= 10.;
      minis /= 10.;
      //maxis *= 10.;
    }
  }
}
*/

int main(int argc, char* argv[]) {
  // TODO - Remove (this is just for testing the CVODES implementation)
  //valid_integration_search();
  //return 0;

  materials::SA304(config.material);
  nuclear_reactors::OSIRIS(config.reactor);

  try {
    // Declare the supported options
    po::options_description all_options("General Options");
    all_options.add_options()("help", "display help message")(
        "csv", "csv output formatting")(
        "step-print", "print simulation state at every time step")(
        "output-file", po::value<std::string>()->value_name("filename"),
        "write simulation output to a file")("db", "database options")(
        "data-validation",
        po::value<std::string>()->value_name("toggle")->implicit_value("on"),
        "turn on/off data validation (on by default)")(
        "max-cluster-size",
        po::value<size_t>()->implicit_value(config.max_cluster_size),
        "set the max size of defect clustering to model")(
        "time", po::value<gp_float>()->implicit_value(simulation_time),
        "the simulation environment time span to model (in seconds)")(
        "time-delta", po::value<gp_float>()->implicit_value(time_delta),
        "the time delta for every step of the simulation (in seconds)")(
        "sample-interval",
        po::value<gp_float>()->implicit_value(sample_interval),
        "how often to record simulation environment state (in seconds)")
        ("relative-tolerance",
        po::value<gp_float>()->implicit_value(config.relative_tolerance),
        "scalar relative tolerance for integration error")
        ("absolute-tolerance",
        po::value<gp_float>()->implicit_value(config.absolute_tolerance),
        "absolute relative tolerance for integration error")
        ("max-num-integration-steps",
        po::value<size_t>()->implicit_value(config.max_num_integration_steps),
        "maximum allowed number of integration steps to achieve a single estimation")
        ("min-integration-step",
        po::value<gp_float>()->implicit_value(config.min_integration_step),
        "minimum step size for integration")
        ("max-integration-step",
        po::value<gp_float>()->implicit_value(config.max_integration_step),
        "maximum step size for integration");

    po::options_description db_options("Database Options [--db]");
    db_options.add_options()("history,h", "display simulation history")(
        "history-detail", "display detailed simulation history")(
        "run,r", po::value<int>()->value_name("id"),
        "run a simulation from the history by [id]")(
        "clear,c", "clear simulation history");

    po::options_description sa_options(
        "Sensitivity Analysis Options [--sensitivity-analysis]");
    sa_options.add_options()(
        "sensitivity-analysis-help",
        "display sensitivity analysis supported variables and example command")(
        "sensitivity-analysis,s", "use sensitivity analysis mode")(
        "num-sims,n", po::value<int>()->implicit_value(2),
        "number of simulations you want to run in sensititivy analysis "
        "mode (REQUIRED)")(
        "sensitivity-var,v", po::value<std::string>()->value_name("var name"),
        "specify the variable to do sensitivity analysis on (REQUIRED)")(
        "sensitivity-var-delta,d", po::value<gp_float>(),
        "amount to change [sensitivity-var] by for each simulation (REQUIRED)");

    all_options.add(db_options).add(sa_options);

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, all_options), vm);
    po::notify(vm);

    // Help message
    if (vm.count("help")) {
      std::cout << all_options << "\n";
      return 1;
    } else if (vm.count("sensitivity-analysis-help")) {
      // TODO - refactor supported variables, description, and corresponding
      // enums
      std::cout << "\nSupported Variables [--sensitivity-var]:\n"
                << "i_migration\n"
                << "v_migration\n"
                << "i_formation\n"
                << "v_formation\n"
                << "i_binding\n"
                << "v_binding\n"
                << "dislocation_density_0\n"
                << "flux\n"
                << "temperature\n"
                << "dislocation_density_evolution\n";

      std::cout << "\nexample command: run 10 simulations, increasing the "
                   "reactor flux by 1e-7 for each simulation\n"
                << "./cd_cli --sensitivity-analysis --num-sims 10 "
                   "--sensitivity-var flux --sensitivity-var-delta 1e-7\n\n";
      return 1;
    }

    // Redirect output to file
    if (vm.count("output-file")) {
      filename = vm["output-file"].as<std::string>();
      output_file.open(filename);
      if (output_file.is_open()) os.rdbuf(output_file.rdbuf());
    }

    if (vm.count("max-cluster-size")) {
      size_t mcs = vm["max-cluster-size"].as<size_t>();
      if (mcs <= 0)
        throw GpiesException(
            "Value for max-cluster-size must be a positive, non-zero integer.");

      config.max_cluster_size = mcs;
    }

    if (vm.count("time")) {
      gp_float st = vm["time"].as<gp_float>();
      if (st <= 0.)
        throw GpiesException(
            "Value for time must be a positive, non-zero decimal.");

      simulation_time = st;
    }

    if (vm.count("time-delta")) {
      gp_float td = vm["time-delta"].as<gp_float>();
      if (td <= 0.)
        throw GpiesException(
            "Value for time-delta must be a positive, non-zero "
            "decimal.");

      time_delta = td;
    }

    if (vm.count("sample-interval")) {
      gp_float si = vm["sample-interval"].as<gp_float>();
      if (si <= 0.)
        throw GpiesException(
            "Value for sample-interval must be a positive, non-zero "
            "decimal.");

      sample_interval = si;
    }

    if (vm.count("relative-tolerance")) {
      gp_float rt = vm["relative-tolerance"].as<gp_float>();
      if (rt <= 0.)
        throw GpiesException(
            "Value for relative-tolerance must be a positive, non-zero "
            "decimal.");

      config.relative_tolerance = rt;
    }

    if (vm.count("absolute-tolerance")) {
      gp_float at = vm["absolute-tolerance"].as<gp_float>();
      if (at <= 0.)
        throw GpiesException(
            "Value for absolute-tolerance must be a positive, non-zero "
            "decimal.");

      config.absolute_tolerance = at;
    }

    if (vm.count("max-num-integration-steps")) {
      size_t mnis = vm["max-num-integration-steps"].as<size_t>();
      if (mnis <= 0)
        throw GpiesException(
            "Value for max-num-integration-steps must be a positive, non-zero "
            "integer.");

      config.max_num_integration_steps = mnis;
    }

    if (vm.count("min-integration-step")) {
      gp_float minis = vm["min-integration-step"].as<gp_float>();
      if (minis <= 0.)
        throw GpiesException(
            "Value for min-integration-step must be a positive, non-zero "
            "decimal.");

      config.min_integration_step = minis;
    }

    if (vm.count("max-integration-step")) {
      gp_float maxis = vm["max-integration-step"].as<gp_float>();
      if (maxis <= 0.)
        throw GpiesException(
            "Value for max-integration-step must be a positive, non-zero "
            "decimal.");

      config.max_integration_step = maxis;
    }

    // Output formatting
    csv = static_cast<bool>(vm.count("csv"));
    step_print = static_cast<bool>(vm.count("step-print"));

    // Toggle data validation
    if (vm.count("data-validation")) {
      config.data_validation_on =
          0 == vm["data-validation"].as<std::string>().compare("on");
    }

    ClientDb db(DEV_DEFAULT_CLIENT_DB_PATH, false);
    // Open SQLite connection and create database
    db.init();

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
          config.max_cluster_size = sim.max_cluster_size;
          simulation_time = sim.simulation_time;

          // TODO - Support sample interval and set a max resolution to
          // avoid bloating the database. For this to work we will need a
          // list of ClusterDynamicState objects and a SQLite intersection
          // table.
          time_delta = sample_interval = sim.time_delta;

          config.material = sim.material;
          config.reactor = sim.reactor;

          run_simulation();
        } else {
          std::cerr << "Could not find simulation " << sim_sqlite_id
                    << std::endl;
        }
      }
    } else if (vm.count("sensitivity-analysis")) {  // SENSITIVITY ANALYSIS
      // Set sensitivity analysis mode to true
      if (vm.count("num-sims") && vm.count("sensitivity-var") &&
          vm.count("sensitivity-var-delta")) {
        sa_num_simulations = vm["num-sims"].as<int>();

        if (sa_num_simulations <= 0)
          throw GpiesException(
              "Value for num-sims must be a positive, non-zero integer.");

        sa_var = vm["sensitivity-var"].as<std::string>();
        sa_var_delta = vm["sensitivity-var-delta"].as<gp_float>();
      } else {
        throw GpiesException(
            "Missing required arguments for sensitivity "
            "analysis.\n--help to see required variables.");
      }

      // TODO - support sample interval
      sample_interval = time_delta;

      // --------------------------------------------------------------------------------------------
      // sensitivity analysis simulation loop
      for (size_t n = 0; n < sa_num_simulations; n++) {
        ClusterDynamics cd(config);

        ClusterDynamicsState state;
        update_for_sensitivity_analysis(
            cd, config.reactor, config.material, static_cast<gp_float>(n) * sa_var_delta);

        if (n > 0) os << "\n";  // visual divider for consecutive sims

        if (csv) {
          os << "Simulation " << n + 1 << ",Sensitivity Variable: " << sa_var
             << ",Delta=" << static_cast<gp_float>(n) * sa_var_delta << "\n\n";
          os << "Time (s),Cluster Size,"
                "Interstitials / cm^3,Vacancies / cm^3\n";
        } else {
          os << "Simulation " << n + 1 << "\tSensitivity Variable: " << sa_var
             << "\tDelta=" << static_cast<gp_float>(n) * sa_var_delta
             << std::endl;
        }

        print_start_message();

        for (gp_float t = 0; t < simulation_time; t = state.time) {
          // run simulation for this time slice
          state = cd.run(time_delta, sample_interval);

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
      ClusterDynamicsState state = run_simulation();

      // --------------------------------------------------------------------------------------------
      // Write simulation result to the database
      HistorySimulation history_simulation(config.max_cluster_size, simulation_time,
                                           time_delta, config.reactor, config.material,
                                           state);

      db.create_simulation(history_simulation);
      // --------------------------------------------------------------------------------------------
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

  if (output_file.is_open()) {
    std::cout << "\nResults written to: " << filename << std::endl;
    output_file.close();
  } 

  return 0;
}
