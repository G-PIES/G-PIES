#include "cluster_dynamics/cluster_dynamics.hpp"

#include "cluster_dynamics/cluster_dynamics_config.hpp"

#if defined(USE_CUDA)
#include "cluster_dynamics_cuda_impl.hpp"
#elif defined(USE_METAL)
#include "cluster_dynamics_metal_impl.hpp"
#else
#include "cluster_dynamics_cpu_impl.hpp"
#endif

ClusterDynamics ClusterDynamics::cpu(ClusterDynamicsConfig &config) {
  #if defined(USE_CUDA)
  auto impl = std::make_unique<ClusterDynamicsCudaImpl>(config);
  #else
  auto impl = std::make_unique<ClusterDynamicsCpuImpl>(config);
  #endif
  return ClusterDynamics(config, std::move(impl));
}

ClusterDynamics::ClusterDynamics(ClusterDynamicsConfig &config,
                                 std::unique_ptr<ClusterDynamicsImpl> impl)
    : _impl(std::move(impl)),
      material(config.material),
      reactor(config.reactor) {}

/** We cannot use the default destructor that the header would've defined
 * because unique_ptr needs to know how to delete the type it contains:
 * \n
 * https://stackoverflow.com/questions/34072862/why-is-error-invalid-application-of-sizeof-to-an-incomplete-type-using-uniqu
 */
ClusterDynamics::~ClusterDynamics() {}

ClusterDynamicsState ClusterDynamics::run([[maybe_unused]] gp_float time_delta,
                                          gp_float total_time) {
  return _impl->run(total_time);
}

Material ClusterDynamics::get_material() const { return material; }

void ClusterDynamics::set_material(const Material &material) {
  this->material = material;
  _impl->set_material(*material._impl);
}

NuclearReactor ClusterDynamics::get_reactor() const { return reactor; }

void ClusterDynamics::set_reactor(const NuclearReactor &reactor) {
  this->reactor = reactor;
  _impl->set_reactor(*reactor._impl);
}

void ClusterDynamics::set_data_validation(const bool data_validation_on) {
  _impl->data_validation_on = data_validation_on;
}

void ClusterDynamics::set_relative_tolerance(
    const gp_float relative_tolerance) {
  _impl->relative_tolerance = relative_tolerance;
}

void ClusterDynamics::set_absolute_tolerance(
    const gp_float absolute_tolerance) {
  _impl->absolute_tolerance = absolute_tolerance;
}

void ClusterDynamics::set_max_num_integration_steps(
    const size_t max_num_integration_steps) {
  _impl->max_num_integration_steps = max_num_integration_steps;
}

void ClusterDynamics::set_min_integration_step(
    const gp_float min_integration_step) {
  _impl->min_integration_step = min_integration_step;
}

void ClusterDynamics::set_max_integration_step(
    const gp_float max_integration_step) {
  _impl->max_integration_step = max_integration_step;
}
