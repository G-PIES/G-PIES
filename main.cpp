#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "cluster_dynamics.hpp"

#define DEBUG_PRINT

void debug_print();
void debug_print_iterations(const char* func_name, double (*func)(int));

// --------------------------------------------------------------------------------------------
// GLOBALS
NuclearReactor OSIRIS = { "OSIRIS", 2.9e-7, 330.f, .3f, .5f, .2f, .06f, .06f, .03f, .02f };
Material SA304 = { "SA304", .45f, 1.35f, 1e-3, .6f, 4.1f, 1.7f, .6f, .5f, .7e-7, 63.f, .8f, 1.1f, 33, .65f, 1.f, 1 / (10e10 * 1e4), 4e-3 };

int concentration_boundary;
int simulation_time;
int delta_time;

double interstitials[CONCENTRATION_BOUNDARY];
double vacancies[CONCENTRATION_BOUNDARY];

NuclearReactor* reactor = &OSIRIS;
Material* material = &SA304;
// --------------------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    // command-line argument parsing
    // Pass the value of the CONCENTRATION_BOUNDS via the command-line.
    // If no value was passed, we can simply rely on the previously
    // provided global CONCENTRATION_BOUNDS. If a value was provided, we can
    // update the global integer of concentration_bounds.
    if (argc > 1) concentration_boundary = atoi(argv[1]);
    else concentration_boundary = CONCENTRATION_BOUNDARY;

    if (argc > 2) simulation_time = atoi(argv[2]);
    else simulation_time = SIMULATION_TIME;

    if (argc > 3) delta_time = atoi(argv[4]);
    else delta_time = DELTA_TIME;

    // All of the values of the results arrays should be set to 0
    //interstitials = (double*)calloc(concentration_boundary, sizeof(double));
    //vacancies = (double*)calloc(concentration_boundary, sizeof(double));

    // malloc() and calloc() return a value of NULL if the memory allocation failed. We need to
    // test for that.
    /*
    if (interstitials == NULL) {
        fprintf(stderr, "An error occurred when allocating memory for the interstitial array.\n");
        return 1;
    } else if (vacancies == NULL) {
        fprintf(stderr, "An error occurred when allocating memory for the vacancies array.\n");
        return 2;
    } else
        fprintf(stdout, "%lu Bytes of memory was successfully allocated for both the interstitial and vacancy arrays.\n", concentration_boundary * sizeof(double));
    */


    // Using memset to initialize the rest or each array to 0
    // IMPORTANT: We need to offset the arrays by the size of the double datatype to
    // ensure we only overwrite the indices from 0 to concentration_boundary - 1.
    // As such, we are only setting (concentration_boundary) * sizeof(double)
    // bytes of memory. - Sean H.
    memset(interstitials, 0, concentration_boundary * sizeof(double));
    memset(vacancies, 0, concentration_boundary * sizeof(double));

    #ifdef DEBUG_PRINT
        debug_print();
        return 0;
    #endif

    // --------------------------------------------------------------------------------------------
    // main simulation loop
    for (int t = 0; t < simulation_time; t += delta_time)
    {
        // calculate interstitial / vacancy concentrations for this time slice
        for (int n = 1; n < concentration_boundary; ++n)
        {
            interstitials[n] = i_clusters(n);
            vacancies[n] = v_clusters(n);
        }
    }
    // --------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------
    // print results
    fprintf(stdout, "Cluster Size\t-\tInterstitials\t-\tVacancies\n\n");
    for (int n = 1; n < concentration_boundary; ++n)
    {
        fprintf(stdout, "%d\t\t\t%8.10f\t\t%8.10f\n\n", n, interstitials[n], vacancies[n]);
    }
    // --------------------------------------------------------------------------------------------


    // Free up the dynamically allocated arrays.
    //free(interstitials);
    //free(vacancies);


    return 0;
}

void debug_print()
{
    fprintf(stdout, "\n\n");

    debug_print_iterations("i_defect_production", i_defect_production);
    debug_print_iterations("v_defect_production", v_defect_production);

    for (int n = 0; n < concentration_boundary; ++n)
    {
        interstitials[n] = i_clusters(n);
        vacancies[n] = v_clusters(n);
        fprintf(stdout, "i_clusters(%d)\t%8.20lf\n", n, interstitials[n]);
        fprintf(stdout, "v_clusters(%d)\t%8.20lf\n", n, vacancies[n]);
        fprintf(stdout, "\n\n");
    }

    debug_print_iterations("iemission_vabsorption_np1", iemission_vabsorption_np1);
    debug_print_iterations("iemission_vabsorption_n", iemission_vabsorption_n);
    debug_print_iterations("iemission_vabsorption_nm1", iemission_vabsorption_nm1);

    debug_print_iterations("vemission_iabsorption_np1", vemission_iabsorption_np1);
    debug_print_iterations("vemission_iabsorption_n", vemission_iabsorption_n);
    debug_print_iterations("vemission_iabsorption_nm1", vemission_iabsorption_nm1);

    debug_print_iterations("i_clusters1", i_clusters1);
    debug_print_iterations("v_clusters1", v_clusters1);

    debug_print_iterations("i_emission_time", i_emission_time);
    debug_print_iterations("v_emission_time", v_emission_time);

    debug_print_iterations("i_absorption_time", i_absorption_time);
    debug_print_iterations("v_absorption_time", v_absorption_time);

    fprintf(stdout, "%s\t%8.20f\n", "annihilation_rate", annihilation_rate());
    fprintf(stdout, "\n\n");

    fprintf(stdout, "%s\t%8.20f\n", "i_dislocation_annihilation_time", i_dislocation_annihilation_time());
    fprintf(stdout, "%s\t%8.20f\n", "v_dislocation_annihilation_time", v_dislocation_annihilation_time());
    fprintf(stdout, "\n\n");

    debug_print_iterations("i_grain_boundary_annihilation_time", i_grain_boundary_annihilation_time);
    debug_print_iterations("v_grain_boundary_annihilation_time", v_grain_boundary_annihilation_time);
    debug_print_iterations("ii_sum_absorption", ii_sum_absorption);
    debug_print_iterations("iv_sum_absorption", iv_sum_absorption);
    debug_print_iterations("vv_sum_absorption", vv_sum_absorption);
    debug_print_iterations("vi_sum_absorption", vi_sum_absorption);

    debug_print_iterations("ii_emission", ii_emission);
    debug_print_iterations("ii_absorption", ii_absorption);
    debug_print_iterations("iv_absorption", iv_absorption);
    debug_print_iterations("vv_emission", vv_emission);
    debug_print_iterations("vv_absorption", vv_absorption);
    debug_print_iterations("vi_absorption", vi_absorption);

    debug_print_iterations("i_bias_factor", i_bias_factor);
    debug_print_iterations("v_bias_factor", v_bias_factor);

    debug_print_iterations("i_binding_energy", i_binding_energy);
    debug_print_iterations("v_binding_energy", v_binding_energy);
}

void debug_print_iterations(const char* func_name, double (*func)(int))
{
    for (int n = 0; n < concentration_boundary; ++n)
        fprintf(stdout, "%s(%d)\t%8.20f\n", func_name, n, func(n));
    fprintf(stdout, "\n\n");
}