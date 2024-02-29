#ifndef SIMULATION_MODEL_HPP
#define SIMULATION_MODEL_HPP

#include <string>

#include "cluster_dynamics/cluster_dynamics_state.hpp"
#include "utils/datetime.hpp"
#include "utils/types.hpp"

struct SimulationModel {
  SimulationModel() : sqlite_id(-1) { datetime::utc_now(creation_datetime); }
  int sqlite_id;
  std::string creation_datetime;
  int id_reactor = 0;
  int id_material = 0;
  ClusterDynamicsState cd_state;
};

#endif  // SIMULATION_MODEL_HPP
