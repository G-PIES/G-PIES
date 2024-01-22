#ifndef NUCLEAR_REACTOR_HPP
#define NUCLEAR_REACTOR_HPP 

#include "conversions.hpp"

// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 5
struct NuclearReactor
{
    int sqlite_id = -1;
    const char* sqlite_creation_datetime;

    const char* species;

    // neutron flux inside of the nuclear reactor (cm^2 / s)
    double flux; 

    // (Kelvin) 
    double temperature;

    // recombination in the cascades
    double recombination; 

    // interstitials in the cascades
    double i_bi;
    double i_tri;
    double i_quad;

    // vacancies in the cascades
    double v_bi;
    double v_tri;
    double v_quad;

    // factor of dislocation density evolution
    double dislocation_density_evolution;
};

// --------------------------------------------------------------------------------------------
// REACTOR DEFINITIONS
namespace nuclear_reactors
{

inline NuclearReactor OSIRIS()
{
    return
    {
        .species = "OSIRIS",
        .flux = 2.9e-7,
        .temperature = CELCIUS_KELVIN_CONV(330.),
        .recombination = .3, 
        .i_bi = .5,
        .i_tri = .2,
        .i_quad = .06,
        .v_bi = .06,
        .v_tri = .03,
        .v_quad = .02,
        .dislocation_density_evolution = 300.
    };
}

}

#endif