#include "model/nuclear_reactor.hpp"
#include "nuclear_reactor_impl.hpp"
#include "utils/datetime.hpp"

NuclearReactor::NuclearReactor() : sqlite_id(-1) {
    datetime::utc_now(creation_datetime);
    _impl = std::make_unique<NuclearReactorImpl>();
}

NuclearReactor::NuclearReactor(const NuclearReactor &other)
    : sqlite_id(other.sqlite_id),
      creation_datetime(other.creation_datetime),
      species(other.species) {
    _impl = other._impl;
}

NuclearReactor::~NuclearReactor() {}

NuclearReactor &NuclearReactor::operator=(const NuclearReactor &other) {
    if (this != &other) copy(other);

    return *this;
}

void NuclearReactor::copy(const NuclearReactor &other) {
    sqlite_id = other.sqlite_id;
    creation_datetime = other.creation_datetime;
    species = other.species;

    _impl = other._impl;
}

/// @brief Returns the neutron flux through the material in dpa/s
gp_float NuclearReactor::get_flux() const { return _impl->flux; }

/// @brief Sets the neutron flux through the material.
/// @param val Neutron flux in dpa/s
void NuclearReactor::set_flux(const gp_float val) { _impl->flux = val; }

/// @brief Returns the temperature in Kelvin.
gp_float NuclearReactor::get_temperature() const { return _impl->temperature; }

/// @brief Sets the temperature.
/// @param val Temperature in Kelvin
void NuclearReactor::set_temperature(const gp_float val) {
    _impl->temperature = val;
}

/// @brief Returns the recombination factor for collision cascades.
gp_float NuclearReactor::get_recombination() const {
    return _impl->recombination;
}

/// @brief Sets the recombination factor for collision cascades.
/// @param val Recombination factor
void NuclearReactor::set_recombination(const gp_float val) {
    _impl->recombination = val;
}

/// @brief Returns the fraction of generated interstitial clusters which are
/// size 2.
gp_float NuclearReactor::get_i_bi() const { return _impl->i_bi; }

/// @brief Sets the fraction of generated interstitial clusters which are
/// size 2.
/// @param val Fraction of generated interstitial clusters which are size 2
void NuclearReactor::set_i_bi(const gp_float val) { _impl->i_bi = val; }

/// @brief Returns the fraction of generated interstitial clusters which are
/// size 3.
gp_float NuclearReactor::get_i_tri() const { return _impl->i_tri; }

/// @brief Sets the fraction of generated interstitial clusters which are
/// size 3.
/// @param val Fraction of generated interstitial clusters which are size 3
void NuclearReactor::set_i_tri(const gp_float val) { _impl->i_tri = val; }

/// @brief Returns the fraction of generated interstitial clusters which are
/// size 4.
gp_float NuclearReactor::get_i_quad() const { return _impl->i_quad; }

/// @brief Sets the fraction of generated interstitial clusters which are
/// size 4.
/// @param val Fraction of generated interstitial clusters which are size 4.
void NuclearReactor::set_i_quad(const gp_float val) { _impl->i_quad = val; }

/// @brief Returns the fraction of generated vacancy clusters which are size 2.
gp_float NuclearReactor::get_v_bi() const { return _impl->v_bi; }

/// @brief Sets the fraction of generated vacancy clusters which are size 2.
/// @param val Fraction of generated vacancy clusters which are size 2.
void NuclearReactor::set_v_bi(const gp_float val) { _impl->v_bi = val; }

/// @brief Returns the fraction of generated vacancy clusters which are size 3.
gp_float NuclearReactor::get_v_tri() const { return _impl->v_tri; }

/// @brief Sets the fraction of generated vacancy clusters which are size 3.
/// @param val Fraction of generated vacancy clusters which are size 3.
void NuclearReactor::set_v_tri(const gp_float val) { _impl->v_tri = val; }

/// @brief Returns the fraction of generated vacancy clusters which are size 4.
gp_float NuclearReactor::get_v_quad() const { return _impl->v_quad; }

/// @brief Sets the fraction of generated vacancy clusters which are size 4.
/// @param val Fraction of generated vacancy clusters which are size 4.
void NuclearReactor::set_v_quad(const gp_float val) { _impl->v_quad = val; }

/// @brief Returns the parameter of dislocation network evolution.
gp_float NuclearReactor::get_dislocation_density_evolution() const {
    return _impl->dislocation_density_evolution;
}

/// @brief Sets the parameter of dislocation network evolution.
/// @param val The parameter of dislocation network evolution.
void NuclearReactor::set_dislocation_density_evolution(const gp_float val) {
    _impl->dislocation_density_evolution = val;
}

NuclearReactorImpl *NuclearReactor::impl() { return _impl.get(); }

namespace nuclear_reactors {
/** @brief A function which fills a NuclearReactor object with parameters that
 * roughly correspond to the French OSIRIS reactor.
 *  @param reactor A reference to the NuclearReactor object to be populated with
 * data.
 */
void OSIRIS(NuclearReactor &reactor) {
    reactor.species = "OSIRIS";
    reactor._impl->flux = 2.9e-7;
    reactor._impl->temperature = CELCIUS_KELVIN_CONV(330.);
    reactor._impl->recombination = .3;
    reactor._impl->i_bi = .5;
    reactor._impl->i_tri = .2;
    reactor._impl->i_quad = .06;
    reactor._impl->v_bi = .06;
    reactor._impl->v_tri = .03;
    reactor._impl->v_quad = .02;
    reactor._impl->dislocation_density_evolution = 300.;
}
}  // namespace nuclear_reactors
