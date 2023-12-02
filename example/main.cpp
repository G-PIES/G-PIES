#include <iostream>
#include <cstring>
#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"
#include "material.hpp"
#include "nuclear_reactor.hpp"

double simulation_time;
double delta_time;
double sample_interval; // How often (in seconds) to record the state

void print_start_message()
{
    fprintf(stderr, "\nSimulation Started: ");
    fprintf(stderr, "delta_time: %g, ", delta_time);
    fprintf(stderr, "simulation_time: %g, ", simulation_time);
    fprintf(stderr, "concentration_boundary: %llu\n\n", (unsigned long long)CONCENTRATION_BOUNDARY);
}

void print_state(ClusterDynamicsState& state)
{
    if (!state.valid) fprintf(stdout, "\nINVALID SIM @ Time=%g", state.time);
    else fprintf(stdout, "\nTime=%g", state.time);

    fprintf(stdout, "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n");
    for (uint64_t n = 1; n < CONCENTRATION_BOUNDARY; ++n)
    {
        fprintf(stdout, "%llu\t\t\t\t\t%13g\t\t\t  %15g\n\n", (unsigned long long)n, state.interstitials[n], state.vacancies[n]);
    }
}

void print_csv(ClusterDynamicsState& state)
{
    for (uint64_t n = 1; n < CONCENTRATION_BOUNDARY; ++n)
    {
        fprintf(stdout, "%g,%llu,%g,%g\n", state.time, (unsigned long long) n, state.interstitials[n], state.vacancies[n]);
    }
}

int main(int argc, char* argv[])
{

    NuclearReactor reactor = nuclear_reactors::OSIRIS();
    Material material = materials::SA304();

    switch (argc)
    {
        case 3:
            simulation_time = strtod(argv[2], NULL);
        case 2:
            delta_time = strtod(argv[1], NULL);
        default:
            break;
    }

    sample_interval = delta_time;

    ClusterDynamics cd(CONCENTRATION_BOUNDARY, reactor, material);

    print_start_message();

    #if CSV
    fprintf(stdout, "Time (s),Cluster Size,Interstitials / cm^3,Vacancies / cm^3\n");
    #endif

    ClusterDynamicsState state;

    // --------------------------------------------------------------------------------------------
    // main simulation loop
    for (double t = 0; t < simulation_time; t += delta_time)
    {
        // run simulation for this time slice
        ClusterDynamicsState state = cd.Run(delta_time, sample_interval);

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
