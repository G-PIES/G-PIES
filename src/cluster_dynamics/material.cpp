#include "model/material.hpp"

#include "material_impl.hpp"
#include "utils/datetime.hpp"

Material::Material() : sqlite_id(-1) {
  datetime::utc_now(creation_datetime);
  _impl = std::make_shared<MaterialImpl>();
}

Material::Material(const Material &other)
    : sqlite_id(other.sqlite_id),
      creation_datetime(other.creation_datetime),
      species(other.species) {
  _impl = other._impl;
}

Material::~Material() {}

Material &Material::operator=(const Material &other) {
  if (this != &other) copy(other);

  return *this;
}

void Material::copy(const Material &other) {
  sqlite_id = other.sqlite_id;
  creation_datetime = other.creation_datetime;
  species = other.species;

  _impl = other._impl;
}

/// @brief Returns the single interstitial migration energy in eV.
gp_float Material::get_i_migration() const { return _impl->i_migration; }

/** @brief Sets the single interstitial migration energy.
 *  @param val Migration energy in units of eV.
 * */
void Material::set_i_migration(const gp_float val) { _impl->i_migration = val; }

/// @brief Returns the single vacancy migration energy in eV.
gp_float Material::get_v_migration() const { return _impl->v_migration; }

/** @brief Sets the single vacancy migration energy.
 *  @param val Migration energy in units of eV.
 * */
void Material::set_v_migration(const gp_float val) { _impl->v_migration = val; }

/// @brief Returns the single interstitial preexponential diffusion constant in
/// cm^2/s.
gp_float Material::get_i_diffusion_0() const { return _impl->i_diffusion_0; }

/** @brief Sets the single interstitial preexponential diffusion constant.
 *  @param val Preexponential diffusion constant in units of cm^2/s.
 * */
void Material::set_i_diffusion_0(const gp_float val) {
  _impl->i_diffusion_0 = val;
}

/// @brief Returns the single vacancy preexponential diffusion constant in
/// cm^2/s.
gp_float Material::get_v_diffusion_0() const { return _impl->v_diffusion_0; }

/** @brief Sets the single vacancy preexponential diffusion constant.
 *  @param val Preexponential diffusion constant in units of cm^2/s.
 * */
void Material::set_v_diffusion_0(const gp_float val) {
  _impl->v_diffusion_0 = val;
}

/// @brief Returns the interstitial formation energy in eV.
gp_float Material::get_i_formation() const { return _impl->i_formation; }

/** @brief Sets the the interstitial formation energy.
 *  @param val Formation energy in units of eV.
 * */
void Material::set_i_formation(const gp_float val) { _impl->i_formation = val; }

/// @brief Returns the vacancy formation energy in eV.
gp_float Material::get_v_formation() const { return _impl->v_formation; }

/** @brief Sets the the vacancy formation energy.
 *  @param val Formation energy in units of eV.
 * */
void Material::set_v_formation(const gp_float val) { _impl->v_formation = val; }

/// @brief Binding energy for size 2 interstitials in eV.
gp_float Material::get_i_binding() const { return _impl->i_binding; }

/** @brief Sets the the vacancy formation energy.
 *  @param val Binding energy in units of eV.
 * */
void Material::set_i_binding(const gp_float val) { _impl->i_binding = val; }

/// @brief Binding energy for size 2 vacancies in eV.
gp_float Material::get_v_binding() const { return _impl->v_binding; }

/** @brief Sets the the binding energy for size 2 vacancies.
 *  @param val Binding energy in units of eV.
 * */
void Material::set_v_binding(const gp_float val) { _impl->v_binding = val; }

/// @brief Recombination radius of point defects in cm.
gp_float Material::get_recombination_radius() const {
  return _impl->recombination_radius;
}
/// @brief Sets the recombination radius of point defects.
/// @param val Recombination radius in units of cm
void Material::set_recombination_radius(const gp_float val) {
  _impl->recombination_radius = val;
}

/// @brief Returns the interstitial loop bias factor.
gp_float Material::get_i_loop_bias() const { return _impl->i_loop_bias; }

/// @brief Sets the interstitial loop bias factor.
/// @param val Interstitial loop bias factor.
void Material::set_i_loop_bias(const gp_float val) { _impl->i_loop_bias = val; }

//!< \todo Get better descriptions for the bias factors
/// @brief Returns the interstitial dislocation bias factor.
gp_float Material::get_i_dislocation_bias() const {
  return _impl->i_dislocation_bias;
}

/// @brief Sets the interstitial dislocation bias factor.
/// @param val Interstitial dislocation bias factor.
void Material::set_i_dislocation_bias(const gp_float val) {
  _impl->i_dislocation_bias = val;
}

/// @brief Returns the interstitial dislocation bias parameter.
gp_float Material::get_i_dislocation_bias_param() const {
  return _impl->i_dislocation_bias_param;
}

