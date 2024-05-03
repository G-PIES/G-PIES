#ifndef CLUSTER_DYNAMICS_IMPL_HPP
#define CLUSTER_DYNAMICS_IMPL_HPP

#include "utils/diagnostics.hpp"

DIAGNOSTIC_PUSH
DIAGNOSTIC_DISABLE("-Wunused-parameter")
#include <cvodes/cvodes.h>
#include <nvector/nvector_serial.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h>
DIAGNOSTIC_POP

#include <cmath>
#include <iostream>
#include <vector>

#include "cluster_dynamics/cluster_dynamics_config.hpp"
#include "cluster_dynamics/cluster_dynamics_state.hpp"
#include "material_impl.hpp"
#include "nuclear_reactor_impl.hpp"
#include "utils/constants.hpp"

class ClusterDynamicsImpl {
 public:
  gp_float time;
  gp_float dpa;

  N_Vector state;
  SUNContext sun_context;
  SUNMatrix jacobian_matrix;
  SUNLinearSolver linear_solver;
  void* cvodes_memory_block;

  gp_float* interstitials;
  gp_float* vacancies;
  gp_float* dislocation_density;

  size_t max_cluster_size;
  size_t state_size;

  /// @brief Precomputed in step_init() using mean_dislocation_cell_radius()
  gp_float mean_dislocation_radius_val;
  /// @brief Precomputed in step_init() using ii_sum_absorption()
  gp_float ii_sum_absorption_val;
  /// @brief Precomputed in step_init() using iv_sum_absorption()
  gp_float iv_sum_absorption_val;
  /// @brief Precomputed in step_init() using vv_sum_absorption()
  gp_float vv_sum_absorption_val;
  /// @brief Precomputed in step_init() using vi_sum_absorption()
  gp_float vi_sum_absorption_val;
  gp_float i_diffusion_val;  //!< Precomputed in step_init() using i_diffusion()
  gp_float v_diffusion_val;  //!< Precomputed in step_init() using v_diffusion()

  MaterialImpl material;
  NuclearReactorImpl reactor;

  bool data_validation_on;
  gp_float relative_tolerance;
  gp_float absolute_tolerance;
  size_t max_num_integration_steps;
  gp_float min_integration_step;
  gp_float max_integration_step;

  // Physics Model Functions
  gp_float i_concentration_derivative(size_t) const;
  gp_float v_concentration_derivative(size_t) const;
  gp_float i1_concentration_derivative() const;
  gp_float v1_concentration_derivative() const;
  gp_float dislocation_density_derivative() const;
  gp_float i_defect_production(size_t) const;
  gp_float v_defect_production(size_t) const;
  gp_float i_demotion_rate(size_t) const;
  gp_float v_demotion_rate(size_t) const;
  gp_float i_combined_promotion_demotion_rate(size_t) const;
  gp_float v_combined_promotion_demotion_rate(size_t) const;
  gp_float i_promotion_rate(size_t) const;
  gp_float v_promotion_rate(size_t) const;
  gp_float i_emission_rate() const;
  gp_float v_emission_rate() const;
  gp_float i_absorption_rate() const;
  gp_float v_absorption_rate() const;
  gp_float annihilation_rate() const;
  gp_float i_dislocation_annihilation_rate() const;
  gp_float v_dislocation_annihilation_rate() const;
  gp_float i_grain_boundary_annihilation_rate() const;
  gp_float v_grain_boundary_annihilation_rate() const;
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

  gp_float i_diffusion() const;
  gp_float v_diffusion() const;
  gp_float mean_dislocation_cell_radius() const;
  gp_float ii_sum_absorption(size_t) const;
  gp_float iv_sum_absorption(size_t) const;
  gp_float vi_sum_absorption(size_t) const;
  gp_float vv_sum_absorption(size_t) const;

  // Simulation Operation Functions
  void step_init();
  static int system(double t, N_Vector state, N_Vector state_derivatives,
                    void* user_data);
  void validate(size_t) const;

  // Interface functions
  explicit ClusterDynamicsImpl(ClusterDynamicsConfig& config);
  ~ClusterDynamicsImpl();

  ClusterDynamicsState run(gp_float total_time);
  MaterialImpl get_material() const;
  void set_material(const MaterialImpl& material);
  NuclearReactorImpl get_reactor() const;
  void set_reactor(const NuclearReactorImpl& reactor);
};

#endif  // CLUSTER_DYNAMICS_IMPL_HPP
