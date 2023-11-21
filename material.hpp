#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <cmath>

#include "conversions.hpp"

// C. Pokor et al. / Journal of Nuclear Materials 326 (2004), Table 6
struct Material
{
    const char* species;

    // migration energy (eV)
    double i_migration;
    double v_migration;

    // diffusion coefficients (pre-exponential) (cm^2 / s)
    double i_diffusion;
    double v_diffusion;

    // formation energy (eV)
    double i_formation;
    double v_formation;

    // binding energy for bi-interstitials and bi-vacancies (eV)
    double i_binding;
    double v_binding;

    // recombination radius between defects (cm)
    double recombination_radius;

    // bias factor of the loops for interstitials
    double i_loop_bias;

    // bias factor of dislocations for interstitial
    double i_dislocation_bias;
    double i_dislocation_bias_param;

    // bias factor of the loops for vacancies
    double v_loop_bias;

    // bias factor of dislocations for vacancies 
    double v_dislocation_bias;
    double v_dislocation_bias_param;

    // (cm^2)
    double dislocation_density_initial;

    // grain size (cm)
    double grain_size;

    // lattice parameter (cm)
    double lattice_param;

    // burgers vector magnitude
    double burgers_vector;

    // average volume of a single atom in the lattice
    double atomic_volume;
};


// --------------------------------------------------------------------------------------------
// LATTICE PARAMETERS
namespace lattice_params
{

    const double chromium = 291. * PM_CM_CONV;
    const double nickel = 352.4 * PM_CM_CONV;
    const double fcc_nickel = 360. * PM_CM_CONV;
    const double carbon = 246.4 * PM_CM_CONV;

}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
// MATERIALS 
namespace materials
{

inline Material SA304() 
{
    double lattice_param = lattice_params::fcc_nickel;

    return
    { 
        .species = "SA304",
        .i_migration = .45,
        .v_migration = 1.35,
        .i_diffusion = 1e-3,
        .v_diffusion = .6,
        .i_formation = 4.1,
        .v_formation = 1.7,
        .i_binding = .6,
        .v_binding = .5,
        .recombination_radius = .7e-7,
        .i_loop_bias = 63.,
        .i_dislocation_bias = .8, 
        .i_dislocation_bias_param = 1.1,
        .v_loop_bias = 33.,
        .v_dislocation_bias = .65, 
        .v_dislocation_bias_param = 1.,
        .dislocation_density_initial = 1. / (10e10 * M_CM_CONV),
        .grain_size = 4e-3,
        .lattice_param = lattice_param,
        .burgers_vector = lattice_param / pow(2., .5),
        .atomic_volume = pow(lattice_param, 3.) / 4.
    };
}

}


#endif