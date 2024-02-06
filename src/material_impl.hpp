#ifndef MATERIAL_IMPL_HPP
#define MATERIAL_IMPL_HPP

#include "types.hpp"

// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 6
struct MaterialImpl
{
    // migration energy (eV)
    gp_float i_migration;
    gp_float v_migration;

    // diffusion coefficients (pre-exponential) (cm^2 / s)
    gp_float i_diffusion_0;
    gp_float v_diffusion_0;

    // formation energy (eV)
    gp_float i_formation;
    gp_float v_formation;

    // binding energy for bi-interstitials and bi-vacancies (eV)
    gp_float i_binding;
    gp_float v_binding;

    // recombination radius between defects (cm)
    gp_float recombination_radius;

    // bias factor of the loops for interstitials
    gp_float i_loop_bias;

    // bias factor of dislocations for interstitial
    gp_float i_dislocation_bias;
    gp_float i_dislocation_bias_param;

    // bias factor of the loops for vacancies
    gp_float v_loop_bias;

    // bias factor of dislocations for vacancies 
    gp_float v_dislocation_bias;
    gp_float v_dislocation_bias_param;

    // (cm^2)
    gp_float dislocation_density_0;

    // grain size (cm)
    gp_float grain_size;

    // lattice parameter (cm)
    gp_float lattice_param;

    // burgers vector magnitude
    gp_float burgers_vector;

    // average volume of a single atom in the lattice
    gp_float atomic_volume;
};

#endif // MATERIAL_IMPL_HPP