#ifndef CLUSTER_DYNAMICS_CONFIG_HPP
#define CLUSTER_DYNAMICS_CONFIG_HPP

#include <cstring>
#include <vector>

#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/sensitivity_variable.hpp"
#include "utils/types.hpp"

struct ClusterDynamicsConfig {
  gp_float simulation_time = 1e8;
  gp_float time_delta = 1e6;
  gp_float sample_interval =
      time_delta;  // How often (in seconds) to record the state
  bool data_validation_on = true;
  size_t max_cluster_size = 1001;

  // Integration Params
  gp_float relative_tolerance = 1e-6;
  gp_float absolute_tolerance = 1e1;
  size_t max_num_integration_steps = 5000;
  gp_float min_integration_step = 1e-30;
  gp_float max_integration_step = 1e20;

  NuclearReactor reactor;
  Material material;

  // Initial Defect Concentration State
  std::vector<gp_float> init_interstitials;
  std::vector<gp_float> init_vacancies;

  // Sensitivity Analysis Params
  size_t sa_num_simulations = 0;
  SensitivityVariable sa_var = SensitivityVariable::NONE;
  gp_float sa_var_delta = 0.;
};

#endif  // CLUSTER_DYNAMICS_CONFIG_HPP
