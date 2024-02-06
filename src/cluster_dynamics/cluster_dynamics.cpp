#include "cluster_dynamics.hpp"

#if defined(USE_CUDA)
    #include "cluster_dynamics_cuda_impl.hpp"
#elif defined(USE_METAL)
    #include "cluster_dynamics_metal_impl.hpp"
#else
    #include "cluster_dynamics_impl.hpp"
#endif

ClusterDynamics::ClusterDynamics(size_t concentration_boundary, const NuclearReactor& reactor, const Material& material)
{
  _impl = std::make_unique<ClusterDynamicsImpl>(concentration_boundary, reactor, material);
}

/** We cannot use the default destructor that the header would've defined
 * because unique_ptr needs to know how to delete the type it contains:
 * \n https://stackoverflow.com/questions/34072862/why-is-error-invalid-application-of-sizeof-to-an-incomplete-type-using-uniqu
*/
ClusterDynamics::~ClusterDynamics()
{
}

ClusterDynamicsState ClusterDynamics::run(gp_float delta_time, gp_float total_time)
{
  return _impl->run(delta_time, total_time);
}

Material ClusterDynamics::get_material() const
{
  return _impl->get_material();
}

void ClusterDynamics::set_material(const Material& material)
{
  _impl->set_material(material);
}

NuclearReactor ClusterDynamics::get_reactor() const
{
  return _impl->get_reactor();
}

void ClusterDynamics::set_reactor(const NuclearReactor& reactor)
{
  _impl->set_reactor(reactor);
}
