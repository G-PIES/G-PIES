#include <yaml-cpp/yaml.h>

#include <array>
#include <boost/program_options.hpp>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "arg_consumer.hpp"
#include "client_db/client_db.hpp"
#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/timer.hpp"
#include "utils/progress_bar.hpp"

namespace po = boost::program_options;

std::string filename;
std::ofstream output_file;
std::ostream os(std::cout.rdbuf());

bool csv = false;
bool step_print = false;

gp_float simulation_time = 1e8;
gp_float time_delta = 1e6;
gp_float sample_interval =
    time_delta;  // How often (in seconds) to record the state

ClusterDynamicsConfig config;

void print_reactor() {
  std::cout
      << config.reactor.species << "\nflux: " << config.reactor.get_flux()
      << " dpa/s"
      << "\ntemperature: " << config.reactor.get_temperature() << " kelvin"
      << "\nrecombination rate: " << config.reactor.get_recombination()
      << "\nbi-interstitial generation rate: " << config.reactor.get_i_bi()
      << "\ntri-interstitial generation rate: " << config.reactor.get_i_tri()
      << "\nquad-interstitial generation rate: " << config.reactor.get_i_quad()
      << "\nbi-vacancy generation rate: " << config.reactor.get_v_bi()
      << "\ntri-vacancy generation rate: " << config.reactor.get_v_tri()
      << "\nquad-vacancy generation rate: " << config.reactor.get_v_quad()
      << "\ndislocation density evolution: "
      << config.reactor.get_dislocation_density_evolution() << std::endl;
}

void print_material() {
  std::cout << config.material.species
            << "\ninterstitial migration: " << config.material.get_i_migration()
            << " eV"
            << "\nvacancy migration: " << config.material.get_v_migration()
            << " eV"
            << "\ninitial interstitial diffusion: "
            << config.material.get_i_diffusion_0() << " cm^2/s"
            << "\ninitial vacancy diffusion: "
            << config.material.get_v_diffusion_0() << " cm^2/s"
            << "\ninterstitial formation: " << config.material.get_i_formation()
            << " eV"
            << "\nvacancy formation: " << config.material.get_v_formation()
            << " eV"
            << "\ninterstitial binding: " << config.material.get_i_binding()
            << " eV"
            << "\nvacancy binding: " << config.material.get_v_binding() << " eV"
            << "\nrecombination radius: "
            << config.material.get_recombination_radius() << " cm"
            << "\ninterstitial loop bias: " << config.material.get_i_loop_bias()
            << "\ninterstitial dislocation bias: "
            << config.material.get_i_dislocation_bias()
            << "\ninterstitial dislocation bias param: "
            << config.material.get_i_dislocation_bias_param()
            << "\nvacancy loop bias: " << config.material.get_v_loop_bias()
            << "\nvacancy dislocation bias: "
            << config.material.get_v_dislocation_bias()
            << "\nvacancy dislocation bias param: "
            << config.material.get_v_dislocation_bias_param()
            << "\ninitial dislocation density: "
            << config.material.get_dislocation_density_0() << " cm^-2"
            << "\ngrain size: " << config.material.get_grain_size() << " cm"
            << "\nlattice parameter: " << config.material.get_lattice_param()
            << " cm"
            << "\nburgers vector (lattice_parameter / sqrt(2)): "
            << config.material.get_burgers_vector() << " cm"
            << "\natomic volume (lattice_parameter^3 / 4): "
            << config.material.get_atomic_volume() << "cm^3" << std::endl;
}

