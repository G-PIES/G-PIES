#ifndef CLUSTER_DYNAMICS_IMPL_HPP
#define CLUSTER_DYNAMICS_IMPL_HPP

#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "cluster_dynamics/cluster_dynamics_state.hpp"
#include "material_impl.hpp"
#include "nuclear_reactor_impl.hpp"
#include "utils/constants.hpp"

class ClusterDynamicsImpl {
 public:
  bool data_validation_on;
  gp_float relative_tolerance;
  gp_float absolute_tolerance;
  size_t max_num_integration_steps;
  gp_float min_integration_step;
  gp_float max_integration_step;

  virtual ClusterDynamicsState run(gp_float total_time) = 0;
  virtual MaterialImpl get_material() const = 0;
  virtual void set_material(const MaterialImpl& material) = 0;
  virtual NuclearReactorImpl get_reactor() const = 0;
  virtual void set_reactor(const NuclearReactorImpl& reactor) = 0;

  virtual ~ClusterDynamicsImpl() = default;
};

#endif  // CLUSTER_DYNAMICS_IMPL_HPP
