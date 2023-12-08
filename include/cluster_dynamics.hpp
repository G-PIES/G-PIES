#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#include <vector>

#include "nuclear_reactor.hpp"
#include "material.hpp"

// used formatted debug printing
#define TABS "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"

enum class BackendType
{
  SOFTWARE,
  CUDA,
  OPENCL
};

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
private:
  // --------------------------------------------------------------------------------------------
  // SIMULATION PHYSICS FUNCTIONS 
  double i_defect_production(uint64_t);
  double v_defect_production(uint64_t);
  double i_clusters_delta(uint64_t);
  double v_clusters_delta(uint64_t);
  double iemission_vabsorption_np1(uint64_t);
  double vemission_iabsorption_np1(uint64_t);
  double iemission_vabsorption_n(uint64_t);
  double vemission_iabsorption_n(uint64_t);
  double iemission_vabsorption_nm1(uint64_t);
  double vemission_iabsorption_nm1(uint64_t);
  double i1_cluster_delta(uint64_t);
  double v1_cluster_delta(uint64_t);
  double i_emission_time(uint64_t);
  double v_emission_time(uint64_t);
  double i_absorption_time(uint64_t);
  double v_absorption_time(uint64_t);
  double annihilation_rate();
  double i_dislocation_annihilation_time();
  double v_dislocation_annihilation_time();
  double i_grain_boundary_annihilation_time(uint64_t);
  double v_grain_boundary_annihilation_time(uint64_t);
  double ii_sum_absorption(uint64_t);
  double iv_sum_absorption(uint64_t);
  double vv_sum_absorption(uint64_t);
  double vi_sum_absorption(uint64_t);
  double ii_emission(uint64_t);
  double ii_absorption(uint64_t);
  double iv_absorption(uint64_t);
  double vv_emission(uint64_t);
  double vv_absorption(uint64_t);
  double vi_absorption(uint64_t);
  double i_bias_factor(uint64_t);
  double v_bias_factor(uint64_t);
  double i_binding_energy(uint64_t);
  double v_binding_energy(uint64_t);
  double i_diffusion();
  double v_diffusion();
  double mean_dislocation_cell_radius(uint64_t);
  double dislocation_promotion_probability(uint64_t);
  double dislocation_density_delta();
  double cluster_radius(uint64_t);
  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  // SIMULATION CONTROL FUNCTIONS 
  bool step(double);
  bool update_clusters(double);
  bool update_clusters_software(double);
  bool update_clusters_CUDA(double);
  bool update_clusters_OpenCL(double);

  bool validate(uint64_t, double);
  // --------------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------------
  // STATE
  uint64_t concentration_boundary;
  double time;

  std::vector<double> interstitials;
  std::vector<double> interstitials_temp;
  std::vector<double> vacancies;
  std::vector<double> vacancies_temp;
  double dislocation_density;

  NuclearReactor reactor;
  Material material;

  BackendType backend_type = BackendType::CUDA;
  // --------------------------------------------------------------------------------------------

public:

  ClusterDynamics(uint64_t concentration_boundary, NuclearReactor reactor, Material material);
  ClusterDynamicsState run(double delta_time, double total_time);

  Material get_material();
  void set_material(Material material);
  NuclearReactor get_reactor();
  void set_reactor(NuclearReactor reactor);
};


#endif
