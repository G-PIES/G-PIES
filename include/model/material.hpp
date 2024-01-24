#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <cmath>
#include <string>

#include "conversions.hpp"
#include "datetime.hpp"

#ifndef BOLTZMANN_EV_KELVIN
#define BOLTZMANN_EV_KELVIN 8.6173e-5 // (eV / Kelvin)
#endif

// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 6
struct Material
{
    Material() : sqlite_id(-1) 
    {
        datetime::utc_now(creation_datetime);
    }

    int sqlite_id;
    std::string creation_datetime;

    std::string species;

    // migration energy (eV)
    double i_migration;
    double v_migration;

    // diffusion coefficients (pre-exponential) (cm^2 / s)
    double i_diffusion_0;
    double v_diffusion_0;

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
    double dislocation_density_0;

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

    const double chromium = PM_CM_CONV(291.);
    const double nickel = PM_CM_CONV(352.4);
    const double fcc_nickel = PM_CM_CONV(360.);
    const double carbon = PM_CM_CONV(246.4);

}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
// MATERIALS 
namespace materials
{

static void SA304(Material& material) 
{
    double lattice_param = lattice_params::fcc_nickel;

    material.species = "SA304";
    material.i_migration = .45;  // eV
    material.v_migration = 1.35; // eV
    material.i_diffusion_0 = 1e-3; // cm^2/s
    material.v_diffusion_0 = .6;   // cm^2/s
    material.i_formation = 4.1;  // eV
    material.v_formation = 1.7;  // eV
    material.i_binding = .6;     // eV
    material.v_binding = .5;     // eV
    material.recombination_radius = .7e-7;  // cm
    material.i_loop_bias = 63.;
    material.i_dislocation_bias = .8; 
    material.i_dislocation_bias_param = 1.1;
    material.v_loop_bias = 33.;
    material.v_dislocation_bias = .65; 
    material.v_dislocation_bias_param = 1.;
    material.dislocation_density_0 = 1. / (double)M_CM_CONV(10e10);
    material.grain_size = 4e-3;
    material.lattice_param = lattice_param;                 //cm
    material.burgers_vector = lattice_param / std::sqrt(2.);
    material.atomic_volume = std::pow(lattice_param, 3) / 4.;    //cm^3
}

}


#endif // MATERIAL_HPP