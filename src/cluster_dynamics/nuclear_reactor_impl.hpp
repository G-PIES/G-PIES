#ifndef NUCLEAR_REACTOR_IMPL_HPP
#define NUCLEAR_REACTOR_IMPL_HPP

#include "utils/types.hpp"

// C. Pokor / Journal of Nuclear Materials 326 (2004), Table 5
struct NuclearReactorImpl {
    /// @brief Neutron flux through the material in dpa/s
    gp_float flux;

    /// @brief Temperature in Kelvin
    gp_float temperature;

    /// @brief Recombination factor for collision cascades.
    gp_float recombination;

    /// @brief The fraction of generated interstitial clusters which are size 2.
    gp_float i_bi;
    /// @brief The fraction of generated interstitial clusters which are size 3.
    gp_float i_tri;
    /// @brief The fraction of generated interstitial clusters which are size 4.
    gp_float i_quad;

    /// @brief The fraction of generated vacancy clusters which are size 2.
    gp_float v_bi;
    /// @brief The fraction of generated vacancy clusters which are size 2.
    gp_float v_tri;
    /// @brief The fraction of generated vacancy clusters which are size 2.
    gp_float v_quad;

    /// @brief Parameter which affects the evolution of the dislocation network.
    gp_float dislocation_density_evolution;
};

#endif  // NUCLEAR_REACTOR_IMPL_HPP
