#ifndef CLUSTER_DYNAMICS_METAL_IMPL_ARGS_HPP
#define CLUSTER_DYNAMICS_METAL_IMPL_ARGS_HPP

#include "material_impl.hpp"
#include "nuclear_reactor_impl.hpp"

#if defined(__METAL__)
    #define __METALDECL__ device 
#else
    #define __METALDECL__
#endif

struct ClusterDynamicsMetalArgs
{
    __METALDECL__ MaterialImpl* material;
    __METALDECL__ NuclearReactorImpl* reactor;

    __METALDECL__ float* interstitials;
    __METALDECL__ float* vacancies;

    uint64_t concentration_boundary;
    float dislocation_density;
    float delta_time;

    float mean_dislocation_radius_val;
    float ii_sum_absorption_val;
    float iv_sum_absorption_val;
    float vv_sum_absorption_val;
    float vi_sum_absorption_val;
    float i1_val;
    float v1_val;
};

#endif // CLUSTER_DYNAMICS_METAL_IMPL_ARGS_HPP