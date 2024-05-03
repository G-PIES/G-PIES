#include <yaml-cpp/yaml.h>

#include <boost/program_options.hpp>
#include <string>

#include "client_db/client_db.hpp"
#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/timer.hpp"

namespace po = boost::program_options;

class ArgConsumer {
 public:
  ArgConsumer(int argc, char *argv[], const po::options_description &options) {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("config")) {
      std::string config_filename = vm["config"].as<std::string>();
      config = YAML::LoadFile(config_filename);
      has_config_file = !config.IsNull();
    }
  }

  bool has_arg(const std::string &arg,
               const std::string &config_category = "simulation") {
    if (has_config_file) {
      if (config_category.empty()) {
        return static_cast<bool>(config[arg]);
      }

      return static_cast<bool>(config[config_category][arg]);
    }

    return vm.count(arg);
  }

  template <typename T>
  T get_value(const std::string &arg,
              const std::string &config_category = "simulation") {
    if (has_config_file) {
      return config[config_category][arg].as<T>();
    }

    return vm[arg].as<T>();
  }

  void populate_reactor(NuclearReactor &reactor) {
    reactor.set_flux(config["reactor"]["flux-dpa-s"].as<gp_float>());
    reactor.set_temperature(
        config["reactor"]["temperature-kelvin"].as<gp_float>());
    reactor.set_recombination(
        config["reactor"]["recombination-rate"].as<gp_float>());
    reactor.set_i_bi(config["reactor"]["bi-interstitial-rate"].as<gp_float>());
    reactor.set_i_tri(
        config["reactor"]["tri-interstitial-rate"].as<gp_float>());
    reactor.set_i_quad(
        config["reactor"]["quad-interstitial-rate"].as<gp_float>());
    reactor.set_v_bi(config["reactor"]["bi-vacancy-rate"].as<gp_float>());
    reactor.set_v_tri(config["reactor"]["tri-vacancy-rate"].as<gp_float>());
    reactor.set_v_quad(config["reactor"]["quad-vacancy-rate"].as<gp_float>());
    reactor.set_dislocation_density_evolution(
        config["reactor"]["dislocation-density-evolution"].as<gp_float>());
  }

  void populate_material(Material &material) {
    material.set_i_migration(
        config["material"]["interstitial-migration-ev"].as<gp_float>());
    material.set_v_migration(
        config["material"]["vacancy-migration-ev"].as<gp_float>());
    material.set_i_diffusion_0(
        config["material"]["initial-interstitial-diffusion"].as<gp_float>());
    material.set_v_diffusion_0(
        config["material"]["initial-vacancy-diffusion"].as<gp_float>());
    material.set_i_formation(
        config["material"]["interstitial-formation-ev"].as<gp_float>());
    material.set_v_formation(
        config["material"]["vacancy-formation-ev"].as<gp_float>());
    material.set_i_binding(
        config["material"]["interstitial-binding-ev"].as<gp_float>());
    material.set_v_binding(
        config["material"]["vacancy-binding-ev"].as<gp_float>());
    material.set_recombination_radius(
        config["material"]["recombination-radius-cm"].as<gp_float>());
    material.set_i_loop_bias(
        config["material"]["interstitial-loop-bias"].as<gp_float>());
    material.set_i_dislocation_bias(
        config["material"]["interstitial-dislocation-bias"].as<gp_float>());
    material.set_i_dislocation_bias_param(
        config["material"]["interstitial-dislocation-bias-param"]
            .as<gp_float>());
    material.set_v_loop_bias(
        config["material"]["vacancy-loop-bias"].as<gp_float>());
    material.set_v_dislocation_bias(
        config["material"]["vacancy-dislocation-bias"].as<gp_float>());
    material.set_v_dislocation_bias_param(
        config["material"]["vacancy-dislocation-bias-param"].as<gp_float>());
    material.set_dislocation_density_0(
        config["material"]["initial-dislocation-density-cm^-2"].as<gp_float>());
    material.set_grain_size(config["material"]["grain-size-cm"].as<gp_float>());
    material.set_lattice_param(
        config["material"]["lattice-param-cm"].as<gp_float>());
  }

  SensitivityVariable get_sa_var() {
    std::string arg_name =
        get_value<std::string>("sensitivity-var", "sensitivity-analysis");

    if (sensitivity_variables.count(arg_name)) {
      return sensitivity_variables[arg_name];
    }

    return SensitivityVariable::NONE;
  }

 private:
  po::variables_map vm;
  YAML::Node config;
  bool has_config_file;
};
