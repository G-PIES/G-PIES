#ifndef YAML_CONSUMER
#define YAML_CONSUMER

#include <yaml-cpp/yaml.h>

#include <exception>
#include <string>
#include <vector>

#include "arg_consumer.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"

class YamlConsumer : public ArgConsumer {
 public:
  YamlConsumer() {}

  YamlConsumer(const std::string &yaml_filename) { load_yaml(yaml_filename); }

  void load_yaml(const std::string &yaml_filename) {
    config = YAML::LoadFile(yaml_filename);
  }

  void populate_cd_config(ClusterDynamicsConfig &cd_config) {
    ArgConsumer::populate_cd_config(cd_config);
  }

  bool has_config_file() { return !config.IsNull(); }

  bool has_arg(const std::string &arg,
               const std::string &config_category = "") {
    if (config_category.empty()) {
      return static_cast<bool>(config[arg]);
    }

    return static_cast<bool>(config[config_category][arg]);
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
    if (config_category.empty()) {
      return config[arg].as<T>();
    }

    return config[config_category][arg].as<T>();
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

 private:
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

#endif  // YAML_CONSUMER
