#ifndef MATERIAL_IMPL_HPP
#define MATERIAL_IMPL_HPP

#include "utils/types.hpp"

/// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 6
/// \todo Get better descriptions for the bias factors
struct MaterialImpl {
  /// @brief Single interstitial migration energy in eV.
  gp_float i_migration;
  /// @brief Single vacancy migration energy in eV.
  gp_float v_migration;

  /// @brief Single interstitial preexponential diffusion constant in cm^2/s.
  gp_float i_diffusion_0;
  /// @brief Single vacancy preexponential diffusion constant in cm^2/s.
  gp_float v_diffusion_0;

  /// @brief Interstitial formation energy in eV.
  gp_float i_formation;
  /// @brief Vacancy formation energy in eV.
  gp_float v_formation;

  /// @brief Binding energy for size 2 interstitials in eV.
  gp_float i_binding;
  /// @brief Binding energy for size 2 interstitials in eV.
  gp_float v_binding;

  /// @brief Recombination radius of point defects in cm.
  gp_float recombination_radius;

  /// @brief Interstitial loop bias factor.
  gp_float i_loop_bias;
  /// @brief Interstitial dislocation bias factor.
  gp_float i_dislocation_bias;
  /// @brief Interstitial dislocation bias parameter.
  gp_float i_dislocation_bias_param;

  /// @brief Vacancy loop bias factor.
  gp_float v_loop_bias;
  /// @brief Vacancy dislocation bias factor.
  gp_float v_dislocation_bias;
  /// @brief Vacancy dislocation bias parameter.
  gp_float v_dislocation_bias_param;

  /// @brief Initial dislocation network density in cm^-2.
  gp_float dislocation_density_0;

  /// @brief Grain size in cm.
  gp_float grain_size;

  /// @brief Lattice parameter in cm.
  gp_float lattice_param;

  /// @brief The magnitude of the burgers vector.
  gp_float burgers_vector;

  /// @brief The average volume of a single atom in the material lattice.
  gp_float atomic_volume;
};

#endif  // MATERIAL_IMPL_HPP
