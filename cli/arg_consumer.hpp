#ifndef ARG_CONSUMER
#define ARG_CONSUMER

#include <boost/program_options.hpp>
#include <exception>
#include <string>
#include <vector>

#include "client_db/client_db.hpp"
#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/timer.hpp"
#include "yaml_consumer.hpp"

namespace po = boost::program_options;

class ArgConsumer {
 public:
  ArgConsumer(int argc, char *argv[], const po::options_description &options) {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("config")) {
      std::string config_filename = vm["config"].as<std::string>();
      yaml_consumer.load_yaml(config_filename);
    }
  }

  bool has_arg(const std::string &arg,
               const std::string &config_category = "") {
    if (vm.count(arg)) return true;

    if (yaml_consumer.has_config_file()) {
      return yaml_consumer.has_arg(arg, config_category);
    }

    return false;
  }

  template <typename T>
  T get_value(const std::string &arg, const std::string &config_category = "") {
    if (vm.count(arg)) return vm[arg].as<T>();

    if (yaml_consumer.has_config_file()) {
      return yaml_consumer.get_value<T>(arg, config_category);
    }

    return vm[arg].as<T>();
  }

  SensitivityVariable get_sa_var() {
    std::string arg_name =
        get_value<std::string>("sensitivity-var", "sensitivity-analysis");

    if (sensitivity_variables.count(arg_name)) {
      return sensitivity_variables[arg_name];
    }

    return SensitivityVariable::NONE;
  }

  void populate_reactor(NuclearReactor &reactor) {
    yaml_consumer.populate_reactor(reactor);
  }
  void populate_material(Material &material) {
    yaml_consumer.populate_material(material);
  }

  void populate_init_interstitials(ClusterDynamicsConfig &cd_config) {
    yaml_consumer.populate_init_interstitials(cd_config);
  }

  void populate_init_vacancies(ClusterDynamicsConfig &cd_config) {
    yaml_consumer.populate_init_vacancies(cd_config);
  }

 private:
  po::variables_map vm;
  YamlConsumer yaml_consumer;
};

#endif  // ARG_CONSUMER
