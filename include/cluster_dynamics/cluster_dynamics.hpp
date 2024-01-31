#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#include <memory>
#include "cluster_dynamics_state.hpp"
#include "material.hpp"
#include "nuclear_reactor.hpp"

class ClusterDynamicsImpl;

/// Class which defines a cluster dynamics simulation.
/** 
 * 
 * */
class ClusterDynamics
{
private:
  std::unique_ptr<ClusterDynamicsImpl> _impl; //< Pointer to a backend implementation class.
public:
  ClusterDynamics(size_t concentration_boundary, NuclearReactor reactor, Material material);
  ~ClusterDynamics();
    
  ClusterDynamicsState run(double delta_time, double total_time);
  Material get_material() const;
  void set_material(Material material);
  NuclearReactor get_reactor() const;
  void set_reactor(NuclearReactor reactor);
};

#endif // CLUSTER_DYNAMICS_HPP