void print_start_message() {
  std::cout << "\nG-PIES simulation started\n"
            << "simulation time: " << simulation_time
            << "  time delta: " << time_delta
            << "  sample interval: " << sample_interval
            << "  max cluster size: "
            << static_cast<int>(config.max_cluster_size)
            << "  data validation: "
            << (config.data_validation_on ? "on" : "off") << std::endl
            << "Integration Settings\n"
            << "  relative tolerance: " << config.relative_tolerance
            << "  absolute tolerance: " << config.absolute_tolerance
            << "  max num integration steps: "
            << config.max_num_integration_steps
            << "  min integration step: " << config.min_integration_step
            << "  max integration step: " << config.max_integration_step
            << std::endl;

  std::cout << "\nReactor Settings\n";
  print_reactor();

  std::cout << "\nMaterial Settings\n";
  print_material();

  std::cout << std::endl;

  std::cout << "\nInitial Defect Clustering";
  std::cout << "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n";
  for (size_t n = 1; n < config.max_cluster_size; ++n) {
    if (config.init_interstitials[n] > 0. || config.init_vacancies[n] > 0.) {
      std::cout << (long long unsigned int)n << "\t\t\t\t\t"
                << config.init_interstitials[n] << "\t\t\t"
                << config.init_vacancies[n] << std::endl;
    }
  }
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
    os << "ID ~ Max Cluster Size ~ Simulation Time ~ Delta "
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

gp_float sa_update_config() {
  switch (config.sa_var) {
    case SensitivityVariable::interstitial_migration_ev:
      config.material.set_i_migration(config.material.get_i_migration() +
                                      config.sa_var_delta);
      return config.material.get_i_migration();
    case SensitivityVariable::vacancy_migration_ev:
      config.material.set_v_migration(config.material.get_v_migration() +
                                      config.sa_var_delta);
      return config.material.get_v_migration();
    case SensitivityVariable::interstitial_formation_ev:
      config.material.set_i_formation(config.material.get_i_formation() +
                                      config.sa_var_delta);
      return config.material.get_i_formation();
    case SensitivityVariable::vacancy_formation_ev:
      config.material.set_v_formation(config.material.get_v_formation() +
                                      config.sa_var_delta);
      return config.material.get_v_formation();
    case SensitivityVariable::interstitial_binding_ev:
      config.material.set_i_binding(config.material.get_i_binding() +
                                    config.sa_var_delta);
      return config.material.get_i_binding();
    case SensitivityVariable::vacancy_binding_ev:
      config.material.set_v_binding(config.material.get_v_binding() +
                                    config.sa_var_delta);
      return config.material.get_v_binding();
    case SensitivityVariable::initial_dislocation_density_cm:
      config.material.set_dislocation_density_0(
          config.material.get_dislocation_density_0() + config.sa_var_delta);
      return config.material.get_dislocation_density_0();
    case SensitivityVariable::flux_dpa_s:
      config.reactor.set_flux(config.reactor.get_flux() + config.sa_var_delta);
      return config.reactor.get_flux();
    case SensitivityVariable::temperature_kelvin:
      config.reactor.set_temperature(config.reactor.get_temperature() +
                                     config.sa_var_delta);
      return config.reactor.get_temperature();
    case SensitivityVariable::dislocation_density_evolution:
      config.reactor.set_dislocation_density_evolution(
          config.reactor.get_dislocation_density_evolution() +
          config.sa_var_delta);
      return config.reactor.get_dislocation_density_evolution();
    default:
      break;
  }

  return 0.;
}

gp_float get_sa_var_value() {
  switch (config.sa_var) {
    case SensitivityVariable::interstitial_migration_ev:
      return config.material.get_i_migration();
    case SensitivityVariable::vacancy_migration_ev:
      return config.material.get_v_migration();
    case SensitivityVariable::interstitial_formation_ev:
      return config.material.get_i_formation();
    case SensitivityVariable::vacancy_formation_ev:
      return config.material.get_v_formation();
    case SensitivityVariable::interstitial_binding_ev:
      return config.material.get_i_binding();
    case SensitivityVariable::vacancy_binding_ev:
      return config.material.get_v_binding();
    case SensitivityVariable::initial_dislocation_density_cm:
      return config.material.get_dislocation_density_0();
    case SensitivityVariable::flux_dpa_s:
      return config.reactor.get_flux();
    case SensitivityVariable::temperature_kelvin:
      return config.reactor.get_temperature();
    case SensitivityVariable::dislocation_density_evolution:
      return config.reactor.get_dislocation_density_evolution();
    default:
      break;
  }

  return 0.;
}

ClusterDynamicsState run_simulation() {
  ClusterDynamics cd(config);

  print_start_message();

  if (csv) {
    os << "Time (s),";
    for (size_t i = 1; i < config.max_cluster_size; ++i) {
      os << "i" << i << ",v" << i << ",";
    }
    os << "\n";
  }

  progressbar bar(simulation_time);
  bar.set_todo_char(" ");
  bar.set_done_char("█");
  bar.set_opening_bracket_char("[");
  bar.set_closing_bracket_char("]");

  // TODO - support sample interval
  sample_interval = time_delta;

  ClusterDynamicsState state;
  // --------------------------------------------------------------------------------------------
  // main simulation loop
  for (gp_float t = 0; t < simulation_time; t = state.time) {
    if (!step_print) {
      bar.update();
    } 

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

void emit_config_yaml(const std::string& filename) {
  YAML::Emitter out;
  YAML::Emitter sa_comment;
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
      << "initial-dislocation-density-cm" << YAML::Value << "10.0e+12"
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
      << YAML::Newline << YAML::BeginMap << YAML::Key << "init-interstitials"
      << YAML::Value << YAML::BeginMap << YAML::Key << "array" << YAML::Value
      << YAML::Flow
      << std::vector<std::string>{"1.0e-13", "6.0e-10", "4.0e-2", "2.0e-1"}
      << YAML::Comment("cluster concentrations for cluster sizes 1-4")
      << YAML::Key << "ranges" << YAML::Value << YAML::BeginSeq << YAML::Flow
      << std::vector<std::string>{"50", "55", "9.9e+30"}
      << YAML::Comment("cluster concentrations for cluster sizes 50-55")
      << YAML::Flow << std::vector<std::string>{"90", "102", "5.5e+29"}
      << YAML::Comment("cluster concentrations for cluster sizes 90-102")
      << YAML::Flow << std::vector<std::string>{"995", "1000", "2.2e+22"}
      << YAML::Comment("cluster concentrations for cluster sizes 995-1000")
      << YAML::EndSeq << YAML::EndMap << YAML::EndMap << YAML::Newline
      << YAML::Newline << YAML::BeginMap << YAML::Key << "init-vacancies"
      << YAML::Value << YAML::BeginMap << YAML::Key << "array" << YAML::Value
      << YAML::Flow
      << std::vector<std::string>{"1.0e-5", "2.3e-1", "4.2e-20", "1.3e-13"}
      << YAML::Comment("cluster concentrations for cluster sizes 1-4")
      << YAML::Key << "ranges" << YAML::Value << YAML::BeginSeq << YAML::Flow
      << std::vector<std::string>{"50", "55", "6.2e+29"}
      << YAML::Comment("cluster concentrations for cluster sizes 50-55")
      << YAML::Flow << std::vector<std::string>{"90", "102", "1.5e+13"}
      << YAML::Comment("cluster concentrations for cluster sizes 90-102")
      << YAML::Flow << std::vector<std::string>{"995", "1000", "4.3e+12"}
      << YAML::Comment("cluster concentrations for cluster sizes 995-1000")
      << YAML::EndSeq << YAML::EndMap << YAML::EndMap << YAML::Newline
      << YAML::Newline
      << YAML::Comment(
             "#################################################################"
             "####")
      << YAML::Newline << YAML::Newline
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
  // valid_integration_search();
  // return 0;

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
        po::value<size_t>()->implicit_value(config.max_cluster_size),
        "set the max size of defect clustering to model")(
        "time", po::value<gp_float>()->implicit_value(simulation_time),
        "the simulation environment time span to model (in seconds)")(
        "time-delta", po::value<gp_float>()->implicit_value(time_delta),
        "the time delta for every step of the simulation (in seconds)")(
        "sample-interval",
        po::value<gp_float>()->implicit_value(sample_interval),
        "how often to record simulation environment state (in seconds)")(
        "relative-tolerance",
        po::value<gp_float>()->implicit_value(config.relative_tolerance),
        "scalar relative tolerance for integration error")(
        "absolute-tolerance",
        po::value<gp_float>()->implicit_value(config.absolute_tolerance),
        "absolute relative tolerance for integration error")(
        "max-num-integration-steps",
        po::value<size_t>()->implicit_value(config.max_num_integration_steps),
        "maximum allowed number of integration steps to achieve a single "
        "estimation")(
        "min-integration-step",
        po::value<gp_float>()->implicit_value(config.min_integration_step),
        "minimum step size for integration")(
        "max-integration-step",
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

    ArgConsumer arg_consumer(argc, argv, all_options);

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
          << "./cd_cli --sensitivity-analysis --num-sims 10 "
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
      if (output_file.is_open()) os.rdbuf(output_file.rdbuf());
    }

    if (arg_consumer.has_arg("max-cluster-size")) {
      size_t mcs = arg_consumer.get_value<size_t>("max-cluster-size");
      if (mcs <= 0)
        throw GpiesException(
            "Value for max-cluster-size must be a positive, non-zero integer.");

      config.max_cluster_size = mcs;
    }

    if (arg_consumer.has_arg("time")) {
      gp_float st = arg_consumer.get_value<gp_float>("time");
      if (st <= 0.)
        throw GpiesException(
            "Value for time must be a positive, non-zero decimal.");

      simulation_time = st;
    }

    if (arg_consumer.has_arg("time-delta")) {
      gp_float td = arg_consumer.get_value<gp_float>("time-delta");
      if (td <= 0.)
        throw GpiesException(
            "Value for time-delta must be a positive, non-zero "
            "decimal.");

      time_delta = td;
    }

    if (arg_consumer.has_arg("sample-interval")) {
      gp_float si = arg_consumer.get_value<gp_float>("sample-interval");
      if (si <= 0.)
        throw GpiesException(
            "Value for sample-interval must be a positive, non-zero "
            "decimal.");

      sample_interval = si;
    }

    if (arg_consumer.has_arg("relative-tolerance")) {
      gp_float rt = arg_consumer.get_value<gp_float>("relative-tolerance");
      if (rt <= 0.)
        throw GpiesException(
            "Value for relative-tolerance must be a positive, non-zero "
            "decimal.");

      config.relative_tolerance = rt;
    }

    if (arg_consumer.has_arg("absolute-tolerance")) {
      gp_float at = arg_consumer.get_value<gp_float>("absolute-tolerance");
      if (at <= 0.)
        throw GpiesException(
            "Value for absolute-tolerance must be a positive, non-zero "
            "decimal.");

      config.absolute_tolerance = at;
    }

    if (arg_consumer.has_arg("max-num-integration-steps")) {
      size_t mnis = arg_consumer.get_value<size_t>("max-num-integration-steps");
      if (mnis <= 0)
        throw GpiesException(
            "Value for max-num-integration-steps must be a positive, non-zero "
            "integer.");

      config.max_num_integration_steps = mnis;
    }

    if (arg_consumer.has_arg("min-integration-step")) {
      gp_float minis = arg_consumer.get_value<gp_float>("min-integration-step");
      if (minis <= 0.)
        throw GpiesException(
            "Value for min-integration-step must be a positive, non-zero "
            "decimal.");

      config.min_integration_step = minis;
    }

    if (arg_consumer.has_arg("max-integration-step")) {
      gp_float maxis = arg_consumer.get_value<gp_float>("max-integration-step");
      if (maxis <= 0.)
        throw GpiesException(
            "Value for max-integration-step must be a positive, non-zero "
            "decimal.");

      config.max_integration_step = maxis;
    }

    // Output formatting
    csv = static_cast<bool>(arg_consumer.has_arg("csv"));
    step_print = static_cast<bool>(arg_consumer.has_arg("step-print"));

    // Toggle data validation
    if (arg_consumer.has_arg("data-validation")) {
      config.data_validation_on =
          0 ==
          arg_consumer.get_value<std::string>("data-validation").compare("on");
    }

    if (arg_consumer.has_arg("reactor")) {
      arg_consumer.populate_reactor(config.reactor);
    } else {
      nuclear_reactors::OSIRIS(config.reactor);
    }

    if (arg_consumer.has_arg("material")) {
      arg_consumer.populate_material(config.material);
    } else {
      materials::SA304(config.material);
    }

    if (arg_consumer.has_arg("init-interstitials", "")) {
      arg_consumer.populate_init_interstitials(config);
    } else {
      config.init_interstitials =
          std::vector<gp_float>(config.max_cluster_size, 0.);
    }

    if (arg_consumer.has_arg("init-vacancies", "")) {
      arg_consumer.populate_init_vacancies(config);
    } else {
      config.init_vacancies =
          std::vector<gp_float>(config.max_cluster_size, 0.);
    }

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
    } else if (arg_consumer.has_arg("sensitivity-analysis",
                                    "")) {  // SENSITIVITY ANALYSIS
      std::string sa_var_name;
      // Set sensitivity analysis mode to true
      if (arg_consumer.has_arg("num-sims", "sensitivity-analysis") &&
          arg_consumer.has_arg("sensitivity-var", "sensitivity-analysis") &&
          arg_consumer.has_arg("sensitivity-var-delta",
                               "sensitivity-analysis")) {
        config.sa_on = true;
        config.sa_num_simulations =
            arg_consumer.get_value<int>("num-sims", "sensitivity-analysis");

        if (config.sa_num_simulations <= 0)
          throw GpiesException(
              "Value for num-sims must be a positive, non-zero integer.");

        config.sa_var = arg_consumer.get_sa_var();
        sa_var_name = arg_consumer.get_value<std::string>(
            "sensitivity-var", "sensitivity-analysis");

        config.sa_var_delta = arg_consumer.get_value<gp_float>(
            "sensitivity-var-delta", "sensitivity-analysis");
      } else {
        throw GpiesException(
            "Missing required arguments for sensitivity "
            "analysis.\n--help to see required variables.");
      }

      // TODO - support sample interval
      sample_interval = time_delta;

      std::cout << "\nSENSITIVITY ANALYSIS MODE\n"
                << "# of simulations: " << config.sa_num_simulations
                << "  sensitivity variable: " << sa_var_name
                << "  sensitivity variable delta: " << config.sa_var_delta
                << "\n\n";

      gp_float sa_var_value = get_sa_var_value();

      // --------------------------------------------------------------------------------------------
      // sensitivity analysis simulation loop
      for (size_t n = 0; n < config.sa_num_simulations; n++) {
        ClusterDynamics cd(config);
        ClusterDynamicsState state;

        if (n > 0) os << "\n";  // visual divider for consecutive sims

        if (csv) {
          os << "simulation " << n + 1
             << ",sensitivity variable: " << sa_var_name
             << ",current value: " << sa_var_value << ",current delta: "
             << static_cast<gp_float>(n) * config.sa_var_delta << "\n\n";
          os << "time (s),cluster size,"
                "interstitials / cm^3,vacancies / cm^3\n";
        } else {
          os << "simulation " << n + 1
             << "\tsensitivity variable: " << sa_var_name
             << "\tcurrent value: " << sa_var_value << "\tcurrent delta: "
             << static_cast<gp_float>(n) * config.sa_var_delta << std::endl;
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

        sa_var_value = sa_update_config();
      }
      // --------------------------------------------------------------------
    } else {  // CLUSTER DYNAMICS OPTIONS
      ClusterDynamicsState state = run_simulation();

      // --------------------------------------------------------------------------------------------
      // Write simulation result to the database
      HistorySimulation history_simulation(
          config.max_cluster_size, simulation_time, time_delta, config.reactor,
          config.material, state);

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
