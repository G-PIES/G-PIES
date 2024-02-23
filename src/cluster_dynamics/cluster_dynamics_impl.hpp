#pragma once

#include <vector>
#include <cstdint>

#include "cluster_dynamics_state.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"

#include "cvodes/cvodes.h"
#include "nvector/nvector_serial.h"
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h> 

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

  vector<double> interstitials_temp;
  vector<double> vacancies_temp;

  double* interstitials;
  double* vacancies;
  double* dislocation_density;

  size_t concentration_boundary;

  double mean_dislocation_radius_val;
  double ii_sum_absorption_val;
  double iv_sum_absorption_val;
  double vv_sum_absorption_val;
  double vi_sum_absorption_val;
  double i_diffusion_val;
  double v_diffusion_val;

  SUNContext sunctx;
  realtype t, tout;
  N_Vector y;
  N_Vector abstol;
  SUNMatrix A;
  SUNLinearSolver LS;
  void *cvode_mem;

  Material material;
  NuclearReactor reactor;

  #ifdef USE_CUDA
    vector<int> indices;
    thrust::device_ptr<ClusterDynamicsImpl> self;
    thrust::host_vector<double> host_interstitials;
    thrust::host_vector<double> host_vacancies;
  #endif

  // Physics Model Functions

  __CUDADECL__ double i_concentration_derivative(size_t) const;
  __CUDADECL__ double v_concentration_derivative(size_t) const;
  double i1_concentration_derivative() const;
  double v1_concentration_derivative() const;
  double dislocation_density_derivative() const;

  __CUDADECL__ double i_defect_production(size_t) const;
  __CUDADECL__ double v_defect_production(size_t) const;
  __CUDADECL__ double iemission_vabsorption_np1(size_t) const;
  __CUDADECL__ double vemission_iabsorption_np1(size_t) const;
  __CUDADECL__ double iemission_vabsorption_n(size_t) const;
  __CUDADECL__ double vemission_iabsorption_n(size_t) const;
  __CUDADECL__ double iemission_vabsorption_nm1(size_t) const;
  __CUDADECL__ double vemission_iabsorption_nm1(size_t) const;
  double i_emission_time() const;
  double v_emission_time() const;
  double i_absorption_time() const;
  double v_absorption_time() const;
  __CUDADECL__ double annihilation_rate() const;
  __CUDADECL__ double i_dislocation_annihilation_time() const;
  __CUDADECL__ double v_dislocation_annihilation_time() const;
  __CUDADECL__ double i_grain_boundary_annihilation_time() const;
  __CUDADECL__ double v_grain_boundary_annihilation_time() const;
  __CUDADECL__ double ii_emission(size_t) const;
  __CUDADECL__ double vv_emission(size_t) const;
  __CUDADECL__ double ii_absorption(size_t) const;
  __CUDADECL__ double iv_absorption(size_t) const;
  __CUDADECL__ double vi_absorption(size_t) const;
  __CUDADECL__ double vv_absorption(size_t) const;
  __CUDADECL__ double i_bias_factor(size_t) const;
  __CUDADECL__ double v_bias_factor(size_t) const;
  __CUDADECL__ double i_binding_energy(size_t) const;
  __CUDADECL__ double v_binding_energy(size_t) const;
  __CUDADECL__ double dislocation_promotion_probability(size_t) const;
  __CUDADECL__ double cluster_radius(size_t) const;

  // Value Precalculation Functions
  double i_diffusion() const;
  double v_diffusion() const;
  double ii_sum_absorption(size_t) const;
  double iv_sum_absorption(size_t) const;
  double vv_sum_absorption(size_t) const;
  double vi_sum_absorption(size_t) const;
  double mean_dislocation_cell_radius() const;

  // Simulation Operation Functions
  bool update_clusters_1(double);
  bool update_clusters(double);
  void step_init();
  bool validate(size_t) const;

  int system(double, N_Vector, N_Vector, void*);

  // Interface functions
  ClusterDynamicsImpl(size_t concentration_boundary, NuclearReactor reactor, Material material);
  ~ClusterDynamicsImpl();
    
  ClusterDynamicsState run(double delta_time, double total_time);
  Material get_material() const;
  void set_material(Material material);
  NuclearReactor get_reactor() const;
  void set_reactor(NuclearReactor reactor);
};