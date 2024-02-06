#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#include <memory>
#include "types.hpp"
#include "cluster_dynamics_state.hpp"
#include "material.hpp"
#include "nuclear_reactor.hpp"

class ClusterDynamicsImpl;

class ClusterDynamics
{
private:
  std::unique_ptr<ClusterDynamicsImpl> _impl;
public:
  ClusterDynamics(size_t concentration_boundary, const NuclearReactor& reactor, const Material& material);
  ~ClusterDynamics();
    
  ClusterDynamicsState run(gp_float delta_time, gp_float total_time);
  Material get_material();
  void set_material(const Material& material);
  NuclearReactor get_reactor();
  void set_reactor(const NuclearReactor& reactor);
};

#endif // CLUSTER_DYNAMICS_HPP