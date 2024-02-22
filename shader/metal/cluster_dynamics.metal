#include <metal_stdlib>

#include "constants.hpp"
#include "cluster_dynamics_metal_kernel.cpp"

kernel void update_clusters(
    device ClusterDynamicsMetalKernel& args,
    device NuclearReactorImpl& reactor,
    device MaterialImpl& material,
    device gp_float* interstitials_in,
    device gp_float* vacancies_in,
    device gp_float* interstitials_out,
    device gp_float* vacancies_out,
    uint index [[thread_position_in_grid]]
    )
{
    if (index > 1)
    {
        args.reactor = &reactor;
        args.material = &material;
        args.interstitials = interstitials_in;
        args.vacancies = vacancies_in;

        interstitials_out[index] += args.i_concentration_derivative(index) * args.delta_time;
        vacancies_out[index] += args.v_concentration_derivative(index) * args.delta_time;
    }
}