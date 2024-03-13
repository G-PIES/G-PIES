// Need to have pybind11 installed with "pip install pybind11"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <array>

#include "cluster_dynamics/cluster_dynamics.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"

#include "timer.hpp"

namespace py = pybind11;

struct Sim_Reactor {
    Sim_Reactor()
    {
        nuclear_reactors::OSIRIS(reactor); //For now..
    }

    NuclearReactor get_reactor()
    {
        return reactor;
    }

    NuclearReactor reactor;
};


struct Sim_Material {
    Sim_Material()
    {
        materials::SA304(material); //For now..
    }

    Material get_material()
    {
        return material;
    }

    Material material;
};

// C++ wrapper file for the Python implementation of cluster dyanamics, based off of the example C++ cluster dynamics files.
struct Simulation {
    // Default Constructor
    Simulation(size_t concentration_boundary, double simulation_time, double delta_time) 
    : concentration_boundary(concentration_boundary), simulation_time(simulation_time), delta_time(delta_time)
    {
        main_sim_reactor = Sim_Reactor();
        main_sim_material = Sim_Material();
        cd = std::make_unique<ClusterDynamics>(concentration_boundary, main_sim_reactor.get_reactor(), main_sim_material.get_material());
    }

    // Overloaded Constructor
    Simulation(size_t concentration_boundary, double simulation_time, double delta_time, Sim_Material material, Sim_Reactor reactor) 
    : concentration_boundary(concentration_boundary), simulation_time(simulation_time), delta_time(delta_time)
    {
        main_sim_material = material;
        main_sim_reactor = reactor;
        cd = std::make_unique<ClusterDynamics>(concentration_boundary, reactor.get_reactor(), material.get_material());
    }

    void run() 
    {
        state = (*cd).run(delta_time, sample_interval);
    }

    void print_state()
    {
        fprintf(stdout, "\nTime=%g", state.time);
        if (state.interstitials.size() != concentration_boundary || state.vacancies.size() != concentration_boundary)
        {
            fprintf(stderr, "\nError: Output data is incorrect size.\n");
            return;
        }
        fprintf(stdout, "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n");
        for (uint64_t n = 1; n < concentration_boundary; ++n)
        {
            fprintf(stdout, "%llu\t\t\t\t\t%13g\t\t\t  %15g\n\n", (unsigned long long)n, state.interstitials[n], state.vacancies[n]);
        }
        fprintf(stderr, "\nDislocation Network Density: %g\n\n", state.dislocation_density);
    }

    std::string string_state()
    {
        std::stringstream str_state;
        std::string str_to_cat;
        
        str_state << "\nTime=" << state.time;
        if (state.interstitials.size() != concentration_boundary || state.vacancies.size() != concentration_boundary)
        {
            str_state << "\nError: Output data is incorrect size.\n";
            return str_state.str();
        }
        str_state << "\nCluster Size\t-\tInterstitials\t-\tVacancies\n\n";
        for (uint64_t n = 1; n < concentration_boundary; ++n)
        {
            str_state << (unsigned long long)n << "\t\t" << state.interstitials[n] << "\t\t" << state.vacancies[n] << "\n\n";
        }
        str_state << "\nDislocation Network Density: " << state.dislocation_density << "\n\n";
        
        return str_state.str();
    }

    double get_state_time() 
    {
        return state.time;
    }

    double get_int_idx(int i)
    {
        return state.interstitials[i];
    }

    double get_vac_idx(int i)
    {
        return state.vacancies[i];
    }

    size_t concentration_boundary;
    double simulation_time;
    double delta_time;
    double sample_interval = delta_time;
    
    std::unique_ptr<ClusterDynamics> cd;

    Sim_Reactor main_sim_reactor;
    Sim_Material main_sim_material;
    ClusterDynamicsState state;
};


// Binds the current backend Cluster Dynamics code with these wrapper functions
PYBIND11_MODULE(pyclusterdynamics, m) {
    // Description
    m.doc() = "Cluster Dynamics C++ backend interface for python3.";
    // The name of the python class object is Simulation
    py::class_<Simulation>(m, "Simulation")
        .def(py::init<size_t, double, double>())
        .def(py::init<size_t, double, double, Sim_Material, Sim_Reactor>())
        .def("run", &Simulation::run) 
        .def("print_state", &Simulation::print_state)
        .def("string_state", &Simulation::string_state)
        .def("get_state_time", &Simulation::get_state_time)
        .def("get_int_idx", &Simulation::get_int_idx) // vacancies
        .def("get_vac_idx", &Simulation::get_vac_idx); // interstials
    
    py::class_<Sim_Reactor>(m, "Sim_Reactor")
        .def(py::init<>());
    
    py::class_<Sim_Material>(m, "Sim_Material")
        .def(py::init<>());

}