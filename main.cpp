#include <iostream>
#include <cstring>
#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"

// --------------------------------------------------------------------------------------------
//  GLOBALS
/*  Nuclear Reactor
    (1) Species Name
    (2) Flux (cm^2 / s)
    (3) Temperature (Kelvin)
    (4) Recombination in Cascades
    (5) Interstitials in Cascades
    (6) Vancacies in Cascades
*/
NuclearReactor OSIRIS =
{ 
    "OSIRIS", // (1)
    2.9e-7, // (2)
    330.0 + CELCIUS_KELVIN_CONV,  // (3)
    .3, // (4)
    // (5)
    .5, // bi
    .2, // tri
    .06, // quad
    // (6)
    .06, // bi
    .03, // tri
    .02  // quad
};

/*  Material
    (1) Species Name
    (2) Migration Energy (eV)
    (3) Diffusion Coefficients (cm^2 / s)
    (4) Formation Energy (eV)
    (5) Binding Energy (eV)
    (6) Recombination Radius (cm)
    (7) Interstitial Loop Bias
    (8) Interstitial Dislocation Bias
    (9) Vacancy Loop Bias
    (10) Vacancy Dislocation Bias
    (11) Initial Dislocation Density (cm^2)
    (12) Grain Size (cm)
*/
Material SA304 = { 
    "SA304", // (1)
    // (2)
    .45, // i
    1.35, // v
    // (3)
    1e-3, // i
    .6, // v
    // (4)
    4.1, // i
    1.7, // v
    // (5)
    .6,  // i
    .5,  // v
    .7e-7, // (6)
    63.0, // (7)
    // (8)
    .8, 
    1.1, // param
    33, // (9)
    // (10)
    .65, 
    1.0, // param
    1 / (10e10 * M_CM_CONV), // (11)
    4e-3 // (12)
};

uint64_t concentration_boundary;
uint64_t simulation_time;
uint64_t delta_time;

std::array<double, CONCENTRATION_BOUNDARY> interstitials;
std::array<double, CONCENTRATION_BOUNDARY> vacancies;
std::array<double, CONCENTRATION_BOUNDARY> interstitials_temp;
std::array<double, CONCENTRATION_BOUNDARY> vacancies_temp;
double dislocation_density;

NuclearReactor* reactor = &OSIRIS;
Material* material = &SA304;
// --------------------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    concentration_boundary = CONCENTRATION_BOUNDARY;
    simulation_time = SIMULATION_TIME;
    delta_time = DELTA_TIME;

    interstitials.fill(0.0);
    vacancies.fill(0.0);
    interstitials_temp.fill(0.0);
    vacancies_temp.fill(0.0);

    // initial dislocation density
    dislocation_density = material->dislocation_density_initial;

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

            interstitials_temp[n] += i_clusters(n);
            vacancies_temp[n] += v_clusters(n);

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