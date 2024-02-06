#ifndef CLUSTER_DYNAMICS_CUDA_IMPL_HPP
#define CLUSTER_DYNAMICS_CUDA_IMPL_HPP

#include <vector>

#include "cluster_dynamics_state.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"

#include <thrust/host_vector.h>
#include <thrust/transform_reduce.h>
#include <thrust/sequence.h>

#include <thrust/device_vector.h>
#include <thrust/device_malloc.h>
#include <thrust/device_free.h>

// Used to transform CUDA code into raw C++ for the software version
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
  gp_float i1_val;
  gp_float v1_val;

  Material material;
  NuclearReactor reactor;

  thrust::device_vector<int> indices;

  thrust::device_ptr<ClusterDynamicsImpl> self;

  // Physics Model Functions

  __CUDADECL__ gp_float i_defect_production(size_t);
  __CUDADECL__ gp_float v_defect_production(size_t);
  __CUDADECL__ gp_float i_clusters_delta(size_t);
  __CUDADECL__ gp_float v_clusters_delta(size_t);
  __CUDADECL__ gp_float iemission_vabsorption_np1(size_t);
  __CUDADECL__ gp_float vemission_iabsorption_np1(size_t);
  __CUDADECL__ gp_float iemission_vabsorption_n(size_t);
  __CUDADECL__ gp_float vemission_iabsorption_n(size_t);
  __CUDADECL__ gp_float iemission_vabsorption_nm1(size_t);
  __CUDADECL__ gp_float vemission_iabsorption_nm1(size_t);
  gp_float i1_cluster_delta();
  gp_float v1_cluster_delta();
  gp_float i_emission_time();
  gp_float v_emission_time();
  gp_float i_absorption_time();
  gp_float v_absorption_time();
  __CUDADECL__ gp_float annihilation_rate();
  __CUDADECL__ gp_float i_dislocation_annihilation_time();
  __CUDADECL__ gp_float v_dislocation_annihilation_time();
  __CUDADECL__ gp_float i_grain_boundary_annihilation_time();
  __CUDADECL__ gp_float v_grain_boundary_annihilation_time();
  __CUDADECL__ gp_float ii_emission(size_t);
  __CUDADECL__ gp_float vv_emission(size_t);
  __CUDADECL__ gp_float ii_absorption(size_t);
  __CUDADECL__ gp_float vi_absorption(size_t);
  __CUDADECL__ gp_float iv_absorption(size_t);
  __CUDADECL__ gp_float vv_absorption(size_t);
  __CUDADECL__ gp_float i_bias_factor(size_t);
  __CUDADECL__ gp_float v_bias_factor(size_t);
  __CUDADECL__ gp_float i_binding_energy(size_t);
  __CUDADECL__ gp_float v_binding_energy(size_t);
  __CUDADECL__ gp_float i_diffusion();
  __CUDADECL__ gp_float v_diffusion();
  __CUDADECL__ gp_float dislocation_promotion_probability(size_t);
  __CUDADECL__ gp_float cluster_radius(size_t);

  // Simulation Operation Functions
  bool update_clusters_1(gp_float);
  bool update_clusters(gp_float);
  gp_float dislocation_density_delta();
  gp_float mean_dislocation_cell_radius();
  gp_float ii_sum_absorption(size_t);
  gp_float iv_sum_absorption(size_t);
  gp_float vv_sum_absorption(size_t);
  gp_float vi_sum_absorption(size_t);
  void step_init();
  bool step(gp_float);
  bool validate(size_t);

  // Interface functions
  ClusterDynamicsImpl(size_t concentration_boundary, const NuclearReactor& reactor, const Material& material);
  ~ClusterDynamicsImpl();
    
  ClusterDynamicsState run(gp_float delta_time, gp_float total_time);
  Material get_material();
  void set_material(const Material& material);
  NuclearReactor get_reactor();
  void set_reactor(const NuclearReactor& reactor);
};

#endif // CLUSTER_DYNAMICS_CUDA_IMPL_HPP