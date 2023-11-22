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
        case 4:
            concentration_boundary = strtoul(argv[3], NULL, 10);
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
    fprintf(stderr, "Simulation Started: ");
    fprintf(stderr, "delta_time: %g, ", delta_time);
    fprintf(stderr, "simulation_time: %g, ", simulation_time);
    fprintf(stderr, "concentration_boundary: %lu", concentration_boundary);
}

int main(int argc, char* argv[])
{
    reactor = nuclear_reactors::OSIRIS();
    material = materials::SA304();

    init_globals(argc, argv);

    // initial dislocation density
    dislocation_density = material.dislocation_density_initial;

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
        for (uint64_t n = 1; n < concentration_boundary && valid_sim; ++n)
        {
            #if VPRINT
            fprintf(stdout, "\n------------------------------------------------------------------------------- t = %lu\tn = %lu\n", t, n);
            #endif

            interstitials_temp[n] += i_clusters_delta(n);
            vacancies_temp[n] += v_clusters_delta(n);

            if (!(valid_sim = validate(n)))
            {
                fprintf(stderr, "\nINVALID SIM @ t = %g\tn = %lu\n\n", t, n);
            }

            #if CSV
                #ifdef N
                fprintf(stdout, "%lu,%lu,%15.15lf,%15.15lf\n", t, (uint64_t)N, interstitials_temp[N], vacancies_temp[N]);
                #else
                fprintf(stdout, "%lu,%lu,%15.15lf,%15.15lf\n", t, n, interstitials_temp[n], vacancies_temp[n]);
                #endif
            #endif
        }

        if (valid_sim)
        {
            interstitials = interstitials_temp;
            vacancies = vacancies_temp;
            dislocation_density += dislocation_density_delta();
        }
    }
    // --------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------
    // print results
    #if !VPRINT && !CSV
    fprintf(stdout, "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n");
    for (uint64_t n = 1; n < concentration_boundary; ++n)
    {
        fprintf(stdout, "%lu\t\t\t\t\t%g\t\t\t\t%g\n\n", n, interstitials[n], vacancies[n]);
    }
    #endif
    // --------------------------------------------------------------------------------------------


    return 0;
}

bool validate(uint64_t n)
{
    return !std::isnan(interstitials_temp[n]) &&
    !std::isinf(interstitials_temp[n]) &&
    !std::isnan(vacancies_temp[n]) &&
    !std::isinf(vacancies_temp[n]);
}
