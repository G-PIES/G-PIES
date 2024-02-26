#ifndef CLUSTER_DYNAMICS_CUDA_IMPL_HPP
#define CLUSTER_DYNAMICS_CUDA_IMPL_HPP

#include <thrust/device_free.h>
#include <thrust/device_malloc.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/sequence.h>
#include <thrust/transform_reduce.h>

#include <cmath>
#include <vector>

#include "cluster_dynamics/cluster_dynamics_state.hpp"
#include "material_impl.hpp"
#include "nuclear_reactor_impl.hpp"
#include "utils/constants.hpp"

#define __CUDADECL__ __device__ __host__

class ClusterDynamicsImpl
{
public:
    gp_float time;

    thrust::device_vector<gp_float> interstitials;
    thrust::device_vector<gp_float> interstitials_temp;
    thrust::device_vector<gp_float> vacancies;
    thrust::device_vector<gp_float> vacancies_temp;

    size_t concentration_boundary;
    gp_float dislocation_density;

    gp_float mean_dislocation_radius_val;
    gp_float ii_sum_absorption_val;
    gp_float iv_sum_absorption_val;
    gp_float vv_sum_absorption_val;
    gp_float vi_sum_absorption_val;
    gp_float i_diffusion_val;
    gp_float v_diffusion_val;

    MaterialImpl material;
    NuclearReactorImpl reactor;

    thrust::device_vector<int> indices;
    thrust::device_ptr<ClusterDynamicsImpl> self;
    thrust::host_vector<double> host_interstitials;
    thrust::host_vector<double> host_vacancies;

    // Physics Model Functions

    __CUDADECL__ gp_float i_concentration_derivative(size_t) const;
    __CUDADECL__ gp_float v_concentration_derivative(size_t) const;
    gp_float i1_concentration_derivative() const;
    gp_float v1_concentration_derivative() const;
    gp_float dislocation_density_derivative() const;
    __CUDADECL__ gp_float i_defect_production(size_t) const;
    __CUDADECL__ gp_float v_defect_production(size_t) const;
    __CUDADECL__ gp_float i_demotion_rate(size_t) const;
    __CUDADECL__ gp_float v_demotion_rate(size_t) const;
    __CUDADECL__ gp_float i_combined_promotion_demotion_rate(size_t) const;
    __CUDADECL__ gp_float v_combined_promotion_demotion_rate(size_t) const;
    __CUDADECL__ gp_float i_promotion_rate(size_t) const;
    __CUDADECL__ gp_float v_promotion_rate(size_t) const;
    gp_float i_emission_rate() const;
    gp_float v_emission_rate() const;
    gp_float i_absorption_rate() const;
    gp_float v_absorption_rate() const;
    __CUDADECL__ gp_float annihilation_rate() const;
    __CUDADECL__ gp_float i_dislocation_annihilation_rate() const;
    __CUDADECL__ gp_float v_dislocation_annihilation_rate() const;
    __CUDADECL__ gp_float i_grain_boundary_annihilation_rate() const;
    __CUDADECL__ gp_float v_grain_boundary_annihilation_rate() const;
    __CUDADECL__ gp_float ii_emission(size_t) const;
    __CUDADECL__ gp_float vv_emission(size_t) const;
    __CUDADECL__ gp_float ii_absorption(size_t) const;
    __CUDADECL__ gp_float iv_absorption(size_t) const;
    __CUDADECL__ gp_float vi_absorption(size_t) const;
    __CUDADECL__ gp_float vv_absorption(size_t) const;
    __CUDADECL__ gp_float i_bias_factor(size_t) const;
    __CUDADECL__ gp_float v_bias_factor(size_t) const;
    __CUDADECL__ gp_float i_binding_energy(size_t) const;
    __CUDADECL__ gp_float v_binding_energy(size_t) const;
    __CUDADECL__ gp_float dislocation_promotion_probability(size_t) const;
    __CUDADECL__ gp_float cluster_radius(size_t) const;

    // Value Precalculation Functions
    gp_float i_diffusion() const;
    gp_float v_diffusion() const;
    gp_float ii_sum_absorption(size_t) const;
    gp_float iv_sum_absorption(size_t) const;
    gp_float vv_sum_absorption(size_t) const;
    gp_float vi_sum_absorption(size_t) const;
    gp_float mean_dislocation_cell_radius() const;

    // Simulation Operation Functions
    bool update_clusters_1(gp_float);
    bool update_clusters(gp_float);
    void step_init();
    bool step(gp_float);
    bool validate(size_t) const;

    // Interface functions
    ClusterDynamicsImpl(size_t concentration_boundary, 
        const NuclearReactorImpl& reactor, const MaterialImpl& material);
    ~ClusterDynamicsImpl();
      
    ClusterDynamicsState run(gp_float delta_time, gp_float total_time);
    MaterialImpl get_material() const;
    void set_material(const MaterialImpl& material);
    NuclearReactorImpl get_reactor() const;
    void set_reactor(const NuclearReactorImpl& reactor);
};

#endif  // CLUSTER_DYNAMICS_CUDA_IMPL_HPP