/// @brief Sets the interstitial dislocation bias parameter.
/// @param val Interstitial dislocation bias parameter.
void Material::set_i_dislocation_bias_param(const gp_float val) {
  _impl->i_dislocation_bias_param = val;
}

/// @brief Returns the vacancy loop bias factor.
gp_float Material::get_v_loop_bias() const { return _impl->v_loop_bias; }

/// @brief Sets the vacancy loop bias factor.
/// @param val Vacancy loop bias factor.
void Material::set_v_loop_bias(const gp_float val) { _impl->v_loop_bias = val; }

/// @brief Returns the vacancy dislocation bias factor.
gp_float Material::get_v_dislocation_bias() const {
  return _impl->v_dislocation_bias;
}

/// @brief Sets the vacancy dislocation bias factor.
/// @param val Dislocation bias factor.
void Material::set_v_dislocation_bias(const gp_float val) {
  _impl->v_dislocation_bias = val;
}

/// @brief Returns the vacancy dislocation bias parameter.
gp_float Material::get_v_dislocation_bias_param() const {
  return _impl->v_dislocation_bias_param;
}

/// @brief Sets the vacancy dislocation bias parameter.
/// @param val Vacancy dislocation bias parameter.
void Material::set_v_dislocation_bias_param(const gp_float val) {
  _impl->v_dislocation_bias_param = val;
}

/// @brief Returns the initial dislocation network density in cm^-2.
gp_float Material::get_dislocation_density_0() const {
  return _impl->dislocation_density_0;
}

/// @brief Sets the initial dislocation network density.
/// @param val Initial dislocation network density in cm^-2.
void Material::set_dislocation_density_0(const gp_float val) {
  _impl->dislocation_density_0 = val;
}

/// @brief Returns the grain size in cm.
gp_float Material::get_grain_size() const { return _impl->grain_size; }

/// @brief Sets the grain size.
/// @param val Grain size in cm.
void Material::set_grain_size(const gp_float val) { _impl->grain_size = val; }

/// @brief Returns the lattice parameter in cm.
gp_float Material::get_lattice_param() const { return _impl->lattice_param; }

/// @brief Sets the lattice parameter.
/// @param val Lattice parameter in cm.
void Material::set_lattice_param(const gp_float val) {
  _impl->lattice_param = val;
  _impl->burgers_vector = _impl->lattice_param / std::sqrt(2.);
  _impl->atomic_volume = std::pow(_impl->lattice_param, 3) / 4.;  // cm^3
}

/// @brief Returns the magnitude of the burgers vector.
gp_float Material::get_burgers_vector() const { return _impl->burgers_vector; }

/// @brief Sets the magnitude of the burgers vector.
/// @param val Magnitude of the burgers vector.
void Material::set_burgers_vector(const gp_float val) {
  _impl->burgers_vector = val;
}

/// @brief Returns the average volume of a single atom in the material lattice
/// in cm^3.
gp_float Material::get_atomic_volume() const { return _impl->atomic_volume; }

/// @brief Sets the average volume of a single atom in the material lattice.
/// @param val Atomic volume in cm^3.
void Material::set_atomic_volume(const gp_float val) {
  _impl->atomic_volume = val;
}

/// @brief Returns a raw pointer to the underlying MaterialImpl object.
MaterialImpl *Material::impl() { return _impl.get(); }

namespace materials {

/** @brief A function which fills a Material object with parameters that roughly
 * correspond to the properties of SA304 steel.
 *  @param material A reference to the Material object to be populated with
 * data.
 */
void SA304(Material &material) {
  gp_float lattice_param = lattice_params::fcc_nickel;

  material.species = "SA304";
  material._impl->i_migration = .45;             // eV
  material._impl->v_migration = 1.35;            // eV
  material._impl->i_diffusion_0 = 1e-3;          // cm^2/s
  material._impl->v_diffusion_0 = .6;            // cm^2/s
  material._impl->i_formation = 4.1;             // eV
  material._impl->v_formation = 1.7;             // eV
  material._impl->i_binding = .6;                // eV
  material._impl->v_binding = .5;                // eV
  material._impl->recombination_radius = .7e-7;  // cm
  material._impl->i_loop_bias = 63.;
  material._impl->i_dislocation_bias = .8;
  material._impl->i_dislocation_bias_param = 1.1;
  material._impl->v_loop_bias = 33.;
  material._impl->v_dislocation_bias = .65;
  material._impl->v_dislocation_bias_param = 1.;
  material._impl->dislocation_density_0 = 1. / (gp_float)M_CM_CONV(10e10);
  material._impl->grain_size = 4e-3;
  material._impl->lattice_param = lattice_param;  // cm
  material._impl->burgers_vector = lattice_param / std::sqrt(2.);
  material._impl->atomic_volume = std::pow(lattice_param, 3) / 4.;  // cm^3
}

}  // namespace materials
