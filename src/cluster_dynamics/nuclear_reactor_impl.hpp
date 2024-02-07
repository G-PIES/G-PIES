#ifndef NUCLEAR_REACTOR_IMPL_HPP
#define NUCLEAR_REACTOR_IMPL_HPP 

#include "types.hpp"

// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 5
struct NuclearReactorImpl
{
  gp_float flux; //!< Neutron flux through the material in dpa/s

  gp_float temperature; //!< Temperature in Kelvin

  gp_float recombination;  //!< Recombination factor for collision cascades.

  gp_float i_bi; //!< The fraction of generated interstitial clusters which are size 2.
  gp_float i_tri; //!< The fraction of generated interstitial clusters which are size 3.
  gp_float i_quad; //!< The fraction of generated interstitial clusters which are size 4.

  gp_float v_bi; //!< The fraction of generated vacancy clusters which are size 2.
  gp_float v_tri; //!< The fraction of generated vacancy clusters which are size 2.
  gp_float v_quad; //!< The fraction of generated vacancy clusters which are size 2.

  gp_float dislocation_density_evolution; //!< Parameter which affects the evolution of the dislocation network.
};

#endif // NUCLEAR_REACTOR_IMPL_HPP 