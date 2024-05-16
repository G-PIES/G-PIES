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
#include "utils/consumers/cli_arg_consumer.hpp"
#include "utils/progress_bar.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/timer.hpp"

namespace po = boost::program_options;

std::string filename;
std::ofstream output_file;
std::ostream os(std::cout.rdbuf());

bool csv = false;
bool step_print = false;

ClusterDynamicsConfig cd_config;

void print_reactor() {
  std::cout
      << cd_config.reactor.species << "\nflux: " << cd_config.reactor.get_flux()
      << " dpa/s"
      << "\ntemperature: " << cd_config.reactor.get_temperature() << " kelvin"
      << "\nrecombination rate: " << cd_config.reactor.get_recombination()
      << "\nbi-interstitial generation rate: " << cd_config.reactor.get_i_bi()
      << "\ntri-interstitial generation rate: " << cd_config.reactor.get_i_tri()
      << "\nquad-interstitial generation rate: "
      << cd_config.reactor.get_i_quad()
      << "\nbi-vacancy generation rate: " << cd_config.reactor.get_v_bi()
      << "\ntri-vacancy generation rate: " << cd_config.reactor.get_v_tri()
      << "\nquad-vacancy generation rate: " << cd_config.reactor.get_v_quad()
      << "\ndislocation density evolution: "
      << cd_config.reactor.get_dislocation_density_evolution() << std::endl;
}

void print_material() {
  std::cout << cd_config.material.species << "\ninterstitial migration: "
            << cd_config.material.get_i_migration() << " eV"
            << "\nvacancy migration: " << cd_config.material.get_v_migration()
            << " eV"
            << "\ninitial interstitial diffusion: "
            << cd_config.material.get_i_diffusion_0() << " cm^2/s"
            << "\ninitial vacancy diffusion: "
            << cd_config.material.get_v_diffusion_0() << " cm^2/s"
            << "\ninterstitial formation: "
            << cd_config.material.get_i_formation() << " eV"
            << "\nvacancy formation: " << cd_config.material.get_v_formation()
            << " eV"
            << "\ninterstitial binding: " << cd_config.material.get_i_binding()
            << " eV"
            << "\nvacancy binding: " << cd_config.material.get_v_binding()
            << " eV"
            << "\nrecombination radius: "
            << cd_config.material.get_recombination_radius() << " cm"
            << "\ninterstitial loop bias: "
            << cd_config.material.get_i_loop_bias()
            << "\ninterstitial dislocation bias: "
            << cd_config.material.get_i_dislocation_bias()
            << "\ninterstitial dislocation bias param: "
            << cd_config.material.get_i_dislocation_bias_param()
            << "\nvacancy loop bias: " << cd_config.material.get_v_loop_bias()
            << "\nvacancy dislocation bias: "
            << cd_config.material.get_v_dislocation_bias()
            << "\nvacancy dislocation bias param: "
            << cd_config.material.get_v_dislocation_bias_param()
            << "\ninitial dislocation density: "
            << cd_config.material.get_dislocation_density_0() << " cm^-2"
            << "\ngrain size: " << cd_config.material.get_grain_size() << " cm"
            << "\nlattice parameter: " << cd_config.material.get_lattice_param()
            << " cm"
            << "\nburgers vector (lattice_parameter / sqrt(2)): "
            << cd_config.material.get_burgers_vector() << " cm"
            << "\natomic volume (lattice_parameter^3 / 4): "
            << cd_config.material.get_atomic_volume() << "cm^3" << std::endl;
}

