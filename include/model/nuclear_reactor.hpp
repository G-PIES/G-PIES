#ifndef NUCLEAR_REACTOR_HPP
#define NUCLEAR_REACTOR_HPP 

#include <string>
#include "conversions.hpp"
#include "datetime.hpp"

/** @brief A class which represents the radiation environment 
 *  parameters for a ClusterDynamics simulation.
 * 
 *  Based loosely on: C. Pokor / Journal of Nuclear Materials 326 (2004), Table 5
*/
struct NuclearReactor
{
    NuclearReactor() : sqlite_id(-1) 
    {
        datetime::utc_now(creation_datetime);
    }

    int sqlite_id;
    std::string creation_datetime; //!< Timestamp of when this Material was first instantiated.

    std::string species; //!< A name for what kind of material this data represents.

    double flux; //!< Neutron flux through the material in dpa/s

    double temperature; //!< Temperature in Kelvin

    double recombination;  //!< Recombination factor for collision cascades.

    double i_bi; //!< The fraction of generated interstitial clusters which are size 2.
    double i_tri; //!< The fraction of generated interstitial clusters which are size 3.
    double i_quad; //!< The fraction of generated interstitial clusters which are size 4.

    double v_bi; //!< The fraction of generated vacancy clusters which are size 2.
    double v_tri; //!< The fraction of generated vacancy clusters which are size 2.
    double v_quad; //!< The fraction of generated vacancy clusters which are size 2.

    double dislocation_density_evolution; //!< Parameter which affects the evolution of the dislocation network.
};

// --------------------------------------------------------------------------------------------
// REACTOR DEFINITIONS
namespace nuclear_reactors
{
/** @brief A function which fills a NuclearReactor object with parameters that roughly correspond to
 *  the French OSIRIS reactor.
 *  @param reactor A reference to the NuclearReactor object to be populated with data.
*/
static inline void OSIRIS(NuclearReactor& reactor)
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