#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <cmath>
#include <string>
#include <memory>

#include "constants.hpp"
#include "conversions.hpp"
#include "datetime.hpp"
#include "material_impl.hpp"

struct Material
{
    Material() : sqlite_id(-1) 
    {
        datetime::utc_now(creation_datetime);
        _impl = std::make_unique<MaterialImpl>();
    }

    Material(const Material& other) :
        sqlite_id(other.sqlite_id),
        creation_datetime(other.creation_datetime),
        species(other.species)
    {
        _impl = std::make_unique<MaterialImpl>(*other._impl.get());
    }

    Material& operator=(const Material& other)
    {
        if (this != &other) copy(other);

        return *this;
    }

    void copy(const Material& other)
    {
        sqlite_id = other.sqlite_id;
        creation_datetime = other.creation_datetime;
        species = other.species;

        _impl.release();
        _impl = std::make_unique<MaterialImpl>(*other._impl.get());
    }

    int sqlite_id;
    std::string creation_datetime;

    std::string species;

    // migration energy (eV)
    gp_float get_i_migration() const
	{
		return _impl->i_migration;
	}

	void set_i_migration(const gp_float val)
	{
		_impl->i_migration = val;
	}

    gp_float get_v_migration() const
	{
		return _impl->v_migration;
	}

	void set_v_migration(const gp_float val)
	{
		_impl->v_migration = val;
	}


    // diffusion coefficients (pre-exponential) (cm^2 / s)
    gp_float get_i_diffusion_0() const
	{
		return _impl->i_diffusion_0;
	}

	void set_i_diffusion_0(const gp_float val)
	{
		_impl->i_diffusion_0 = val;
	}

    gp_float get_v_diffusion_0() const
	{
		return _impl->v_diffusion_0;
	}

	void set_v_diffusion_0(const gp_float val)
	{
		_impl->v_diffusion_0 = val;
	}


    // formation energy (eV)
    gp_float get_i_formation() const
	{
		return _impl->i_formation;
	}

	void set_i_formation(const gp_float val)
	{
		_impl->i_formation = val;
	}

    gp_float get_v_formation() const
	{
		return _impl->v_formation;
	}

	void set_v_formation(const gp_float val)
	{
		_impl->v_formation = val;
	}


    // binding energy for bi-interstitials and bi-vacancies (eV)
    gp_float get_i_binding() const
	{
		return _impl->i_binding;
	}

	void set_i_binding(const gp_float val)
	{
		_impl->i_binding = val;
	}

    gp_float get_v_binding() const
	{
		return _impl->v_binding;
	}

	void set_v_binding(const gp_float val)
	{
		_impl->v_binding = val;
	}


    // recombination radius between defects (cm)
    gp_float get_recombination_radius() const
	{
		return _impl->recombination_radius;
	}

	void set_recombination_radius(const gp_float val)
	{
		_impl->recombination_radius = val;
	}


    // bias factor of the loops for interstitials
    gp_float get_i_loop_bias() const
	{
		return _impl->i_loop_bias;
	}

	void set_i_loop_bias(const gp_float val)
	{
		_impl->i_loop_bias = val;
	}


    // bias factor of dislocations for interstitial
    gp_float get_i_dislocation_bias() const
	{
		return _impl->i_dislocation_bias;
	}

	void set_i_dislocation_bias(const gp_float val)
	{
		_impl->i_dislocation_bias = val;
	}

    gp_float get_i_dislocation_bias_param() const
	{
		return _impl->i_dislocation_bias_param;
	}

	void set_i_dislocation_bias_param(const gp_float val)
	{
		_impl->i_dislocation_bias_param = val;
	}


    // bias factor of the loops for vacancies
    gp_float get_v_loop_bias() const
	{
		return _impl->v_loop_bias;
	}

	void set_v_loop_bias(const gp_float val)
	{
		_impl->v_loop_bias = val;
	}


    // bias factor of dislocations for vacancies 
    gp_float get_v_dislocation_bias() const
	{
		return _impl->v_dislocation_bias;
	}

	void set_v_dislocation_bias(const gp_float val)
	{
		_impl->v_dislocation_bias = val;
	}

    gp_float get_v_dislocation_bias_param() const
	{
		return _impl->v_dislocation_bias_param;
	}

	void set_v_dislocation_bias_param(const gp_float val)
	{
		_impl->v_dislocation_bias_param = val;
	}


    // (cm^2)
    gp_float get_dislocation_density_0() const
	{
		return _impl->dislocation_density_0;
	}

	void set_dislocation_density_0(const gp_float val)
	{
		_impl->dislocation_density_0 = val;
	}


    // grain size (cm)
    gp_float get_grain_size() const
	{
		return _impl->grain_size;
	}

	void set_grain_size(const gp_float val)
	{
		_impl->grain_size = val;
	}


    // lattice parameter (cm)
    gp_float get_lattice_param() const
	{
		return _impl->lattice_param;
	}

	void set_lattice_param(const gp_float val)
	{
		_impl->lattice_param = val;
	}


    // burgers vector magnitude
    gp_float get_burgers_vector() const
	{
		return _impl->burgers_vector;
	}

	void set_burgers_vector(const gp_float val)
	{
		_impl->burgers_vector = val;
	}


    // average volume of a single atom in the lattice
    gp_float get_atomic_volume() const
	{
		return _impl->atomic_volume;
	}

	void set_atomic_volume(const gp_float val)
	{
		_impl->atomic_volume = val;
	}


    MaterialImpl* impl()
    {
        return _impl.get();
    }

    std::unique_ptr<MaterialImpl> _impl;
};


// --------------------------------------------------------------------------------------------
// LATTICE PARAMETERS
namespace lattice_params
{
    const gp_float chromium = PM_CM_CONV(291.);
    const gp_float nickel = PM_CM_CONV(352.4);
    const gp_float fcc_nickel = PM_CM_CONV(360.);
    const gp_float carbon = PM_CM_CONV(246.4);
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
// MATERIALS 
namespace materials
{

static void SA304(Material& material) 
{
    gp_float lattice_param = lattice_params::fcc_nickel;

    material.species = "SA304";
    material._impl->i_migration = .45;  // eV
    material._impl->v_migration = 1.35; // eV
    material._impl->i_diffusion_0 = 1e-3; // cm^2/s
    material._impl->v_diffusion_0 = .6;   // cm^2/s
    material._impl->i_formation = 4.1;  // eV
    material._impl->v_formation = 1.7;  // eV
    material._impl->i_binding = .6;     // eV
    material._impl->v_binding = .5;     // eV
    material._impl->recombination_radius = .7e-7;  // cm
    material._impl->i_loop_bias = 63.;
    material._impl->i_dislocation_bias = .8; 
    material._impl->i_dislocation_bias_param = 1.1;
    material._impl->v_loop_bias = 33.;
    material._impl->v_dislocation_bias = .65; 
    material._impl->v_dislocation_bias_param = 1.;
    material._impl->dislocation_density_0 = 1. / (gp_float)M_CM_CONV(10e10);
    material._impl->grain_size = 4e-3;
    material._impl->lattice_param = lattice_param;                 //cm
    material._impl->burgers_vector = lattice_param / std::sqrt(2.);
    material._impl->atomic_volume = std::pow(lattice_param, 3) / 4.;    //cm^3
}

}


#endif // MATERIAL_HPP