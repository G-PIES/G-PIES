#include <iostream>
#include <cstring>
#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"
#include "nuclear_reactor.hpp"

// --------------------------------------------------------------------------------------------
//  GLOBALS
uint64_t concentration_boundary;
double simulation_time;
double delta_time;

std::array<double, CONCENTRATION_BOUNDARY> interstitials;
std::array<double, CONCENTRATION_BOUNDARY> vacancies;
std::array<double, CONCENTRATION_BOUNDARY> interstitials_temp;
std::array<double, CONCENTRATION_BOUNDARY> vacancies_temp;
double dislocation_density;

NuclearReactor reactor;
Material material;
// --------------------------------------------------------------------------------------------

void init_globals(int argc, char* argv[])
{
    concentration_boundary = CONCENTRATION_BOUNDARY;
    simulation_time = SIMULATION_TIME;
    delta_time = DELTA_TIME;

    switch (argc)
    {
        case 3:
            simulation_time = strtod(argv[2], NULL);
        case 2:
            delta_time = strtod(argv[1], NULL);
        default:
            break;
    }
    
    interstitials.fill(0.);
    vacancies.fill(0.);
    interstitials_temp.fill(0.);
    vacancies_temp.fill(0.);
}

void print_start_message()
{
    fprintf(stderr, "\nSimulation Started: ");
    fprintf(stderr, "delta_time: %g, ", delta_time);
    fprintf(stderr, "simulation_time: %g, ", simulation_time);
    fprintf(stderr, "concentration_boundary: %llu\n\n", (unsigned long long) concentration_boundary);
}

int main(int argc, char* argv[])
{
    reactor = nuclear_reactors::OSIRIS();
    material = materials::SA304();

    init_globals(argc, argv);

    // initial dislocation density
    dislocation_density = material.dislocation_density_0;

    print_start_message();

    #if CSV
    fprintf(stdout, "Time (s),Cluster Size,Interstitials / cm^3,Vacancies / cm^3\n");
    #endif

    bool valid_sim = true;
    // --------------------------------------------------------------------------------------------
    // main simulation loop
    for (double t = 0; t < simulation_time && valid_sim; t += delta_time)
    {
        // calculate interstitial / vacancy concentrations for this time slice

        interstitials_temp[1] += i1_cluster_delta(concentration_boundary - 1) * delta_time;
        vacancies_temp[1] += v1_cluster_delta(concentration_boundary - 1) * delta_time;

        valid_sim = validate(1, t);

        for (uint64_t n = 2; n < concentration_boundary && valid_sim; ++n)
        {
            #if VPRINT
            fprintf(stdout, "\n------------------------------------------------------------------------------- t = %g\tn = %llu\n", t, (unsigned long long) n);
            #endif

            interstitials_temp[n] += i_clusters_delta(n) * delta_time;
            vacancies_temp[n] += v_clusters_delta(n) * delta_time;

            valid_sim = validate(n, t);

            #if CSV
                #ifdef N
                fprintf(stdout, "%g,%llu,%g,%g\n", t, (uint64_t)N, interstitials_temp[N], vacancies_temp[N]);
                #else
                fprintf(stdout, "%g,%llu,%g,%g\n", t, n, interstitials_temp[n], vacancies_temp[n]);
                #endif
            #endif
        }

        if (valid_sim)
        {
            interstitials = interstitials_temp;
            vacancies = vacancies_temp;
            dislocation_density += dislocation_density_delta() * delta_time;
        }
    }
    // --------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------
    // print results
    #if !VPRINT && !CSV
    fprintf(stdout, "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n");
    for (uint64_t n = 1; n < concentration_boundary; ++n)
    {
        fprintf(stdout, "%llu\t\t\t\t\t%13g\t\t\t  %15g\n\n", (unsigned long long) n, interstitials[n], vacancies[n]);
    }
    #endif
    // --------------------------------------------------------------------------------------------


    return 0;
}

bool validate(uint64_t n, double t)
{
    if (
        !std::isnan(interstitials_temp[n]) &&
        !std::isinf(interstitials_temp[n]) &&
        !std::isnan(vacancies_temp[n]) &&
        !std::isinf(vacancies_temp[n]) &&
        !(interstitials_temp[n] < 0) &&
        !(vacancies_temp[n] < 0)
    )
    {
        return true;
    }
    {
        fprintf(stderr, "\nINVALID SIM @ t = %g\tn = %llu\n\n", t, (unsigned long long) n);
        return false;
    }
}
