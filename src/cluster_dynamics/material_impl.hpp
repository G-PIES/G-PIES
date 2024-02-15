#ifndef MATERIAL_IMPL_HPP
#define MATERIAL_IMPL_HPP

#include "types.hpp"

/// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 6
/// \todo Get better descriptions for the bias factors
struct MaterialImpl
{

    gp_float i_migration; //!< Single interstitial migration energy in eV.

    gp_float v_migration; //!< Single vacancy migration energy in eV.

    gp_float i_diffusion_0; //!< Single interstitial preexponential diffusion constant in cm^2/s.
    gp_float v_diffusion_0; //!< Single vacancy preexponential diffusion constant in cm^2/s.

    gp_float i_formation; //!< Interstitial formation energy in eV.
    gp_float v_formation; //!< Vacancy formation energy in eV.

    gp_float i_binding; //!< Binding energy for size 2 interstitials in eV.
    gp_float v_binding; //!< Binding energy for size 2 interstitials in eV.

    gp_float recombination_radius; //!< Recombination radius of point defects in cm.

    gp_float i_loop_bias; //!< Interstitial loop bias factor.
    gp_float i_dislocation_bias; //!< Interstitial dislocation bias factor.
    gp_float i_dislocation_bias_param; //!< Interstitial dislocation bias parameter.

    gp_float v_loop_bias; //!< Vacancy loop bias factor.
    gp_float v_dislocation_bias; //!< Vacancy dislocation bias factor.
    gp_float v_dislocation_bias_param; //!< Vacancy dislocation bias parameter.

    gp_float dislocation_density_0; //!< Initial dislocation network density in cm^-2.

    gp_float grain_size; //!< Grain size in cm.

    gp_float lattice_param; //!< Lattice parameter in cm.

    gp_float burgers_vector; //!< The magnitude of the burgers vector.

    gp_float atomic_volume;
};

#endif // MATERIAL_IMPL_HPP