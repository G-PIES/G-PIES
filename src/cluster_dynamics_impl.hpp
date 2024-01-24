#pragma once

#include <vector>
#include <cstdint>

#include "cluster_dynamics_state.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"

#include <thrust/host_vector.h>
#include <thrust/transform_reduce.h>
#include <thrust/sequence.h>

#ifdef USE_CUDA
  #include <thrust/device_vector.h>
  #include <thrust/device_malloc.h>
#endif

// Used to transform CUDA code into raw C++ for the software version
#ifdef USE_CUDA
    #define __CUDADECL__ __device__ __host__

    template<typename T>
    using thrust_vector = thrust_vector<T>;
#else
    #define __CUDADECL__

    template<typename T>
    using thrust_vector = thrust::host_vector<T>;
#endif

class ClusterDynamicsImpl
{
public:
  double time;

  thrust_vector<double> interstitials;
  thrust_vector<double> interstitials_temp;
  thrust_vector<double> vacancies;
  thrust_vector<double> vacancies_temp;

  size_t concentration_boundary;
  double dislocation_density;

  double mean_dislocation_radius_val;
  double ii_sum_absorption_val;
  double iv_sum_absorption_val;
  double vv_sum_absorption_val;
  double vi_sum_absorption_val;

  Material material;
  NuclearReactor reactor;

  #ifdef USE_CUDA
    thrust::device_ptr<ClusterDynamicsImpl> self;
  #else
    ClusterDynamicsImpl* self;
  #endif

  // Physics Model Functions

  __CUDADECL__ double i_defect_production(size_t);
  __CUDADECL__ double v_defect_production(size_t);
  __CUDADECL__ double i_clusters_delta(size_t);
  __CUDADECL__ double v_clusters_delta(size_t);
  __CUDADECL__ double iemission_vabsorption_np1(size_t);
  __CUDADECL__ double vemission_iabsorption_np1(size_t);
  __CUDADECL__ double iemission_vabsorption_n(size_t);
  __CUDADECL__ double vemission_iabsorption_n(size_t);
  __CUDADECL__ double iemission_vabsorption_nm1(size_t);
  __CUDADECL__ double vemission_iabsorption_nm1(size_t);
  __CUDADECL__ double i1_cluster_delta(size_t);
  __CUDADECL__ double v1_cluster_delta(size_t);
  __CUDADECL__ double i_emission_time(size_t);
  __CUDADECL__ double v_emission_time(size_t);
  __CUDADECL__ double i_absorption_time(size_t);
  __CUDADECL__ double v_absorption_time(size_t);
  __CUDADECL__ double annihilation_rate();
  __CUDADECL__ double i_dislocation_annihilation_time();
  __CUDADECL__ double v_dislocation_annihilation_time();
  __CUDADECL__ double i_grain_boundary_annihilation_time();
  __CUDADECL__ double v_grain_boundary_annihilation_time();
  __CUDADECL__ double ii_emission(size_t);
  __CUDADECL__ double ii_absorption(size_t);
  __CUDADECL__ double iv_absorption(size_t);
  __CUDADECL__ double vv_emission(size_t);
  __CUDADECL__ double vv_absorption(size_t);
  __CUDADECL__ double vi_absorption(size_t);
  __CUDADECL__ double i_bias_factor(size_t);
  __CUDADECL__ double v_bias_factor(size_t);
  __CUDADECL__ double i_binding_energy(size_t);
  __CUDADECL__ double v_binding_energy(size_t);
  __CUDADECL__ double i_diffusion();
  __CUDADECL__ double v_diffusion();
  __CUDADECL__ double mean_dislocation_cell_radius();
  __CUDADECL__ double dislocation_promotion_probability(size_t);
  __CUDADECL__ double dislocation_density_delta();
  __CUDADECL__ double cluster_radius(size_t);

  // Simulation Operation Functions
  bool update_clusters_1(double);
  bool update_clusters(double);
  double ii_sum_absorption(size_t);
  double iv_sum_absorption(size_t);
  double vv_sum_absorption(size_t);
  double vi_sum_absorption(size_t);
  bool step(double);
  bool validate(size_t);

  // Interface functions
  ClusterDynamicsImpl(size_t concentration_boundary, NuclearReactor reactor, Material material);
    
  ClusterDynamicsState run(double delta_time, double total_time);
  Material get_material();
  void set_material(Material material);
  NuclearReactor get_reactor();
  void set_reactor(NuclearReactor reactor);
};