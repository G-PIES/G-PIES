#include <iostream>
#include <cstring>
#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"
#include "nuclear_reactor.hpp"

// --------------------------------------------------------------------------------------------
//  GLOBALS
uint64_t concentration_boundary;
uint64_t simulation_time;
uint64_t delta_time;

std::array<double, CONCENTRATION_BOUNDARY> interstitials;
std::array<double, CONCENTRATION_BOUNDARY> vacancies;
std::array<double, CONCENTRATION_BOUNDARY> interstitials_temp;
std::array<double, CONCENTRATION_BOUNDARY> vacancies_temp;
double dislocation_density;

NuclearReactor reactor;
Material material;
// --------------------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    reactor = nuclear_reactors::OSIRIS();
    material = materials::SA304();

    concentration_boundary = CONCENTRATION_BOUNDARY;
    simulation_time = SIMULATION_TIME;
    delta_time = DELTA_TIME;

    interstitials.fill(0.);
    vacancies.fill(0.);
    interstitials_temp.fill(0.);
    vacancies_temp.fill(0.);

    // initial dislocation density
    dislocation_density = material.dislocation_density_initial;

    #if CSV
    fprintf(stdout, "Time (s),Cluster Size,Interstitials / cm^3,Vacancies / cm^3\n");
    #endif

    bool valid_sim = true;
    // --------------------------------------------------------------------------------------------
    // main simulation loop
    for (uint64_t t = 0; t < simulation_time && valid_sim; t += delta_time)
    {
        // calculate interstitial / vacancy concentrations for this time slice
        for (uint64_t n = 1; n < concentration_boundary && valid_sim; ++n)
        {
            #if VPRINT
            fprintf(stdout, "\n------------------------------------------------------------------------------- t = %llu\tn = %llu\n", t, n);
            #endif

            interstitials_temp[n] += i_clusters_delta(n);
            vacancies_temp[n] += v_clusters_delta(n);

            if (!(valid_sim = validate(n)))
            {
                fprintf(stdout, "\nINVALID SIM @ t = %llu\tn = %llu\n\n", t, n);
            }

            #if CSV
                #ifdef N
                fprintf(stdout, "%llu,%llu,%15.15lf,%15.15lf\n", t, (uint64_t)N, interstitials_temp[N], vacancies_temp[N]);
                #else
                fprintf(stdout, "%llu,%llu,%15.15lf,%15.15lf\n", t, n, interstitials_temp[n], vacancies_temp[n]);
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
        fprintf(stdout, "%llu\t\t\t\t\t%15.15lf\t\t\t%15.15lf\n\n", n, interstitials[n], vacancies[n]);
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