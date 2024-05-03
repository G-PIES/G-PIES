#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#include <memory>
#include <string>

#include "cluster_dynamics_state.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/types.hpp"
#include "utils/gpies_exception.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"

class ClusterDynamicsImpl;

/** @brief Class which defines a cluster dynamics simulation.
 **/
class ClusterDynamics {
 private:
  std::unique_ptr<ClusterDynamicsImpl>
      _impl;  //< Pointer to a backend implementation class.

  Material material;       //< Current set material parameters.
  NuclearReactor reactor;  //< Current set reactor parameters.

 public:
  /** @brief Constructs and initializes a new cluster dynamics simulation.
   *  @param max_cluster_size The number of cluster sizes that the
   * simulation will track.
   *  @param reactor A NuclearReactor object containing the reactor parameters
   * the simulation will use.
   *  @param material A Material object containing the material parameters the
   * simulation will use.
   */
  explicit ClusterDynamics(ClusterDynamicsConfig &config);
  ~ClusterDynamics();

  /** @brief Runs the simulation and returns the end simulation state as a
   * ClusterDynamicsState object.
   *  @param time_delta The time step for the simulation in seconds.
   *  @param total_time The length of time that should be simulated in
   * seconds.
   *
   *  run() can be called multiple times, and the simulation will resume from
   * where it stopped after the last time run() was called.
   */
  ClusterDynamicsState run(gp_float time_delta, gp_float total_time);

  /** @brief Returns the Material parameters that the simulation currently has
   * set.
   */
  Material get_material() const;

  /** @brief Sets the Material parameters for the simulation to use.
   *  @param material A Material object containing the new material
   * parameters.
   *
   *  It is okay to change the material parameters even if the simulation
   *  has already been run() for some time.
   */
  void set_material(const Material &material);

  /** @brief Returns the NuclearReactor parameters that the simulation
   * currently has set.
   */
  NuclearReactor get_reactor() const;

  /** @brief Sets the NuclearReactor parameters for the simulation to use.
   *  @param reactor A NuclearReactor object containing the new reactor
   * parameters.
   *
   *  It is okay to change the reactor parameters even if the simulation
   *  has already been run() for some time.
   */
  void set_reactor(const NuclearReactor &reactor);

  void set_data_validation(const bool data_validation_on);
  void set_relative_tolerance(const gp_float relative_tolerance);
  void set_absolute_tolerance(const gp_float absolute_tolerance);
  void set_max_num_integration_steps(const size_t max_num_integration_steps);
  void set_min_integration_step(const gp_float min_integration_step);
  void set_max_integration_step(const gp_float max_integration_step);
};

class ClusterDynamicsException : public GpiesException {
 public:
  ClusterDynamicsException(const std::string &message,
                    const ClusterDynamicsState &err_state)
      : GpiesException(message), err_state(err_state) {}

  ClusterDynamicsState err_state;
};

#endif  // CLUSTER_DYNAMICS_HPP
