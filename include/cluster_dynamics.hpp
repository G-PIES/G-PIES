#pragma once

#include <vector>
#ifdef WIN32
#include <cstdint>
#endif

#include "nuclear_reactor.hpp"
#include "material.hpp"

// Used to transform CUDA code into raw C++ for the software version
#ifdef USE_CUDA
    #define CUDADECL __device__ __host__
#else
    #define CUDADECL
#endif

struct ClusterDynamicsState
{
    bool valid = true;
    double time = 0.0;
    std::vector<double> interstitials;
    std::vector<double> vacancies;
    double dislocation_density = 0.0;
};

class ClusterDynamics
{
public:
  double time;

  double* interstitials;
  double* interstitials_temp;

  double* vacancies;
  double* vacancies_temp;

  size_t concentration_boundary;
  double dislocation_density;

  Material material;
  NuclearReactor reactor;



  // Physics Model Functions

  CUDADECL double i_defect_production(size_t);
  CUDADECL double v_defect_production(size_t);
  CUDADECL double i_clusters_delta(size_t);
  CUDADECL double v_clusters_delta(size_t);
  CUDADECL double iemission_vabsorption_np1(size_t);
  CUDADECL double vemission_iabsorption_np1(size_t);
  CUDADECL double iemission_vabsorption_n(size_t);
  CUDADECL double vemission_iabsorption_n(size_t);
  CUDADECL double iemission_vabsorption_nm1(size_t);
  CUDADECL double vemission_iabsorption_nm1(size_t);
  CUDADECL double i1_cluster_delta(size_t);
  CUDADECL double v1_cluster_delta(size_t);
  CUDADECL double i_emission_time(size_t);
  CUDADECL double v_emission_time(size_t);
  CUDADECL double i_absorption_time(size_t);
  CUDADECL double v_absorption_time(size_t);
  CUDADECL double annihilation_rate();
  CUDADECL double i_dislocation_annihilation_time();
  CUDADECL double v_dislocation_annihilation_time();
  CUDADECL double i_grain_boundary_annihilation_time(size_t);
  CUDADECL double v_grain_boundary_annihilation_time(size_t);
  CUDADECL double ii_sum_absorption(size_t);
  CUDADECL double iv_sum_absorption(size_t);
  CUDADECL double vv_sum_absorption(size_t);
  CUDADECL double vi_sum_absorption(size_t);
  CUDADECL double ii_emission(size_t);
  CUDADECL double ii_absorption(size_t);
  CUDADECL double iv_absorption(size_t);
  CUDADECL double vv_emission(size_t);
  CUDADECL double vv_absorption(size_t);
  CUDADECL double vi_absorption(size_t);
  CUDADECL double i_bias_factor(size_t);
  CUDADECL double v_bias_factor(size_t);
  CUDADECL double i_binding_energy(size_t);
  CUDADECL double v_binding_energy(size_t);
  CUDADECL double i_diffusion();
  CUDADECL double v_diffusion();
  CUDADECL double mean_dislocation_cell_radius(size_t);
  CUDADECL double dislocation_promotion_probability(size_t);
  CUDADECL double dislocation_density_delta();
  CUDADECL double cluster_radius(size_t);



  // Simulation Control Functions

  CUDADECL void update_cluster_n(size_t n, double delta_time);
  bool update_clusters(double delta_time);
  bool software_update_clusters(double delta_time);
  bool CUDA_update_clusters(double delta_time);
  bool validate(size_t);

public:
  ClusterDynamics(size_t concentration_boundary, NuclearReactor reactor, Material material);
    
  ClusterDynamicsState run(double delta_time, double total_time);
  Material get_material();
  void set_material(Material material);
  NuclearReactor get_reactor();
  void set_reactor(NuclearReactor reactor);
};