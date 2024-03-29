#include "cluster_dynamics/cluster_dynamics.hpp"

#if defined(USE_CUDA)
#include "cluster_dynamics_cuda_impl.hpp"
#elif defined(USE_METAL)
#include "cluster_dynamics_metal_impl.hpp"
#else
#include "cluster_dynamics_impl.hpp"
#endif

ClusterDynamics::ClusterDynamics(size_t max_cluster_size,
                                 const NuclearReactor &reactor,
                                 const Material &material) {
  this->reactor = reactor;
  this->material = material;
  _impl = std::make_unique<ClusterDynamicsImpl>(
      max_cluster_size, *reactor._impl, *material._impl);
}

/** We cannot use the default destructor that the header would've defined
 * because unique_ptr needs to know how to delete the type it contains:
 * \n
 * https://stackoverflow.com/questions/34072862/why-is-error-invalid-application-of-sizeof-to-an-incomplete-type-using-uniqu
 */
ClusterDynamics::~ClusterDynamics() {}

ClusterDynamicsState ClusterDynamics::run(gp_float time_delta,
                                          gp_float total_time) {
  return _impl->run(time_delta, total_time);
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
