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

    __METALDECL__ gp_float* interstitials;
    __METALDECL__ gp_float* vacancies;

    uint64_t concentration_boundary;
    gp_float dislocation_density;
    gp_float delta_time;

    gp_float mean_dislocation_radius_val;
    gp_float ii_sum_absorption_val;
    gp_float iv_sum_absorption_val;
    gp_float vv_sum_absorption_val;
    gp_float vi_sum_absorption_val;
    gp_float i1_val;
    gp_float v1_val;
};

#endif // CLUSTER_DYNAMICS_METAL_IMPL_ARGS_HPP