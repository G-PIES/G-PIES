#include "cluster_dynamics.hpp"
#include "cluster_dynamics_impl.hpp"

ClusterDynamics::ClusterDynamics(size_t concentration_boundary, NuclearReactor reactor, Material material)
{
  _impl = std::make_unique<ClusterDynamicsImpl>(concentration_boundary, reactor, material);
}

/**
 * We cannot use the default destructor that the header would've defined
 * because unique_ptr needs to know how to delete the type it contains:
 * \n https://stackoverflow.com/questions/34072862/why-is-error-invalid-application-of-sizeof-to-an-incomplete-type-using-uniqu
*/
ClusterDynamics::~ClusterDynamics()
{
}

ClusterDynamicsState ClusterDynamics::run(double delta_time, double total_time)
{
  return _impl->run(delta_time, total_time);
}

Material ClusterDynamics::get_material() const
{
  return _impl->get_material();
}

void ClusterDynamics::set_material(Material material)
{
  _impl->set_material(material);
}

NuclearReactor ClusterDynamics::get_reactor() const
{
  return _impl->get_reactor();
}

void ClusterDynamics::set_reactor(NuclearReactor reactor)
{
  _impl->set_reactor(reactor);
}
