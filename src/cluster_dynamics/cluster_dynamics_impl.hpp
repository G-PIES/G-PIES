#pragma once

#include <vector>
#include <cstdint>
#include <exception>

#include "cluster_dynamics_state.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"

#ifdef USE_CUDA
  #include <thrust/host_vector.h>
  #include <thrust/transform_reduce.h>
  #include <thrust/sequence.h>

  #include <thrust/device_vector.h>
  #include <thrust/device_malloc.h>
  #include <thrust/device_free.h>
#endif

// Used to transform CUDA code into raw C++ for the software version
#ifdef USE_CUDA
    #define __CUDADECL__ __device__ __host__

    template<typename T>
    using vector = thrust::device_vector<T>;
#else
    #define __CUDADECL__

    template<typename T>
    using vector = std::vector<T>;
#endif

class ClusterDynamicsImpl
{
public:
  double time;

  vector<vector<double>> interstitials;
  vector<vector<double>> interstitials_temp;
  vector<vector<double>> vacancies;
  vector<vector<double>> vacancies_temp;

  size_t concentration_boundary;
  size_t max_spatial_dim;
  vector<double> dislocation_density;

  vector<double> mean_dislocation_radius_val;
  vector<double> ii_sum_absorption_val;
  vector<double> iv_sum_absorption_val;
  vector<double> vv_sum_absorption_val;
  vector<double> vi_sum_absorption_val;
  double i1_val;
  double v1_val;

  Material material;
  NuclearReactor reactor;

  //vector<int> indices;
  #ifdef USE_CUDA
    thrust::device_ptr<ClusterDynamicsImpl> self;
  #else
    ClusterDynamicsImpl* self;
  #endif

  // Physics Model Functions

  __CUDADECL__ double i_defect_production(size_t);
  __CUDADECL__ double v_defect_production(size_t);
  __CUDADECL__ double i_clusters_delta(size_t, size_t, double); //Changed, good
  __CUDADECL__ double v_clusters_delta(size_t, size_t, double); //Changed, good
  __CUDADECL__ double iemission_vabsorption_np1(size_t, size_t); //Changed, good
  __CUDADECL__ double vemission_iabsorption_np1(size_t, size_t); //Changed, good
  __CUDADECL__ double iemission_vabsorption_n(size_t, size_t); //Changed, good
  __CUDADECL__ double vemission_iabsorption_n(size_t, size_t); //Changed, good
  __CUDADECL__ double iemission_vabsorption_nm1(size_t, size_t); //Changed, good
  __CUDADECL__ double vemission_iabsorption_nm1(size_t, size_t); //Changed, good
  double i_spatial_diffusion_loss_diffusion(double, size_t, size_t);
  double v_spatial_diffusion_loss_diffusion(double, size_t, size_t);
  double i1_cluster_delta(size_t); //Changed, good
  double v1_cluster_delta(size_t); //Changed, good
  double i_emission_time(size_t); //Changed, good
  double v_emission_time(size_t); //Changed, good
  double i_absorption_time(size_t); //Changed, good
  double v_absorption_time(size_t); //Changed, good
  __CUDADECL__ double annihilation_rate();
  __CUDADECL__ double i_dislocation_annihilation_time(size_t);
  __CUDADECL__ double v_dislocation_annihilation_time(size_t);
  __CUDADECL__ double i_grain_boundary_annihilation_time(size_t);
  __CUDADECL__ double v_grain_boundary_annihilation_time(size_t);
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
  __CUDADECL__ double dislocation_promotion_probability(size_t, size_t);
  __CUDADECL__ double cluster_radius(size_t);

  // Simulation Operation Functions
  bool update_clusters_1(double, double); //Changed, good
  bool update_clusters(double, double); //Changed, good
  double dislocation_density_delta(size_t); //Changed, good
  double mean_dislocation_cell_radius(size_t); //Changed, good
  double ii_sum_absorption(size_t, size_t); //Changed, good
  double iv_sum_absorption(size_t, size_t); //Changed, good
  double vv_sum_absorption(size_t, size_t); //Changed, good
  double vi_sum_absorption(size_t, size_t); //Changed, good
  void step_init(size_t); //Changed, good
  bool step(double, double); //Changed, good
  bool validate(size_t, size_t); //Changed, good

  // Interface functions
  ClusterDynamicsImpl(size_t concentration_boundary, size_t max_spatial_dim, NuclearReactor reactor, Material material);
  ~ClusterDynamicsImpl();

  ClusterDynamicsState run(double delta_time, double total_time, double x_delta);
  Material get_material();
  void set_material(Material material);
  NuclearReactor get_reactor();
  void set_reactor(NuclearReactor reactor);
};
