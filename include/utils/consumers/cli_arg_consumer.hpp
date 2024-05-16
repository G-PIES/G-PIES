#ifndef CLI_ARG_CONSUMER
#define CLI_ARG_CONSUMER

#include <boost/program_options.hpp>
#include <exception>
#include <string>
#include <vector>

#include "arg_consumer.hpp"
#include "client_db/client_db.hpp"
#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/timer.hpp"
#include "yaml_consumer.hpp"

namespace po = boost::program_options;

class CliArgConsumer : public ArgConsumer {
 public:
  CliArgConsumer(int argc, char *argv[],
                 const po::options_description &options) {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("config")) {
      std::string config_filename = vm["config"].as<std::string>();
      yaml_consumer.load_yaml(config_filename);
    }
  }

  void populate_cd_config(ClusterDynamicsConfig &cd_config) {
    ArgConsumer::populate_cd_config(cd_config);
  }

  bool has_arg(const std::string &arg,
               const std::string &config_category = "") {
    if (vm.count(arg)) return true;

    if (yaml_consumer.has_config_file()) {
      return yaml_consumer.has_arg(arg, config_category);
    }

    return false;
  }

  std::string get_string(const std::string &arg,
                         const std::string &config_category = "") {
    if (!has_arg(arg, config_category)) return "";
    return get_value<std::string>(arg, config_category);
  }

  size_t get_size_t(const std::string &arg,
                    const std::string &config_category = "") {
    if (!has_arg(arg, config_category)) return 0;
    return get_value<size_t>(arg, config_category);
  }

  gp_float get_float(const std::string &arg,
                     const std::string &config_category = "") {
    if (!has_arg(arg, config_category)) return 0.;
    return get_value<gp_float>(arg, config_category);
  }

  template <typename T>
  T get_value(const std::string &arg, const std::string &config_category = "") {
    if (vm.count(arg)) return vm[arg].as<T>();

    if (yaml_consumer.has_config_file()) {
      return yaml_consumer.get_value<T>(arg, config_category);
    }

    return vm[arg].as<T>();
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

#endif  // CLI_ARG_CONSUMER
