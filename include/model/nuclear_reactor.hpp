#ifndef NUCLEAR_REACTOR_HPP
#define NUCLEAR_REACTOR_HPP

#include <algorithm>
#include <memory>
#include <string>

#include "utils/conversions.hpp"
#include "utils/types.hpp"

struct NuclearReactorImpl;

/** @brief A class which represents the radiation environment
 *  parameters for a ClusterDynamics simulation.
 *
 *  Based loosely on: C. Pokor / Journal of Nuclear Materials 326 (2004), Table
 * 5
 */
struct NuclearReactor {
    NuclearReactor();
    NuclearReactor(const NuclearReactor &other);

    ~NuclearReactor();

    NuclearReactor &operator=(const NuclearReactor &other);
    void copy(const NuclearReactor &other);

    /// @brief Returns the neutron flux through the material in dpa/s
    gp_float get_flux() const;

    /// @brief Sets the neutron flux through the material.
    /// @param val Neutron flux in dpa/s
    void set_flux(const gp_float val);

    /// @brief Returns the temperature in Kelvin.
    gp_float get_temperature() const;

    /// @brief Sets the temperature.
    /// @param val Temperature in Kelvin
    void set_temperature(const gp_float val);

    /// @brief Returns the recombination factor for collision cascades.
    gp_float get_recombination() const;

    /// @brief Sets the recombination factor for collision cascades.
    /// @param val Recombination factor
    void set_recombination(const gp_float val);

    /// @brief Returns the fraction of generated interstitial clusters which are
    /// size 2.
    gp_float get_i_bi() const;

    /// @brief Sets the fraction of generated interstitial clusters which are
    /// size 2.
    /// @param val Fraction of generated interstitial clusters which are size 2
    void set_i_bi(const gp_float val);

    /// @brief Returns the fraction of generated interstitial clusters which are
    /// size 3.
    gp_float get_i_tri() const;

    /// @brief Sets the fraction of generated interstitial clusters which are
    /// size 3.
    /// @param val Fraction of generated interstitial clusters which are size 3
    void set_i_tri(const gp_float val);

    /// @brief Returns the fraction of generated interstitial clusters which are
    /// size 4.
    gp_float get_i_quad() const;

    /// @brief Sets the fraction of generated interstitial clusters which are
    /// size 4.
    /// @param val Fraction of generated interstitial clusters which are size 4.
    void set_i_quad(const gp_float val);

    /// @brief Returns the fraction of generated vacancy clusters which are
    /// size 2.
    gp_float get_v_bi() const;

    /// @brief Sets the fraction of generated vacancy clusters which are size 2.
    /// @param val Fraction of generated vacancy clusters which are size 2.
    void set_v_bi(const gp_float val);

    /// @brief Returns the fraction of generated vacancy clusters which are
    /// size 3.
    gp_float get_v_tri() const;

    /// @brief Sets the fraction of generated vacancy clusters which are size 3.
    /// @param val Fraction of generated vacancy clusters which are size 3.
    void set_v_tri(const gp_float val);

    /// @brief Returns the fraction of generated vacancy clusters which are
    /// size 4.
    gp_float get_v_quad() const;

    /// @brief Sets the fraction of generated vacancy clusters which are size 4.
    /// @param val Fraction of generated vacancy clusters which are size 4.
    void set_v_quad(const gp_float val);

    /// @brief Returns the parameter of dislocation network evolution.
    gp_float get_dislocation_density_evolution() const;

    /// @brief Sets the parameter of dislocation network evolution.
    /// @param val The parameter of dislocation network evolution.
    void set_dislocation_density_evolution(const gp_float val);

    NuclearReactorImpl *impl();

    int sqlite_id;
    std::string creation_datetime;  //!< Timestamp of when this Material was
                                    //!< first instantiated.
    std::string
        species;  //!< A name for what kind of material this data represents.

    std::shared_ptr<NuclearReactorImpl> _impl;
};

// --------------------------------------------------------------------------------------------
// REACTOR DEFINITIONS
namespace nuclear_reactors {
/** @brief A function which fills a NuclearReactor object with parameters that
 * roughly correspond to the French OSIRIS reactor.
 *  @param reactor A reference to the NuclearReactor object to be populated with
 * data.
 */
void OSIRIS(NuclearReactor &reactor);
}  // namespace nuclear_reactors

#endif  // NUCLEAR_REACTOR_HPP
