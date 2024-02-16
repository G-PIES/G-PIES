#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#include <memory>
#include "cluster_dynamics_state.hpp"
#include "material.hpp"
#include "nuclear_reactor.hpp"

class ClusterDynamicsImpl;

class ClusterDynamics
{
private:
  std::unique_ptr<ClusterDynamicsImpl> _impl;
public:
  ClusterDynamics(size_t concentration_boundary, size_t max_spatial_dim, NuclearReactor reactor, Material material);
  ~ClusterDynamics();

  ClusterDynamicsState run(double delta_time, double total_time, double delta_x);
  Material get_material();
  void set_material(Material material);
  NuclearReactor get_reactor();
  void set_reactor(NuclearReactor reactor);
};

#endif // CLUSTER_DYNAMICS_HPP
