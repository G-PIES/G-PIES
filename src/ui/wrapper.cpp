// Need to have pybind11 installed with "pip install pybind11"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>
#include <cstring>
#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"
#include "material.hpp"
#include "nuclear_reactor.hpp"

#include "timer.hpp"

namespace py = pybind11;

// C++ wrapper file for the Python implementation of cluster dyanamics, based off of the example C++ cluster dynamics files.
struct Simulation {
    Simulation(size_t concentration_boundary, double simulation_time, double delta_time) 
    : concentration_boundary(concentration_boundary), simulation_time(simulation_time), delta_time(delta_time)
    {
        nuclear_reactors::OSIRIS(reactor);
        materials::SA304(material);
        cd = std::make_unique<ClusterDynamics>(concentration_boundary, reactor, material);
    }

    void run() 
    {
        state = (*cd).run(delta_time, sample_interval);
        if (!state.valid) 
        {
            printf("State invalid!");
        }
    }

    void print_state()
    {
        if (!state.valid) 
        {
            fprintf(stdout, "\nINVALID SIM @ Time=%g", state.time);
            exit(0);
        }
        else fprintf(stdout, "\nTime=%g", state.time);
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

    Material material;
    NuclearReactor reactor;
    ClusterDynamicsState state;
};

// Binds the current backend Cluster Dynamics code with these wrapper functions
PYBIND11_MODULE(pyclusterdynamics, m) {
    // Description
    m.doc() = "Cluster Dynamics C++ backend interface for python3.";
    // The name of the python class object is Simulation
    py::class_<Simulation>(m, "Simulation")
        .def(py::init<size_t, double, double>())
        .def("run", &Simulation::run) 
        .def("print_state", &Simulation::print_state)
        .def("get_state_time", &Simulation::get_state_time)
        .def("get_int_idx", &Simulation::get_int_idx) // vacancies
        .def("get_vac_idx", &Simulation::get_vac_idx); // interstials
}