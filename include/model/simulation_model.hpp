#ifndef SIMULATION_MODEL_HPP
#define SIMULATION_MODEL_HPP

#include <string>

#include "cluster_dynamics/cluster_dynamics_state.hpp"
#include "model/nuclear_reactor.hpp"
#include "model/material.hpp"
#include "utils/datetime.hpp"
#include "utils/types.hpp"

struct SimulationModel {
  SimulationModel() : sqlite_id(-1) { datetime::utc_now(creation_datetime); }
  int sqlite_id;
  std::string creation_datetime;
  NuclearReactor reactor;
  Material material;
  ClusterDynamicsState cd_state;
};

#endif  // SIMULATION_MODEL_HPP
