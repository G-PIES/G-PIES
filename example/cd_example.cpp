#include <iostream>
#include <cstring>
#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"
#include "material.hpp"
#include "nuclear_reactor.hpp"

#include "timer.hpp"

#ifndef VPRINT
#define VPRINT false
#endif

#ifndef VBREAK
#define VBREAK false
#endif

#ifndef CSV
#define CSV false
#endif

size_t concentration_boundary;
size_t max_spatial_dim;
double simulation_time;
double delta_time;
double delta_x;
double sample_interval; // How often (in seconds) to record the state

void print_start_message()
{
    fprintf(stderr, "\nSimulation Started: ");
    fprintf(stderr, "delta_time: %g, ", delta_time);
    fprintf(stderr, "delta_x: %g, ", delta_x);
    fprintf(stderr, "simulation_time: %g, ", simulation_time);
    fprintf(stderr, "concentration_boundary: %llu, ", (unsigned long long)concentration_boundary);
    fprintf(stderr, "max_spatial_dim: %llu\n", (unsigned long long)max_spatial_dim);
}

void print_state(ClusterDynamicsState& state)
{
    if (!state.valid) fprintf(stdout, "\nINVALID SIM @ Time=%g", state.time);
    else fprintf(stdout, "\nTime=%g", state.time);

    if (state.interstitials.size() != concentration_boundary + 1 || state.vacancies.size() != concentration_boundary + 1)
    {
        fprintf(stderr, "\nError: Output data is incorrect size.\n");
        return;
    }

    fprintf(stdout, "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n");
    for (uint64_t n = 1; n < concentration_boundary; ++n)
    {
        // Change the second index to view a specific spatial dimension.
        // Will need to update the output format here to view everything at once

        fprintf(stdout, "%llu\t\t\t\t\t%13g\t\t\t  %15g\n\n", (unsigned long long)n, state.interstitials[n][2], state.vacancies[n][2]);
    }

    for (int i = 0; i < max_spatial_dim; i++)
      fprintf(stderr, "\n%d Dislocation Network Density: %g\n\n", i, state.dislocation_density[i]);
}

void print_csv(ClusterDynamicsState& state)
{
    for (uint64_t n = 1; n < concentration_boundary; ++n)
    {
        fprintf(stdout, "%g,%llu,%g,%g\n", state.time, (unsigned long long) n, state.interstitials[n], state.vacancies[n]);
    }
}

void profile()
{
    std::vector<double> times;
    Timer timer;

    ClusterDynamicsState state;

    NuclearReactor reactor;
    nuclear_reactors::OSIRIS(reactor);

    Material material;
    materials::SA304(material);

    ClusterDynamics cd(10, 5, reactor, material);
    cd.run(1e-5, 1e-5, 1e-4);

    for (int n = 100; n < 400000; n += 10000)
    {
        fprintf(stderr, "N=%d\n", n);
        ClusterDynamics cd(n, 5, reactor, material);

        timer.Start();
        state = cd.run(1e-5, 1e-5, 1e-4);
        double time = timer.Stop();

        fprintf(stdout, "\n%g", time);
    }
}

int main(int argc, char* argv[])
{
    NuclearReactor reactor;
    nuclear_reactors::OSIRIS(reactor);

    Material material;
    materials::SA304(material);

    // Default values
    concentration_boundary = 10;
    max_spatial_dim = 5;
    simulation_time = 1.0;
    delta_time = 1e-5;
    delta_x = 1e-4;
    sample_interval = delta_time;

    // Override default values with CLI arguments
    switch (argc)
    {
        case 6:
            max_spatial_dim = strtod(argv[5], NULL);
        case 5:
            delta_x = strtod(argv[4], NULL);
        case 4:
            concentration_boundary = strtod(argv[3], NULL);
        case 3:
            simulation_time = strtod(argv[2], NULL);
        case 2:
            delta_time = strtod(argv[1], NULL);
        default:
            break;
    }

    ClusterDynamics cd(concentration_boundary, max_spatial_dim, reactor, material);

    print_start_message();

    #if CSV
    fprintf(stdout, "Time (s),Cluster Size,Interstitials / cm^3,Vacancies / cm^3\n");
    #endif

    Timer timer;
    ClusterDynamicsState state;

    // --------------------------------------------------------------------------------------------
    // main simulation loop
    for (double t = 0; t < simulation_time; t = state.time)
    {
        // run simulation for this time slice
        state = cd.run(delta_time, sample_interval, delta_x);

        #if VPRINT
            print_state(state);
        #elif CSV
            print_csv(state);
        #endif

        if (!state.valid)
        {
            break;
        }

        #if VBREAK
        fgetc(stdin);
        #endif
    }
    // --------------------------------------------------------------------------------------------

    // --------------------------------------------------------------------------------------------
    // print results
    #if !VPRINT && !CSV
    print_state(state);
    #endif
    // --------------------------------------------------------------------------------------------

    return 0;
}
