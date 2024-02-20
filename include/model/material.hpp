#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <cmath>
#include <string>
#include <algorithm>
#include <memory>

#include "utils/constants.hpp"
#include "utils/conversions.hpp"
#include "utils/datetime.hpp"
#include "utils/types.hpp"

struct MaterialImpl;

/** @brief A class which represents the material property parameters for a
 * ClusterDynamics simulation.
 *
 * Based loosely on: C. Pokor / Journal of Nuclear Materials 326 (2004), Table 6
 * */
struct Material {
    Material();
    Material(const Material &other);
    ~Material();

    Material &operator=(const Material &other);
    void copy(const Material &other);

    /// @brief Returns the single interstitial migration energy in eV.
    gp_float get_i_migration() const;

    /// @brief Sets the single interstitial migration energy.
    /// @param val Migration energy in units of eV.
    void set_i_migration(const gp_float val);

    /// @brief Returns the single vacancy migration energy in eV.
    gp_float get_v_migration() const;

    /// @brief Sets the single vacancy migration energy.
    /// @param val Migration energy in units of eV.
    void set_v_migration(const gp_float val);

    /// @brief Returns the single interstitial preexponential diffusion constant
    /// in cm^2/s.
    gp_float get_i_diffusion_0() const;

    /// @brief Sets the single interstitial preexponential diffusion constant.
    /// @param val Preexponential diffusion constant in units of cm^2/s.
    void set_i_diffusion_0(const gp_float val);

    /// @brief Returns the single vacancy preexponential diffusion constant in
    /// cm^2/s.
    gp_float get_v_diffusion_0() const;

    /// @brief Sets the single vacancy preexponential diffusion constant.
    /// @param val Preexponential diffusion constant in units of cm^2/s.
    void set_v_diffusion_0(const gp_float val);

    /// @brief Returns the interstitial formation energy in eV.
    gp_float get_i_formation() const;

    /// @brief Sets the the interstitial formation energy.
    /// @param val Formation energy in units of eV.
    void set_i_formation(const gp_float val);

    /// @brief Returns the vacancy formation energy in eV.
    gp_float get_v_formation() const;

    /// @brief Sets the the vacancy formation energy.
    /// @param val Formation energy in units of eV.
    void set_v_formation(const gp_float val);

    /// @brief Binding energy for size 2 interstitials in eV.
    gp_float get_i_binding() const;

    /// @brief Sets the the vacancy formation energy.
    /// @param val Binding energy in units of eV.
    void set_i_binding(const gp_float val);

    /// @brief Binding energy for size 2 vacancies in eV.
    gp_float get_v_binding() const;

    /// @brief Sets the the binding energy for size 2 vacancies.
    /// @param val Binding energy in units of eV.
    void set_v_binding(const gp_float val);

    /// @brief Recombination radius of point defects in cm.
    gp_float get_recombination_radius() const;

    /// @brief Sets the recombination radius of point defects.
    /// @param val Recombination radius in units of cm
    void set_recombination_radius(const gp_float val);

    /// @brief Returns the interstitial loop bias factor.
    gp_float get_i_loop_bias() const;

    /// @brief Sets the interstitial loop bias factor.
    /// @param val Interstitial loop bias factor.
    void set_i_loop_bias(const gp_float val);

    //!< \todo Get better descriptions for the bias factors
    /// @brief Returns the interstitial dislocation bias factor.
    gp_float get_i_dislocation_bias() const;

    /// @brief Sets the interstitial dislocation bias factor.
    /// @param val Interstitial dislocation bias factor.
    void set_i_dislocation_bias(const gp_float val);

    /// @brief Returns the interstitial dislocation bias parameter.
    gp_float get_i_dislocation_bias_param() const;

    /// @brief Sets the interstitial dislocation bias parameter.
    /// @param val Interstitial dislocation bias parameter.
    void set_i_dislocation_bias_param(const gp_float val);

    /// @brief Returns the vacancy loop bias factor.
    gp_float get_v_loop_bias() const;

    /// @brief Sets the vacancy loop bias factor.
    /// @param val Vacancy loop bias factor.
    void set_v_loop_bias(const gp_float val);

    /// @brief Returns the vacancy dislocation bias factor.
    gp_float get_v_dislocation_bias() const;

    /// @brief Sets the vacancy dislocation bias factor.
    /// @param val Dislocation bias factor.
    void set_v_dislocation_bias(const gp_float val);

    /// @brief Returns the vacancy dislocation bias parameter.
    gp_float get_v_dislocation_bias_param() const;

    /// @brief Sets the vacancy dislocation bias parameter.
    /// @param val Vacancy dislocation bias parameter.
    void set_v_dislocation_bias_param(const gp_float val);

    /// @brief Returns the initial dislocation network density in cm^-2.
    gp_float get_dislocation_density_0() const;

    /// @brief Sets the initial dislocation network density.
    /// @param val Initial dislocation network density in cm^-2.
    void set_dislocation_density_0(const gp_float val);

    /// @brief Returns the grain size in cm.
    gp_float get_grain_size() const;

    /// @brief Sets the grain size.
    /// @param val Grain size in cm.
    void set_grain_size(const gp_float val);

    /// @brief Returns the lattice parameter in cm.
    gp_float get_lattice_param() const;

    /// @brief Sets the lattice parameter.
    /// @param val Lattice parameter in cm.
    void set_lattice_param(const gp_float val);

    /// @brief Returns the magnitude of the burgers vector.
    gp_float get_burgers_vector() const;

    /// @brief Sets the magnitude of the burgers vector.
    /// @param val Magnitude of the burgers vector.
    void set_burgers_vector(const gp_float val);

    /// @brief Returns the average volume of a single atom in the material
    /// lattice in cm^3.
    gp_float get_atomic_volume() const;

    /// @brief Sets the average volume of a single atom in the material lattice.
    /// @param val Atomic volume in cm^3.
    void set_atomic_volume(const gp_float val);

    MaterialImpl *impl();

    int sqlite_id;
    std::string creation_datetime;  //!< Timestamp of when this Material was
                                    //!< first instantiated.
    std::string
        species;  //!< A name for what kind of material this data represents.

    std::unique_ptr<MaterialImpl> _impl;
};

// --------------------------------------------------------------------------------------------
// LATTICE PARAMETERS
namespace lattice_params {
const gp_float chromium = PM_CM_CONV(291.);
const gp_float nickel = PM_CM_CONV(352.4);
const gp_float fcc_nickel = PM_CM_CONV(360.);
const gp_float carbon = PM_CM_CONV(246.4);
}  // namespace lattice_params
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
// MATERIALS
namespace materials {

/** @brief A function which fills a Material object with parameters that roughly
 * correspond to the properties of SA304 steel.
 *  @param reactor A reference to the Material object to be populated with data.
 */
void SA304(Material &material);

}  // namespace materials

#endif  // MATERIAL_HPP
