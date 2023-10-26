#include <iostream>

#include "cluster_dynamics.hpp"

#ifndef CONCENTRATION_BOUNDS
#define CONCENTRATION_BOUNDS 100 
#endif

// result arrays of interstitials and vacancies
double interstitials[CONCENTRATION_BOUNDS];
double vacancies[CONCENTRATION_BOUNDS];

// number of clusters of N interstitials (in) per unit volume
/* Pokor et al. 2004, 2a
                  (1)     (2)                    (3)              (4)
    dCi(n) / dt = Gi(n) + a[i,n+1] * Ci(n + 1) - b[i,n] * Ci(n) + c[i,n-1] * Ci(n-1)
*/
double i_clusters(int in, NuclearReactor& reactor, Material& material)
{
    // boundary
    if (in > CONCENTRATION_BOUNDS || in < 1) return 0;

    // if n + 1 has not yet been calculated
    if (interstitials[in + 1] < 0)
    {
        // recurse to the boundary
        interstitials[in + 1] = i_clusters(in + 1, reactor, material);
    }

    return
        // (1)
        reactor.i_defect_production(in) +
        // (2)
        iemission_vabsorption_np1(in + 1) * interstitials[in + 1] -
        // (3)
        iemission_vabsorption_n(in) * interstitials[in] +
        // (4)
        iemission_vabsorption_nm1(in - 1) * interstitials[in - 1];
}

// number of clusters of N vacancies (vn) per unit volume
/* Pokor et al. 2004, 2a
                  (1)     (2)                    (3)              (4)
    dCv(n) / dt = Gv(n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(n) + c[v,n-1] * Cv(n-1)
*/
double v_clusters(int vn, NuclearReactor& reactor, Material& material)
{
    // boundary
    if (vn > CONCENTRATION_BOUNDS || vn < 1) return 0;

    // if n + 1 has not yet been calculated
    if (vacancies[vn + 1] < 0)
    {
        // recurse to the boundary
        vacancies[vn + 1] = v_clusters(vn + 1, reactor, material);
    }

    return
        // (1)
        reactor.v_defect_production(vn) +
        // (2)
        iemission_vabsorption_np1(vn + 1) * vacancies[vn + 1] -
        // (3)
        iemission_vabsorption_n(vn) * vacancies[vn] +
        // (4)
        iemission_vabsorption_nm1(vn - 1) * vacancies[vn - 1];

}

int main(int argc, char* argv[])
{
    // initialize result arrays ----------------------------------------
    interstitials[0] = 0.f;
    vacancies[0] = 0.f;

    for (int i = 1; i < CONCENTRATION_BOUNDS; ++i)
    {
        interstitials[i] = -1.f;
        vacancies[i] = -1.f;
    }
    // -----------------------------------------------------------------

    // calculate interstitial / vacancy concentrations -----------------
    for (int n = 1; n < CONCENTRATION_BOUNDS; ++n)
    {
        interstitials[n] = i_clusters(n, OSIRIS, SA304);
        vacancies[n] = v_clusters(n, OSIRIS, SA304);
    }
    // -----------------------------------------------------------------

    // print results ---------------------------------------------------
    fprintf(stdout, "Cluster Size\t-\tInterstitials\t-\tVacancies\n\n");
    for (int n = 1; n < CONCENTRATION_BOUNDS; ++n)
    {
        fprintf(stdout, "%d\t\t\t%8.1f\t\t%8.1f\n\n", n, interstitials[n], vacancies[n]);
    }
    // -----------------------------------------------------------------

    return 0;
}