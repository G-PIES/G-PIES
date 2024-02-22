#include <metal_stdlib>

#include "constants.hpp"
#include "cluster_dynamics_metal_kernel.cpp"

kernel void update_clusters(
    device ClusterDynamicsMetalKernel& mtl_kernel,
    device gp_float* interstitials_in,
    device gp_float* vacancies_in,
    device gp_float* interstitials_out,
    device gp_float* vacancies_out,
    constant gp_float& delta_time,
    uint index [[thread_position_in_grid]]
    )
{
    mtl_kernel.interstitials = interstitials_in;
    mtl_kernel.vacancies = vacancies_in;

    interstitials_out[index] += mtl_kernel.i_concentration_derivative(index) * delta_time;
    vacancies_out[index] += mtl_kernel.v_concentration_derivative(index) * delta_time;
}