void print_start_message() {
  std::cout << "\nG-PIES SIMULATION CONFIGURATION\n"
            << "simulation time: " << cd_config.simulation_time
            << "  time delta: " << cd_config.time_delta
            << "  sample interval: " << cd_config.sample_interval
            << "  max cluster size: "
            << static_cast<int>(cd_config.max_cluster_size)
            << "  data validation: "
            << (cd_config.data_validation_on ? "on" : "off") << std::endl
            << "Integration Settings\n"
            << "  relative tolerance: " << cd_config.relative_tolerance
            << "  absolute tolerance: " << cd_config.absolute_tolerance
            << "  max num integration steps: "
            << cd_config.max_num_integration_steps
            << "  min integration step: " << cd_config.min_integration_step
            << "  max integration step: " << cd_config.max_integration_step
            << std::endl;

  std::cout << "\nReactor Settings\n";
  print_reactor();

  std::cout << "\nMaterial Settings\n";
  print_material();

  std::cout << std::endl;

  std::cout << "\nInitial Defect Clustering";
  bool is_perfect_lattice = true;
  for (size_t n = 1; n < cd_config.max_cluster_size; ++n) {
    if (cd_config.init_interstitials[n] > 0. ||
        cd_config.init_vacancies[n] > 0.) {
      // only print header if there is information to display
      if (is_perfect_lattice) {
        is_perfect_lattice = false;
        std::cout
            << "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n";
      }
      std::cout << (long long unsigned int)n << "\t\t\t\t\t"
                << cd_config.init_interstitials[n] << "\t\t\t"
                << cd_config.init_vacancies[n] << std::endl;
    }
  }

  // indicate that there are no defect clusters
  if (is_perfect_lattice) {
    std::cout << ": No Defects (Perfect Lattice)\n";
  }

  std::cout << std::endl;
}

void print_state(const ClusterDynamicsState& state) {
  os << "\nTime=" << state.time;

  size_t size = state.interstitials.size();

  os << "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n";
  for (size_t n = 1; n < size; ++n) {
    for (size_t n = 1; n < size; ++n) {
      os << (long long unsigned int)n << "\t\t\t\t\t" << std::setprecision(13)
         << state.interstitials[n] << "\t\t\t" << std::setprecision(15)
         << state.vacancies[n] << std::endl;
    }

    os << "\nDislocation Network Density: " << state.dislocation_density
       << std::endl;
  }
}

