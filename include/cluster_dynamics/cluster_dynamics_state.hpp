#ifndef CLUSTER_DYNAMICS_STATE_HPP
#define CLUSTER_DYNAMICS_STATE_HPP

#include <vector>

#include "utils/types.hpp"

/** @brief Class which contains information about the state of a ClusterDynamics
 * simulation.
 */
struct ClusterDynamicsState {
  /** Marks how much time in seconds has been simulated to create this state.
   */
  gp_float time = 0.0;

  /** The cumulative radiation dose in displacements per atom (dpa)
   */
  gp_float dpa = 0.0;

  /** @brief The current concentration of each size of interstitial cluster
   * in \todo UNITS
   *
   * The concentration of clusters of size `n` will be found at
   * interstitials[n]. The 0th element is always 0. The size of this data is
   * determined by the `max_cluster_size` that the ClusterDynamics
   * simulation was constructed with.
   */
  std::vector<gp_float> interstitials;

  /** @brief The current concentration of each size of vacancy cluster
   *  in \todo UNITS
   *
   * The concentration of clusters of size `n` will be found at
   * interstitials[n]. The 0th element is always 0. The size of this data is
   * determined by the `max_cluster_size` that the ClusterDynamics
   * simulation was constructed with.
   */
  std::vector<gp_float> vacancies;

  /** @brief The current density of the dislocation network in \todo UNITS
   */
  gp_float dislocation_density = 0.0;
};

#endif  // CLUSTER_DYNAMICS_STATE_HPP
