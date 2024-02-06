#ifndef NUCLEAR_REACTOR_HPP
#define NUCLEAR_REACTOR_HPP 

#include <string>

#include "conversions.hpp"
#include "datetime.hpp"
#include "nuclear_reactor_impl.hpp"

struct NuclearReactor
{
    NuclearReactor() : sqlite_id(-1) 
    {
        datetime::utc_now(creation_datetime);
        _impl = std::make_unique<NuclearReactorImpl>();
    }

    NuclearReactor(const NuclearReactor& other) :
        sqlite_id(other.sqlite_id),
        creation_datetime(other.creation_datetime),
        species(other.species)
    {
        _impl = std::make_unique<NuclearReactorImpl>(*other._impl.get());
    }

    NuclearReactor& operator=(const NuclearReactor & other)
    {
        if (this != &other) copy(other);

        return *this;
    }

    void copy(const NuclearReactor& other)
    {
        sqlite_id = other.sqlite_id;
        creation_datetime = other.creation_datetime;
        species = other.species;

        _impl.release();
        _impl = std::make_unique<NuclearReactorImpl>(*other._impl.get());
    }

    int sqlite_id;
    std::string creation_datetime;

    std::string species;

    // neutron flux inside of the nuclear reactor (cm^2 / s)
    gp_float get_flux() const
	{
		return _impl->flux;
	}

	void set_flux(const gp_float val)
	{
		_impl->flux = val;
	}


    // (Kelvin) 
    gp_float get_temperature() const
	{
		return _impl->temperature;
	}

	void set_temperature(const gp_float val)
	{
		_impl->temperature = val;
	}


    // recombination in the cascades
    gp_float get_recombination() const
	{
		return _impl->recombination;
	}

	void set_recombination(const gp_float val)
	{
		_impl->recombination = val;
	}
 

    // interstitials in the cascades
    gp_float get_i_bi() const
	{
		return _impl->i_bi;
	}

	void set_i_bi(const gp_float val)
	{
		_impl->i_bi = val;
	}

    gp_float get_i_tri() const
	{
		return _impl->i_tri;
	}

	void set_i_tri(const gp_float val)
	{
		_impl->i_tri = val;
	}

    gp_float get_i_quad() const
	{
		return _impl->i_quad;
	}

	void set_i_quad(const gp_float val)
	{
		_impl->i_quad = val;
	}


    // vacancies in the cascades
    gp_float get_v_bi() const
	{
		return _impl->v_bi;
	}

	void set_v_bi(const gp_float val)
	{
		_impl->v_bi = val;
	}

    gp_float get_v_tri() const
	{
		return _impl->v_tri;
	}

	void set_v_tri(const gp_float val)
	{
		_impl->v_tri = val;
	}

    gp_float get_v_quad() const
	{
		return _impl->v_quad;
	}

	void set_v_quad(const gp_float val)
	{
		_impl->v_quad = val;
	}


    // factor of dislocation density evolution
    gp_float get_dislocation_density_evolution() const
	{
		return _impl->dislocation_density_evolution;
	}

	void set_dislocation_density_evolution(const gp_float val)
	{
		_impl->dislocation_density_evolution = val;
	}


    NuclearReactorImpl* impl()
    {
        return _impl.get();
    }

    std::unique_ptr<NuclearReactorImpl> _impl;
};

// --------------------------------------------------------------------------------------------
// REACTOR DEFINITIONS
namespace nuclear_reactors
{

static void OSIRIS(NuclearReactor& reactor)
{
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

}

#endif // NUCLEAR_REACTOR_HPP