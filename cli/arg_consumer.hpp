#include <yaml-cpp/yaml.h>

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

namespace po = boost::program_options;

class ArgConsumer {
 public:
  ArgConsumer(int argc, char *argv[], const po::options_description &options) {
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("config")) {
      std::string config_filename = vm["config"].as<std::string>();
      config = YAML::LoadFile(config_filename);
    }
  }

  bool has_config_file() { return !config.IsNull(); }

  bool has_arg(const std::string &arg,
               const std::string &config_category = "") {
    if (vm.count(arg)) return true;

    if (has_config_file()) {
      if (config_category.empty()) {
        return static_cast<bool>(config[arg]);
      }

      return static_cast<bool>(config[config_category][arg]);
    }

    return false;
  }

  template <typename T>
  T get_value(const std::string &arg, const std::string &config_category = "") {
    if (vm.count(arg)) return vm[arg].as<T>();

    if (has_config_file()) {
      if (config_category.empty()) {
        return config[arg].as<T>();
      }

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

  void populate_init_interstitials(ClusterDynamicsConfig &cd_config) {
    cd_config.init_interstitials =
        std::vector<gp_float>(cd_config.max_cluster_size, 0.);

    populate_vector_from_config(cd_config.init_interstitials,
                                config["init-interstitials"]);
  }

  void populate_init_vacancies(ClusterDynamicsConfig &cd_config) {
    cd_config.init_vacancies =
        std::vector<gp_float>(cd_config.max_cluster_size, 0.);

    populate_vector_from_config(cd_config.init_vacancies,
                                config["init-vacancies"]);
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

  void populate_vector_from_config(std::vector<gp_float> &vec,
                                   const YAML::Node &vec_config) {
    if (has_config_file() && vec_config.IsMap()) {
      YAML::Node array_node = vec_config["array"];
      YAML::Node ranges_node = vec_config["ranges"];

      // array mapping
      if (array_node.IsSequence()) {
        for (size_t i = 0; i < array_node.size() && i < vec.size() - 1; ++i) {
          // index 0 from the config represents size 1 cluster concentration
          gp_float value = array_node[i].as<gp_float>();

          if (value < 0.) {
            throw std::runtime_error(
                "defect array values must be greater than or equal to 0.0");
          }

          vec[i + 1] = value;
        }
      }

      // range mapping
      if (ranges_node.IsSequence()) {
        for (size_t i = 0; i < ranges_node.size(); ++i) {
          if (ranges_node[i].size() == 3) {
            size_t start_index = ranges_node[i][0].as<size_t>();
            size_t end_index = ranges_node[i][1].as<size_t>();
            gp_float value = ranges_node[i][2].as<gp_float>();

            if (start_index > end_index) start_index = end_index;

            if (end_index > vec.size() - 1) end_index = vec.size() - 1;

            if (value < 0.) {
              throw std::runtime_error(
                  "defect array values must be greater than or equal to 0.0");
            }

            for (size_t i = start_index; i <= end_index; ++i) {
              vec[i] = value;
            }
          } else {
            throw std::runtime_error(
                "ranges must be have 3 elements: [start, end, value]");
          }
        }
      }
    } else {
      throw std::runtime_error(
          "defect array value config is improperly formatted");
    }
  }
};