void print_csv(const ClusterDynamicsState& state) {
  os << state.time << ", " << state.dislocation_density;
  for (uint64_t n = 1; n < cd_config.max_cluster_size; ++n) {
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
    os << "ID ~ Max Cluster Size ~ Simulation Time ~ Delta "
          "Time ~ Reactor ~ Material ~ Simulation Completion Datetime\n\n";

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

  nuclear_reactors::OSIRIS(cd_config.reactor);
  materials::SA304(cd_config.material);

  ClusterDynamics cd = ClusterDynamics::cpu(cd_config);
  cd.run(1e-5, 1e-5);

  for (int n = 100; n < 400000; n += 10000) {
    os << "N=" << n << std::endl;
    ClusterDynamics cd = ClusterDynamics::cpu(cd_config);

    timer.Start();
    state = cd.run(1e-5, 1e-5);
    gp_float time = timer.Stop();

    os << std::endl << time;
  }
}

gp_float sa_update_config() {
  switch (cd_config.sa_var) {
    case SensitivityVariable::interstitial_migration_ev:
      cd_config.material.set_i_migration(cd_config.material.get_i_migration() +
                                         cd_config.sa_var_delta);
      return cd_config.material.get_i_migration();
    case SensitivityVariable::vacancy_migration_ev:
      cd_config.material.set_v_migration(cd_config.material.get_v_migration() +
                                         cd_config.sa_var_delta);
      return cd_config.material.get_v_migration();
    case SensitivityVariable::interstitial_formation_ev:
      cd_config.material.set_i_formation(cd_config.material.get_i_formation() +
                                         cd_config.sa_var_delta);
      return cd_config.material.get_i_formation();
    case SensitivityVariable::vacancy_formation_ev:
      cd_config.material.set_v_formation(cd_config.material.get_v_formation() +
                                         cd_config.sa_var_delta);
      return cd_config.material.get_v_formation();
    case SensitivityVariable::interstitial_binding_ev:
      cd_config.material.set_i_binding(cd_config.material.get_i_binding() +
                                       cd_config.sa_var_delta);
      return cd_config.material.get_i_binding();
    case SensitivityVariable::vacancy_binding_ev:
      cd_config.material.set_v_binding(cd_config.material.get_v_binding() +
                                       cd_config.sa_var_delta);
      return cd_config.material.get_v_binding();
    case SensitivityVariable::initial_dislocation_density_cm:
      cd_config.material.set_dislocation_density_0(
          cd_config.material.get_dislocation_density_0() +
          cd_config.sa_var_delta);
      return cd_config.material.get_dislocation_density_0();
    case SensitivityVariable::flux_dpa_s:
      cd_config.reactor.set_flux(cd_config.reactor.get_flux() +
                                 cd_config.sa_var_delta);
      return cd_config.reactor.get_flux();
    case SensitivityVariable::temperature_kelvin:
      cd_config.reactor.set_temperature(cd_config.reactor.get_temperature() +
                                        cd_config.sa_var_delta);
      return cd_config.reactor.get_temperature();
    case SensitivityVariable::dislocation_density_evolution:
      cd_config.reactor.set_dislocation_density_evolution(
          cd_config.reactor.get_dislocation_density_evolution() +
          cd_config.sa_var_delta);
      return cd_config.reactor.get_dislocation_density_evolution();
    default:
      break;
  }

  return 0.;
}

gp_float get_sa_var_value() {
  switch (cd_config.sa_var) {
    case SensitivityVariable::interstitial_migration_ev:
      return cd_config.material.get_i_migration();
    case SensitivityVariable::vacancy_migration_ev:
      return cd_config.material.get_v_migration();
    case SensitivityVariable::interstitial_formation_ev:
      return cd_config.material.get_i_formation();
    case SensitivityVariable::vacancy_formation_ev:
      return cd_config.material.get_v_formation();
    case SensitivityVariable::interstitial_binding_ev:
      return cd_config.material.get_i_binding();
    case SensitivityVariable::vacancy_binding_ev:
      return cd_config.material.get_v_binding();
    case SensitivityVariable::initial_dislocation_density_cm:
      return cd_config.material.get_dislocation_density_0();
    case SensitivityVariable::flux_dpa_s:
      return cd_config.reactor.get_flux();
    case SensitivityVariable::temperature_kelvin:
      return cd_config.reactor.get_temperature();
    case SensitivityVariable::dislocation_density_evolution:
      return cd_config.reactor.get_dislocation_density_evolution();
    default:
      break;
  }

  return 0.;
}

ClusterDynamicsState run_simulation() {
  ClusterDynamics cd = ClusterDynamics::cpu(cd_config);

  print_start_message();

  if (csv) {
    os << "Time (s), Dislocation Density (cm^-2),";
    for (size_t i = 1; i < cd_config.max_cluster_size; ++i) {
      os << "i" << i << ",v" << i << ",";
    }
    os << "\n";
  }

  ClusterDynamicsState state;

  progressbar bar(
      static_cast<int>(cd_config.simulation_time / cd_config.time_delta), true,
      std::cout);
  bar.set_todo_char(" ");
  bar.set_done_char("█");
  bar.set_opening_bracket_char("[");
  bar.set_closing_bracket_char("]");

  if (!step_print) {
    std::cout << "G-PIES Simulation Running..." << std::endl;
  }

  // --------------------------------------------------------------------------------------------
  // main simulation loop
  for (gp_float t = 0.; t < cd_config.simulation_time; t = state.time) {
    if (!step_print) {
      bar.update();
    }

    // run simulation for this time slice
    state = cd.run(cd_config.time_delta, cd_config.sample_interval);

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

void emit_config_yaml(const std::string& filename) {
  YAML::Emitter out;
  YAML::Emitter sa_comment;
  YAML::Emitter arrays_comment;

  arrays_comment
      << YAML::BeginMap << YAML::Key << "init-interstitials" << YAML::Value
      << YAML::BeginMap << YAML::Key << "array" << YAML::Value << YAML::Flow
      << std::vector<std::string>{"1.0e-13", "6.0e-10", "4.0e-20", "2.0e-1"}
      << YAML::Comment("cluster concentrations for cluster sizes 1-4")
      << YAML::Key << "ranges" << YAML::Value << YAML::BeginSeq << YAML::Flow
      << std::vector<std::string>{"50", "55", "9.9e-30"}
      << YAML::Comment("cluster concentrations for cluster sizes 50-55")
      << YAML::Flow << std::vector<std::string>{"90", "102", "5.5e-29"}
      << YAML::Comment("cluster concentrations for cluster sizes 90-102")
      << YAML::Flow << std::vector<std::string>{"995", "1000", "2.2e-22"}
      << YAML::Comment("cluster concentrations for cluster sizes 995-1000")
      << YAML::EndSeq << YAML::EndMap << YAML::EndMap << YAML::Newline
      << YAML::Newline << YAML::BeginMap << YAML::Key << "init-vacancies"
      << YAML::Value << YAML::BeginMap << YAML::Key << "array" << YAML::Value
      << YAML::Flow
      << std::vector<std::string>{"1.0e-5", "2.3e-12", "4.2e-20", "1.3e-13"}
      << YAML::Comment("cluster concentrations for cluster sizes 1-4")
      << YAML::Key << "ranges" << YAML::Value << YAML::BeginSeq << YAML::Flow
      << std::vector<std::string>{"50", "55", "6.2e-29"}
      << YAML::Comment("cluster concentrations for cluster sizes 50-55")
      << YAML::Flow << std::vector<std::string>{"90", "102", "1.5e-13"}
      << YAML::Comment("cluster concentrations for cluster sizes 90-102")
      << YAML::Flow << std::vector<std::string>{"995", "1000", "4.3e-12"}
      << YAML::Comment("cluster concentrations for cluster sizes 995-1000")
      << YAML::EndSeq << YAML::EndMap << YAML::EndMap;
  sa_comment << YAML::BeginMap << YAML::Key << "sensitivity-analysis"
             << YAML::Value << YAML::BeginMap << YAML::Key << "num-sims"
             << YAML::Value << "10" << YAML::Key << "sensitivity-var"
             << YAML::Value << "flux-dpa-s" << YAML::Key
             << "sensitivity-var-delta" << YAML::Value << "1.0e-7"
             << YAML::EndMap << YAML::EndMap;

  out << YAML::BeginMap << YAML::Key << "simulation" << YAML::Value
      << YAML::BeginMap << YAML::Key << "time" << YAML::Value << "1.0e+8"
      << YAML::Key << "time-delta" << YAML::Value << "1.0e+6" << YAML::Key
      << "sample-interval" << YAML::Value << "1.0e+6" << YAML::Key
      << "data-validation" << YAML::Value << "on" << YAML::Key
      << "max-cluster-size" << YAML::Value << "1001" << YAML::Key
      << "relative-tolerance" << YAML::Value << "1.0e-6" << YAML::Key
      << "absolute-tolerance" << YAML::Value << "1.0e+1" << YAML::Key
      << "max-num-integration-steps" << YAML::Value << "5000" << YAML::Key
      << "min-integration-step" << YAML::Value << "1.0e-30" << YAML::Key
      << "max-integration-step" << YAML::Value << "1.0e+20" << YAML::EndMap
      << YAML::EndMap << YAML::Newline << YAML::Newline << YAML::BeginMap
      << YAML::Key << "reactor" << YAML::Value << YAML::BeginMap << YAML::Key
      << "flux-dpa-s" << YAML::Value << "2.9e-7" << YAML::Key
      << "temperature-kelvin" << YAML::Value << "603.15" << YAML::Key
      << "recombination-rate" << YAML::Value << "0.3" << YAML::Key
      << "bi-interstitial-rate" << YAML::Value << "0.5" << YAML::Key
      << "tri-interstitial-rate" << YAML::Value << "0.2" << YAML::Key
      << "quad-interstitial-rate" << YAML::Value << "0.06" << YAML::Key
      << "bi-vacancy-rate" << YAML::Value << "0.06" << YAML::Key
      << "tri-vacancy-rate" << YAML::Value << "0.03" << YAML::Key
      << "quad-vacancy-rate" << YAML::Value << "0.02" << YAML::Key
      << "dislocation-density-evolution" << YAML::Value << "300.0"
      << YAML::EndMap << YAML::EndMap << YAML::Newline << YAML::Newline
      << YAML::BeginMap << YAML::Key << "material" << YAML::Value
      << YAML::BeginMap << YAML::Key << "interstitial-migration-ev"
      << YAML::Value << "0.45" << YAML::Key << "vacancy-migration-ev"
      << YAML::Value << "1.35" << YAML::Key << "initial-interstitial-diffusion"
      << YAML::Value << "1.0e-3" << YAML::Key << "initial-vacancy-diffusion"
      << YAML::Value << "0.6" << YAML::Key << "interstitial-formation-ev"
      << YAML::Value << "4.1" << YAML::Key << "vacancy-formation-ev"
      << YAML::Value << "4.1" << YAML::Key << "interstitial-binding-ev"
      << YAML::Value << "0.6" << YAML::Key << "vacancy-binding-ev"
      << YAML::Value << "0.5" << YAML::Key << "recombination-radius-cm"
      << YAML::Value << "0.7e-7" << YAML::Key << "interstitial-loop-bias"
      << YAML::Value << "63.0" << YAML::Key << "interstitial-dislocation-bias"
      << YAML::Value << "0.8" << YAML::Key
      << "interstitial-dislocation-bias-param" << YAML::Value << "1.1"
      << YAML::Key << "vacancy-loop-bias" << YAML::Value << "33.0" << YAML::Key
      << "vacancy-dislocation-bias" << YAML::Value << "0.65" << YAML::Key
      << "vacancy-dislocation-bias-param" << YAML::Value << "1.0" << YAML::Key
      << "initial-dislocation-density-cm^-2" << YAML::Value << "10.0e+12"
      << YAML::Key << "grain-size-cm" << YAML::Value << "4.0e-3" << YAML::Key
      << "lattice-param-cm" << YAML::Value << "3.6e-8" << YAML::EndMap
      << YAML::EndMap << YAML::Newline << YAML::Newline << YAML::Newline
      << YAML::Comment(
             "#################################################################"
             "####")
      << YAML::Newline
      << YAML::Comment(
             "INITIAL DEFECT CLUSTER CONCENTRATIONS "
             "###############################")
      << YAML::Newline
      << YAML::Comment(
             "you can define a subset of the array of concentrations with "
             "\"array\"")
      << YAML::Newline
      << YAML::Comment(
             "and define ranges throughout the array with \"ranges\" ")
      << YAML::Newline
      << YAML::Comment(
             "#################################################################"
             "####")
      << YAML::Newline << YAML::Newline
      << YAML::Comment(
             "UNCOMMENT LINES BELOW TO DEFINE INITIAL DEFECT CLUSTER "
             "CONCENTRATIONS")
      << YAML::Newline << YAML::Comment(arrays_comment.c_str()) << YAML::Newline
      << YAML::Newline
      << YAML::Comment("UNCOMMENT LINES BELOW TO TURN ON SENSITIVITY ANALYSIS")
      << YAML::Newline << YAML::Comment(sa_comment.c_str()) << YAML::Newline;

  std::ofstream file;
  file.open(filename);
  if (file.is_open()) {
    file << out.c_str();
    file.close();
    std::cout << "config file created: " << filename << std::endl;
  } else {
    std::cerr << "failed to create config file: " << filename << std::endl;
  }
}

int main(int argc, char* argv[]) {
  try {
    // Declare the supported options
    po::options_description all_options("General Options");
    all_options.add_options()("help", "display help message")(
        "version", "display version information")(
        "config", po::value<std::string>(),
        "configure simulation with a .yaml file")(
        "generate-config-file",
        po::value<std::string>()
            ->value_name("filename")
            ->implicit_value("config.yaml"),
        "generate an example .yaml config file")("csv",
                                                 "csv output formatting")(
        "step-print", "display simulation state at every time step")(
        "output-file", po::value<std::string>()->value_name("filename"),
        "write simulation output to a file")("db", "database options")(
        "data-validation",
        po::value<std::string>()->value_name("toggle")->implicit_value("on"),
        "turn on/off data validation (on by default)")(
        "max-cluster-size",
        po::value<size_t>()->implicit_value(cd_config.max_cluster_size),
        "set the max size of defect clustering to model")(
        "time",
        po::value<gp_float>()->implicit_value(cd_config.simulation_time),
        "the simulation environment time span to model (in seconds)")(
        "time-delta",
        po::value<gp_float>()->implicit_value(cd_config.time_delta),
        "the time delta for every step of the simulation (in seconds)")(
        "sample-interval",
        po::value<gp_float>()->implicit_value(cd_config.sample_interval),
        "how often to record simulation environment state (in seconds)")(
        "relative-tolerance",
        po::value<gp_float>()->implicit_value(cd_config.relative_tolerance),
        "relative tolerance for integration error")(
        "absolute-tolerance",
        po::value<gp_float>()->implicit_value(cd_config.absolute_tolerance),
        "absolute tolerance for integration solution error")(
        "max-num-integration-steps",
        po::value<size_t>()->implicit_value(
            cd_config.max_num_integration_steps),
        "maximum allowed number of integration steps to achieve a single "
        "estimation")(
        "min-integration-step",
        po::value<gp_float>()->implicit_value(cd_config.min_integration_step),
        "minimum step size for integration")(
        "max-integration-step",
        po::value<gp_float>()->implicit_value(cd_config.max_integration_step),
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

    CliArgConsumer arg_consumer(argc, argv, all_options);

    // Help message
    if (arg_consumer.has_arg("help")) {
      std::cout << all_options << "\n";
      return 1;
    } else if (arg_consumer.has_arg("sensitivity-analysis-help")) {
      std::cout << "\nSupported Variables [--sensitivity-var]:\n";
      for (const auto& [key, value] : sensitivity_variables) {
        std::cout << key << std::endl;
      }

      std::cout
          << "\nexample command: run 10 simulations, increasing the "
             "reactor flux by 1e-7 for each simulation\n"
          << "./gpies --sensitivity-analysis --num-sims 10 "
             "--sensitivity-var flux-dpa-s --sensitivity-var-delta 1e-7\n\n";
      return 1;
    } else if (arg_consumer.has_arg("version")) {
      std::cout << "G-PIES version " << GPIES_SEMANTIC_VERSION << "\n";
      return 1;
    } else if (arg_consumer.has_arg("generate-config-file")) {
      std::string filename =
          arg_consumer.get_value<std::string>("generate-config-file");
      emit_config_yaml(filename);
      return 1;
    }

    // Redirect output to file
    if (arg_consumer.has_arg("output-file")) {
      filename = arg_consumer.get_value<std::string>("output-file");
      output_file.open(filename);
      if (output_file.is_open()) {
        std::cout << "\nOutput File: " << filename << std::endl;
        os.rdbuf(output_file.rdbuf());
      }
    }

    // Output formatting
    csv = static_cast<bool>(arg_consumer.has_arg("csv", "simulation"));
    step_print =
        static_cast<bool>(arg_consumer.has_arg("step-print", "simulation"));

    // Get cluster dynamics configuration
    arg_consumer.populate_cd_config(cd_config);

    ClientDb db(DEV_DEFAULT_CLIENT_DB_PATH, false);
    // Open SQLite connection and create database
    db.init();

    // --------------------------------------------------------------------------------------------
    // arg parsing
    if (arg_consumer.has_arg("db", "")) {  // DATABASE
      if (arg_consumer.has_arg("history", "db")) {
        // print simulation history
        print_simulation_history(db, false);
      } else if (arg_consumer.has_arg("history-detail", "db")) {
        // print detailed simulation history
        print_simulation_history(db, true);
      } else if (arg_consumer.has_arg("clear", "db")) {
        // clear history
        if (db.delete_simulations()) {
          std::cout << "Simulation History Cleared. " << db.changes()
                    << " Simulation(s) Deleted.\n";
        }
      } else if (arg_consumer.has_arg("run", "db")) {
        // rerun a previous simulation by database id
        int sim_sqlite_id = arg_consumer.get_value<int>("run", "db");
        HistorySimulation sim;
        if (db.read_simulation(sim_sqlite_id, sim)) {
          // TODO - support storing sensitivity analysis
          std::cout << "Running simulation " << sim_sqlite_id << std::endl;
          cd_config.max_cluster_size = sim.max_cluster_size;
          cd_config.simulation_time = sim.simulation_time;

          // TODO - Support sample interval and set a max resolution to
          // avoid bloating the database. For this to work we will need a
          // list of ClusterDynamicState objects and a SQLite intersection
          // table.
          cd_config.time_delta = cd_config.sample_interval = sim.time_delta;

          cd_config.material = sim.material;
          cd_config.reactor = sim.reactor;

          run_simulation();
        } else {
          std::cerr << "Could not find simulation " << sim_sqlite_id
                    << std::endl;
        }
      }
    } else if (arg_consumer.has_arg("sensitivity-analysis",
                                    "")) {  // SENSITIVITY ANALYSIS
      std::string sa_var_name;
      // Set sensitivity analysis mode to true
      if (arg_consumer.has_arg("num-sims", "sensitivity-analysis") &&
          arg_consumer.has_arg("sensitivity-var", "sensitivity-analysis") &&
          arg_consumer.has_arg("sensitivity-var-delta",
                               "sensitivity-analysis")) {
        cd_config.sa_num_simulations =
            arg_consumer.get_value<int>("num-sims", "sensitivity-analysis");

        if (cd_config.sa_num_simulations <= 0)
          throw GpiesException(
              "Value for num-sims must be a positive, non-zero integer.");

        cd_config.sa_var = arg_consumer.get_sa_var();
        sa_var_name = arg_consumer.get_value<std::string>(
            "sensitivity-var", "sensitivity-analysis");

        cd_config.sa_var_delta = arg_consumer.get_value<gp_float>(
            "sensitivity-var-delta", "sensitivity-analysis");
      } else {
        throw GpiesException(
            "Missing required arguments for sensitivity "
            "analysis.\n--help to see required variables.");
      }

      std::cout << "\nSENSITIVITY ANALYSIS MODE\n"
                << "# of simulations: " << cd_config.sa_num_simulations
                << "  sensitivity variable: " << sa_var_name
                << "  sensitivity variable delta: " << cd_config.sa_var_delta
                << "\n\n";

      gp_float sa_var_value = get_sa_var_value();

      // --------------------------------------------------------------------------------------------
      // sensitivity analysis simulation loop
      for (size_t n = 0; n < cd_config.sa_num_simulations; n++) {
        ClusterDynamics cd = ClusterDynamics::cpu(cd_config);
        ClusterDynamicsState state;

        if (n > 0) os << "\n";  // visual divider for consecutive sims

        if (csv) {
          os << "simulation " << n + 1
             << ",sensitivity variable: " << sa_var_name
             << ",current value: " << sa_var_value << ",current delta: "
             << static_cast<gp_float>(n) * cd_config.sa_var_delta << "\n\n";
          os << "time (s),cluster size,"
                "interstitials / cm^3,vacancies / cm^3\n";
        } else {
          os << "simulation " << n + 1
             << "\tsensitivity variable: " << sa_var_name
             << "\tcurrent value: " << sa_var_value << "\tcurrent delta: "
             << static_cast<gp_float>(n) * cd_config.sa_var_delta << std::endl;
        }

        print_start_message();

        for (gp_float t = 0; t < cd_config.simulation_time; t = state.time) {
          // run simulation for this time slice
          state = cd.run(cd_config.time_delta, cd_config.sample_interval);

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

        sa_var_value = sa_update_config();
      }
      // --------------------------------------------------------------------
    } else {  // CLUSTER DYNAMICS OPTIONS
      ClusterDynamicsState state = run_simulation();

      // --------------------------------------------------------------------------------------------
      // Write simulation result to the database
      HistorySimulation history_simulation(
          cd_config.max_cluster_size, cd_config.simulation_time,
          cd_config.time_delta, cd_config.reactor, cd_config.material, state);

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
