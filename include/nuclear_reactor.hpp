#ifndef NUCLEAR_REACTOR_HPP
#define NUCLEAR_REACTOR_HPP 

#include <string>
#include "conversions.hpp"
#include "datetime.hpp"

// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 5
struct NuclearReactor
{
    NuclearReactor() : sqlite_id(-1) 
    {
        datetime::utc_now(creation_datetime);
    }

    int sqlite_id;
    std::string creation_datetime;

    std::string species;

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

static void OSIRIS(NuclearReactor& reactor)
{
    reactor.species = "OSIRIS";
    reactor.flux = 2.9e-7;
    reactor.temperature = CELCIUS_KELVIN_CONV(330.);
    reactor.recombination = .3; 
    reactor.i_bi = .5;
    reactor.i_tri = .2;
    reactor.i_quad = .06;
    reactor.v_bi = .06;
    reactor.v_tri = .03;
    reactor.v_quad = .02;
    reactor.dislocation_density_evolution = 300.;
}

}

#endif // NUCLEAR_REACTOR_HPP