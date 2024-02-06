#ifndef NUCLEAR_REACTOR_IMPL_HPP
#define NUCLEAR_REACTOR_IMPL_HPP 

#include "types.hpp"

// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 5
struct NuclearReactorImpl
{
    // neutron flux inside of the nuclear reactor (cm^2 / s)
    gp_float flux;

    // (Kelvin) 
    gp_float temperature;

    // recombination in the cascades
    gp_float recombination; 

    // interstitials in the cascades
    gp_float i_bi;
    gp_float i_tri;
    gp_float i_quad;

    // vacancies in the cascades
    gp_float v_bi;
    gp_float v_tri;
    gp_float v_quad;

    // factor of dislocation density evolution
    gp_float dislocation_density_evolution;
};

#endif // NUCLEAR_REACTOR_IMPL_HPP 