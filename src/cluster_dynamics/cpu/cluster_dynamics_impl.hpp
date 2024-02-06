#ifndef CLUSTER_DYNAMICS_IMPL_HPP
#define CLUSTER_DYNAMICS_IMPL_HPP

#include <vector>
#include <cmath>

#include "cluster_dynamics_state.hpp"
#include "constants.hpp"
#include "material_impl.hpp"
#include "nuclear_reactor_impl.hpp"

class ClusterDynamicsImpl
{
public:
  gp_float time;

  std::vector<gp_float> interstitials;
  std::vector<gp_float> interstitials_temp;
  std::vector<gp_float> vacancies;
  std::vector<gp_float> vacancies_temp;

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

  // Physics Model Functions
  gp_float i_concentration_derivative(size_t) const;
  gp_float v_concentration_derivative(size_t) const;
  gp_float i1_concentration_derivative() const;
  gp_float v1_concentration_derivative() const;
  gp_float dislocation_density_derivative() const;
  gp_float i_defect_production(size_t) const;
  gp_float v_defect_production(size_t) const;
  gp_float iemission_vabsorption_np1(size_t) const;
  gp_float vemission_iabsorption_np1(size_t) const;
  gp_float iemission_vabsorption_n(size_t) const;
  gp_float vemission_iabsorption_n(size_t) const;
  gp_float iemission_vabsorption_nm1(size_t) const;
  gp_float vemission_iabsorption_nm1(size_t) const;
  gp_float i_emission_time() const;
  gp_float v_emission_time() const;
  gp_float i_absorption_time() const;
  gp_float v_absorption_time() const;
  gp_float annihilation_rate() const;
  gp_float i_dislocation_annihilation_time() const;
  gp_float v_dislocation_annihilation_time() const;
  gp_float i_grain_boundary_annihilation_time() const;
  gp_float v_grain_boundary_annihilation_time() const;
  gp_float ii_emission(size_t) const;
  gp_float vv_emission(size_t) const;
  gp_float ii_absorption(size_t) const;
  gp_float iv_absorption(size_t) const;
  gp_float vi_absorption(size_t) const;
  gp_float vv_absorption(size_t) const;
  gp_float i_bias_factor(size_t) const;
  gp_float v_bias_factor(size_t) const;
  gp_float i_binding_energy(size_t) const;
  gp_float v_binding_energy(size_t) const;
  gp_float dislocation_promotion_probability(size_t) const;
  gp_float cluster_radius(size_t) const;

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
  ClusterDynamicsImpl(size_t concentration_boundary, const NuclearReactorImpl& reactor, const MaterialImpl& material);
  ~ClusterDynamicsImpl();
    
  ClusterDynamicsState run(gp_float delta_time, gp_float total_time);
  MaterialImpl get_material() const;
  void set_material(const MaterialImpl& material);
  NuclearReactorImpl get_reactor() const;
  void set_reactor(const NuclearReactorImpl& reactor);
};

#endif // CLUSTER_DYNAMICS_IMPL_HPP