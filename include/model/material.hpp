#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <cmath>
#include <string>

#include "conversions.hpp"
#include "datetime.hpp"

#ifndef BOLTZMANN_EV_KELVIN
#define BOLTZMANN_EV_KELVIN 8.6173e-5 // (eV / Kelvin)
#endif

/** @brief A class which represents the material property parameters for a ClusterDynamics simulation. 
 * 
 * Based loosely on: C. Pokor / Journal of Nuclear Materials 326 (2004), Table 6
 * */ 
struct Material
{
    Material() : sqlite_id(-1) 
    {
        datetime::utc_now(creation_datetime);
    }

    int sqlite_id;
    std::string creation_datetime; //!< Timestamp of when this Material was first instantiated.

    std::string species; //!< A name for what kind of material this data represents.

    double i_migration; //!< Single interstitial migration energy in eV.
    double v_migration; //!< Single vacancy migration energy in eV.

    double i_diffusion_0; //!< Single interstitial preexponential diffusion constant in cm^2/s.
    double v_diffusion_0; //!< Single vacancy preexponential diffusion constant in cm^2/s.

    double i_formation; //!< Interstitial formation energy in eV.
    double v_formation; //!< Vacancy formation energy in eV.

    double i_binding; //!< Binding energy for size 2 interstitials in eV.
    double v_binding; //!< Binding energy for size 2 interstitials in eV.

    double recombination_radius; //!< Recombination radius of point defects in cm.

    //!< \todo Get better descriptions for the bias factors
    double i_loop_bias; //!< Interstitial loop bias factor.
    double i_dislocation_bias; //!< Interstitial dislocation bias factor.
    double i_dislocation_bias_param; //!< Interstitial dislocation bias parameter.

    double v_loop_bias; //!< Vacancy loop bias factor.
    double v_dislocation_bias; //!< Vacancy dislocation bias factor.
    double v_dislocation_bias_param; //!< Vacancy dislocation bias parameter.

    double dislocation_density_0; //!< Initial dislocation network density in cm^-2.

    double grain_size; //!< Grain size in cm.

    double lattice_param; //!< Lattice parameter in cm.

    double burgers_vector; //!< The magnitude of the burgers vector.

    double atomic_volume; //!< The average volume of a single atom in the material lattice.
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

/** @brief A function which fills a Material object with parameters that roughly correspond to
 *  the properties of SA304 steel.
 *  @param reactor A reference to the Material object to be populated with data.
*/
static inline void SA304(Material& material) 
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