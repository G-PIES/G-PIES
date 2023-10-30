#include <iostream>
#include <stdlib.h> // For dynamic memory allocation

#include "cluster_dynamics.hpp"


// --------------------------------------------------------------------------------------------
// GLOBALS
NuclearReactor OSIRIS = { "OSIRIS", 2.9e-7, 330.f, .3f, .5f, .2f, .06f, .06f, .03f, .02f };
Material SA304 = { "SA304", .45f, 1.35f, 1e-3, .6f, 4.1f, 1.7f, .6f, .5f, .7f, 63.f, .8f, 1.1f, 33, .65f, 1.f, 1e10, 4e-3 };

int concentration_boundary;
int simulation_time;
int delta_time;

size_t* interstitials;
size_t* vacancies;

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
    interstitials = (size_t*)calloc(concentration_boundary, sizeof(size_t));
    vacancies = (size_t*)calloc(concentration_boundary, sizeof(size_t));

    // malloc() and calloc() return a value of NULL if the memory allocation failed. We need to
    // test for that.
    if (interstitials == NULL) {
        fprintf(stderr, "An error occurred when allocating memory for the interstitial array.\n");
        return 1;
    } else if (vacancies == NULL) {
        fprintf(stderr, "An error occurred when allocating memory for the vacancies array.\n");
        return 2;
    } else
        fprintf(stdout, "%lu Bytes of memory was successfully allocated for both the interstitial and vacancy arrays.\n", concentration_boundary * sizeof(size_t));


    // Using memset to initialize the rest or each array to 0
    // IMPORTANT: We need to offset the arrays by the size of the size_t datatype to
    // ensure we only overwrite the indices from 0 to concentration_boundary - 1.
    // As such, we are only setting (concentration_boundary) * sizeof(size_t)
    // bytes of memory. - Sean H.
    memset(interstitials, 0, (concentration_boundary) * sizeof(size_t));
    memset(vacancies, 0, (concentration_boundary) * sizeof(size_t));


    // --------------------------------------------------------------------------------------------
    // main simulation loop
    for (int t = 0; t < simulation_time; t += delta_time)
    {
        // calculate interstitial / vacancy concentrations for this time slice
        for (int n = 1; n < concentration_boundary; ++n)
        {
            interstitials[n] = (size_t)i_clusters(n);
            vacancies[n] = (size_t)v_clusters(n);
        }
    }
    // --------------------------------------------------------------------------------------------


    // --------------------------------------------------------------------------------------------
    // print results
    fprintf(stdout, "Cluster Size\t-\tInterstitials\t-\tVacancies\n\n");
    for (int n = 1; n < concentration_boundary; ++n)
    {
        fprintf(stdout, "%d\t\t\t%lu\t\t%lu\n\n", n, interstitials[n], vacancies[n]);
    }
    // --------------------------------------------------------------------------------------------


    // Free up the dynamically allocated arrays.
    free(interstitials);
    free(vacancies);


    return 0;
}