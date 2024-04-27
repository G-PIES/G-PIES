#ifndef CLUSTER_DYNAMICS_CONFIG_HPP
#define CLUSTER_DYNAMICS_CONFIG_HPP

#include <cstring>
#include "utils/types.hpp"
#include "model/nuclear_reactor.hpp"
#include "model/material.hpp"

struct ClusterDynamicsConfig {
  bool data_validation_on = true;

  size_t max_cluster_size = 1001;

  gp_float relative_tolerance = 1e-6;
  gp_float absolute_tolerance = 1e1;
  size_t max_num_integration_steps = 5000;
  gp_float min_integration_step = 1e-30;
  gp_float max_integration_step = 1e20;

  NuclearReactor reactor;
  Material material;
};

#endif  // CLUSTER_DYNAMICS_CONFIG_HPP
