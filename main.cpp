#include <iostream>
#include <stdlib.h> // For dynamic memory allocation

#include "cluster_dynamics.hpp"

#ifndef CONCENTRATION_BOUNDS
#define CONCENTRATION_BOUNDS 100 
#endif

// result arrays of interstitials and vacancies

// Dynamic memory allocation and command-line arguments can help
// optimize the simulation's memory usage. - Sean H.
double interstitials[CONCENTRATION_BOUNDS];
double vacancies[CONCENTRATION_BOUNDS];

int concentration_bounds;   // Setting an overall global variable to hold the concentration_bounds
                            // so that we aren't forces to pass the variable to every single function. - Sean H.

// number of clusters of N interstitials (in) per unit volume
/* Pokor et al. 2004, 2a
                  (1)     (2)                    (3)              (4)
    dCi(n) / dt = Gi(n) + a[i,n+1] * Ci(n + 1) - b[i,n] * Ci(n) + c[i,n-1] * Ci(n-1)
*/
// The interstitial and vacancies arrays that were allocated in the main() function
// can be passed by reference. In this case, we only need the interstitials. - Sean H.
double i_clusters(int in, double* inters, NuclearReactor& reactor, Material& material)
{
    // boundary
    if (in > concentration_bounds || in < 1) return 0;

    // if n + 1 has not yet been calculated
    if (inters[in + 1] < 0) {
        // Recurse down to the boundry base-case, see above.
        inters[in + 1] = i_clusters(in + 1, inters, reactor, material);
    }

    // Using the dynamically allocated array to store the interstitial values,
    // calculate the rates, and calculate the new quantity of interstitials. - Sean H.
    return
        // (1)
        reactor.i_defect_production(in) +
        // (2)
        iemission_vabsorption_np1(in + 1) * inters[in + 1] -
        // (3)
        iemission_vabsorption_n(in) * inters[in] +
        // (4)
        iemission_vabsorption_nm1(in - 1) * inters[in - 1];
    /*
    if (interstitials[in + 1] < 0)
    {
        // recurse to the boundary
        interstitials[in + 1] = i_clusters(in + 1, reactor, material);
    }
    */

   /*
    return
        // (1)
        reactor.i_defect_production(in) +
        // (2)
        iemission_vabsorption_np1(in + 1) * interstitials[in + 1] -
        // (3)
        iemission_vabsorption_n(in) * interstitials[in] +
        // (4)
        iemission_vabsorption_nm1(in - 1) * interstitials[in - 1];
    */
}

// number of clusters of N vacancies (vn) per unit volume
/* Pokor et al. 2004, 2a
                  (1)     (2)                    (3)              (4)
    dCv(n) / dt = Gv(n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(n) + c[v,n-1] * Cv(n-1)
*/
// The interstitial and vacancies arrays that were allocated in the main() function
// can be passed by reference. In this case, we only need the vacancies. - Sean H.
double v_clusters(int vn, double* vacans, NuclearReactor& reactor, Material& material)
{
    // boundary
    if (vn > concentration_bounds || vn < 1) return 0;

    // If n + 1 has not yet been calculated
    if (vacans[vn + 1] < 0) {
        // Recurse to the boundry and the base-case.
        vacans[vn + 1] = v_clusters(vn + 1, vacans, reactor, material);
    }

    return
        // (1)
        reactor.v_defect_production(vn) +
        // (2)
        iemission_vabsorption_np1(vn + 1) * vacans[vn + 1] -
        // (3)
        iemission_vabsorption_n(vn) * vacans[vn] +
        // (4)
        iemission_vabsorption_nm1(vn - 1) * vacans[vn - 1];


    // if n + 1 has not yet been calculated
    /*
    if (vacancies[vn + 1] < 0)
    {
        // recurse to the boundary
        vacancies[vn + 1] = v_clusters(vn + 1, reactor, material);
    }
    */

   /*
    return
        // (1)
        reactor.v_defect_production(vn) +
        // (2)
        iemission_vabsorption_np1(vn + 1) * vacancies[vn + 1] -
        // (3)
        iemission_vabsorption_n(vn) * vacancies[vn] +
        // (4)
        iemission_vabsorption_nm1(vn - 1) * vacancies[vn - 1];
    */

}

int main(int argc, char* argv[])
{
    // Pass the value of the CONCENTRATION_BOUNDS via the command-line.
    // If no value was passed, we can simply rely on the previously
    // provided global CONCENTRATION_BOUNDS. If a value was provided, we can
    // update the global integer of concentration_bounds.
    if (argc != 2) {
        concentration_bounds = CONCENTRATION_BOUNDS;
    } else {
        concentration_bounds = atoi(argv[1]);
    }

    // initialize result arrays ----------------------------------------

    // All of the values of the results arrays should be set to -1.f, except for the very
    // first elements, which must be set to 0.f
    double* interstitials_test = (double*)calloc(concentration_bounds, sizeof(double));
    double* vacancies_test = (double*)calloc(concentration_bounds, sizeof(double));

    // malloc() and calloc() return a value of NULL if the memory allocation failed. We need to
    // test for that.
    if (interstitials_test == NULL) {
        fprintf(stderr, "An error occurred when allocating memory for the interstitial array.\n");
        return 1;
    } else if (vacancies_test == NULL) {
        fprintf(stderr, "An error occurred when allocating memory for the vacancies array.\n");
        return 2;
    } else
        printf("$d Bytes of memory was successfully allocated for both the interstitial and vacancy arrays.", concentration_bounds * sizeof(double));

    //interstitials[0] = 0.f;
    //vacancies[0] = 0.f;

    interstitials_test[0] = 0.f;
    vacancies_test[0] = 0.f;

    for (int i = 1; i < concentration_bounds; ++i)
    {
        //interstitials[i] = -1.f;
        //vacancies[i] = -1.f;

        interstitials_test[i] = -1.f;
        vacancies_test[i] = -1.f;
    }
    // -----------------------------------------------------------------

    // calculate interstitial / vacancy concentrations -----------------
    for (int n = 1; n < concentration_bounds; ++n)
    {
        /*
        interstitials[n] = i_clusters(n, OSIRIS, SA304);
        vacancies[n] = v_clusters(n, OSIRIS, SA304);
        */
        interstitials_test[n] = i_clusters(n, interstitials_test, OSIRIS, SA304);
        vacancies_test[n] = v_clusters(n, vacancies_test, OSIRIS, SA304);
    }
    // -----------------------------------------------------------------

    // print results ---------------------------------------------------
    fprintf(stdout, "Cluster Size\t-\tInterstitials\t-\tVacancies\n\n");
    for (int n = 1; n < concentration_bounds; ++n)
    {
        fprintf(stdout, "%d\t\t\t%8.1f\t\t%8.1f\n\n", n, interstitials_test[n], vacancies_test[n]);
    }
    // -----------------------------------------------------------------

    // Free up the dynamically allocated arrays.
    free(interstitials_test);
    free(vacancies_test);

    return 0;
}