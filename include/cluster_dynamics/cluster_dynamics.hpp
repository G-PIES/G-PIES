#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#include <memory>

#include "cluster_dynamics_state.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/types.hpp"

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
     *  @param concentration_boundary The number of cluster sizes that the
     * simulation will track.
     *  @param reactor A NuclearReactor object containing the reactor parameters
     * the simulation will use.
     *  @param material A Material object containing the material parameters the
     * simulation will use.
     */
    ClusterDynamics(size_t concentration_boundary,
                    const NuclearReactor &reactor, const Material &material);
    ~ClusterDynamics();

    /** @brief Runs the simulation and returns the end simulation state as a
     * ClusterDynamicsState object.
     *  @param delta_time The time step for the simulation in seconds.
     *  @param total_time The length of time that should be simulated in
     * seconds.
     *
     *  run() can be called multiple times, and the simulation will resume from
     * where it stopped after the last time run() was called.
     */
    ClusterDynamicsState run(gp_float delta_time, gp_float total_time);

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
};

#endif  // CLUSTER_DYNAMICS_HPP
