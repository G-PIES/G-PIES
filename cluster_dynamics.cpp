#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"
#include "nuclear_reactor.hpp"
#include "material.hpp"

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of interstital defects from the irradiation cascade for size (n) clusters.
*/
double i_defect_production(uint64_t n)
{
    switch (n)
    {
        case 1: return reactor.recombination * reactor.flux *
                       (1. - reactor.i_bi - reactor.i_tri - reactor.i_quad);
        case 2: return reactor.recombination * reactor.flux * reactor.i_bi;
        case 3: return reactor.recombination * reactor.flux * reactor.i_tri;
        case 4: return reactor.recombination * reactor.flux * reactor.i_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.;
};

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of vacancy defects from the irradiation cascade for size (n) clusters.
*/
double v_defect_production(uint64_t n)
{
    switch (n)
    {
        case 1: return reactor.recombination * reactor.flux *
                       (1. - reactor.v_bi - reactor.v_tri - reactor.v_quad);
        case 2: return reactor.recombination * reactor.flux * reactor.v_bi;
        case 3: return reactor.recombination * reactor.flux * reactor.v_tri;
        case 4: return reactor.recombination * reactor.flux * reactor.v_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.;
};
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (in) interstitials per unit volume.

                  (1)     (2)                    (3)              (4)
    dCi(n) / dt = Gi(n) + a[i,n+1] * Ci(n + 1) - b[i,n] * Ci(n) + c[i,n-1] * Ci(n-1)
*/
double i_clusters_delta(uint64_t in)
{
    #if VPRINT
    double g = i_defect_production(in);
    double np1 = iemission_vabsorption_np1(in + 1);
    double n = iemission_vabsorption_n(in);
    double nm1 = in == 1 ? 0. : iemission_vabsorption_nm1(in - 1);
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "i_defect_production(%llu)%.*s%g +\n", (unsigned long long) in, 12, TABS, g);
    fprintf(stderr, "iemission_vabsorption_np1(%llu) * interstitials[%llu]%.*s%g * %g -\n", (unsigned long long) in + 1, (unsigned long long) in + 1, 9, TABS, np1, interstitials[in + 1]);
    fprintf(stderr, "iemission_vabsorption_n(%llu) * interstitials[%llu]%.*s%g * %g", (unsigned long long) in, (unsigned long long) in, 9, TABS, n, interstitials[in]);
    if (in > 1) fprintf(stderr, " +\niemission_vabsorption_nm1(%llu) * interstitials[%llu]%.*s%g * %g",  (unsigned long long) in - 1, (unsigned long long) in - 1, 9, TABS, nm1, interstitials[in - 1]);
    fprintf(stderr, "\n");
    return print_return(g + np1 * interstitials[in + 1] - n * interstitials[in] + (in == 1 ? 0. : nm1 * interstitials[in - 1]));
    #else

    return
        // (1)
        i_defect_production(in) +
        // (2)
        iemission_vabsorption_np1(in + 1) * interstitials[in + 1] -
        // (3)
        iemission_vabsorption_n(in) * interstitials[in] +
        // (4)
        (in == 1 ? 0. : iemission_vabsorption_nm1(in - 1) * interstitials[in - 1]);
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (vn) vacancies per unit volume.

                  (1)     (2)                    (3)              (4)
    dCv(n) / dt = Gv(n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(n) + c[v,n-1] * Cv(n-1)
*/
double v_clusters_delta(uint64_t vn)
{
    #if VPRINT
    double g = v_defect_production(vn);
    double np1 = vemission_iabsorption_np1(vn + 1);
    double n = vemission_iabsorption_n(vn);
    double nm1 = vn == 1 ? 0. : vemission_iabsorption_nm1(vn - 1);
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "v_defect_production(%llu)%.*s%g +\n", (unsigned long long) vn, 12, TABS, g);
    fprintf(stderr, "vemission_iabsorption_np1(%llu) * vacancies[%llu]%.*s%g * %g -\n",  (unsigned long long) vn + 1, (unsigned long long) vn + 1, 9, TABS, np1, vacancies[vn + 1]);
    fprintf(stderr, "vemission_iabsorption_n(%llu) * vacancies[%llu]%.*s%g * %g", (unsigned long long) vn, (unsigned long long) vn, 9, TABS, n, vacancies[vn]);
    if (vn > 1) 
        fprintf(stderr, " +\nvemission_iabsorption_nm1(%llu) * vacancies[%llu]%.*s%g * %g", 
        (unsigned long long) vn - 1, (unsigned long long) vn - 1, 9, 
        TABS, nm1, vacancies[vn - 1]);
    fprintf(stderr, "\n");
    return print_return(g + np1 * vacancies[vn + 1] - n * vacancies[vn] + (vn == 1 ? 0. : nm1 * vacancies[vn - 1]));
    #else
    return
        // (1)
        v_defect_production(vn) +
        // (2)
        vemission_iabsorption_np1(vn + 1) * vacancies[vn + 1] -
        // (3)
        vemission_iabsorption_n(vn) * vacancies[vn] +
        // (4)
        (vn == 1 ? 0. : vemission_iabsorption_nm1(vn - 1) * vacancies[vn - 1]);
    #endif
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (np1),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[i,n+1] = B[i,v](n + 1) * Cv(1) + E[i,i](n + 1)
*/
double iemission_vabsorption_np1(uint64_t np1)
{
    #if VPRINT
    double iva = iv_absorption(np1);
    double vc1 = vacancies[1];
    double iie = ii_emission(np1);
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "iv_absorption(%llu)%.*s%g *\n", (unsigned long long) np1, 12, TABS, iva);
    fprintf(stderr, "v_clusters1(%llu)%.*s%g +\n", (unsigned long long) np1, 13, TABS, vc1);
    fprintf(stderr, "ii_emission(%llu)%.*s%g\n", (unsigned long long) np1, 13, TABS, iie);
    return print_return(iva * vc1 + iie);
    #else

    return
        // (1)
        iv_absorption(np1) *
        // (2)
        vacancies[1] + 
        // (3)
        ii_emission(np1);
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (np1),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[v,n+1] = B[v,i](n + 1) * Ci(1) + E[v,v](n + 1)
*/
double vemission_iabsorption_np1(uint64_t np1)
{
    #if VPRINT
    double via = vi_absorption(np1);
    double ic1 = interstitials[1];
    double vve = vv_emission(np1);
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "vi_absorption(%llu)%.*s%g *\n", (unsigned long long) np1, 12, TABS, via);
    fprintf(stderr, "i_clusters1(%llu)%.*s%g +\n", (unsigned long long) np1, 13, TABS, ic1);
    fprintf(stderr, "vv_emission(%llu)%.*s%g\n", (unsigned long long) np1, 13, TABS, vve);
    return print_return(via * ic1 + vve);
    #else
    return 
        // (1)
        vi_absorption(np1) * 
        // (2)
        interstitials[1] + 
        // (3)
        vv_emission(np1);
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2c
    The rate that a loop of size n can evolve toward a loop of size
    n + 1 absorbing an interstitial, or toward a loop of size
    n - 1 absorbing a vacancy or emitting an interstitial.

             (1)                 (2)                 (3)
    b[i,n] = B[i,v](n) * Cv(1) + B[i,i](n) * Ci(1) + E[i,i](n)
*/
double iemission_vabsorption_n(uint64_t n)
{
    #if VPRINT
    double iva = iv_absorption(n);
    double vc1 = vacancies[1];
    double iia = ii_absorption(n);
    double ic1 = interstitials[1];
    double iie = ii_emission(n);
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "iv_absorption(%llu) * v_clusters1(%llu)%.*s%g * %g +\n", (unsigned long long) n, (unsigned long long) n, 10, TABS, iva, vc1);
    fprintf(stderr, "ii_absorption(%llu) * i_clusters1(%llu)%.*s%g * %g +\n", (unsigned long long) n, (unsigned long long) n, 10, TABS, iia, ic1);
    fprintf(stderr, "ii_emission(%llu)%.*s%g\n", (unsigned long long) n, 13, TABS, iie);
    return print_return(iva * vc1 + iia * ic1 + iie);
    #else
    return 0;
        // (1)
        iv_absorption(n) * vacancies[1] + 
        // (2)
        ii_absorption(n) * interstitials[1] +
        // (3)
        ii_emission(n);
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2c
    The rate that a loop of size n can evolve toward a loop of size
    n + 1 absorbing an vacancy, or toward a loop of size
    n - 1 absorbing an interstitial or emitting a vacancy.

             (1)                 (2)                 (3)
    b[v,n] = B[v,i](n) * Ci(1) + B[v,v](n) * Cv(1) + E[v,v](n)
*/
double vemission_iabsorption_n(uint64_t n)
{
    #if VPRINT
    double via = vi_absorption(n);
    double ic1 = interstitials[1];
    double vva = vv_absorption(n);
    double vc1 = vacancies[1];
    double vve = vv_emission(n);
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "vi_absorption(%llu) * i_clusters1(%llu)%.*s%g * %g +\n", (unsigned long long) n, (unsigned long long) n, 10, TABS, via, ic1);
    fprintf(stderr, "vv_absorption(%llu) * v_clusters1(%llu)%.*s%g * %g +\n", (unsigned long long) n, (unsigned long long) n, 10, TABS, vva, vc1);
    fprintf(stderr, "vv_emission(%llu)%.*s%g\n", (unsigned long long) n, 13, TABS, vve);
    return print_return(via * ic1 + vva * vc1 + vve);
    #else
    return 
        // (1)
        vi_absorption(n) * interstitials[1] + 
        // (2)
        vv_absorption(n) * vacancies[1] +
        // (3)
        vv_emission(n);
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2d
    The rate that an interstitial loop of size n - 1 can evolve into a
    loop of size n by absorbing an interstitial.

               (1)           (2)
    c[i,n-1] = B[i,i](n-1) * Ci(1)
*/
double iemission_vabsorption_nm1(uint64_t nm1)
{
    #if VPRINT
    double iia = ii_absorption(nm1);
    double ic1 = interstitials[1];
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "ii_absorption(%llu)%.*s%g *\n", (unsigned long long) nm1, 12, TABS, iia);
    fprintf(stderr, "i_clusters1(%llu)%.*s%g\n", (unsigned long long) nm1, 13, TABS, ic1);
    return print_return(iia * ic1);
    #else
    return
        // (1)
        ii_absorption(nm1) *
        // (2)
        interstitials[1];
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2d
    The rate that a vacancy loop of size n - 1 can evolve into a
    loop of size n by absorbing a vacancy.

               (1)           (2)
    c[v,n-1] = B[v,v](n-1) * Cv(1)
*/
double vemission_iabsorption_nm1(uint64_t nm1)
{
    #if VPRINT
    double vva = vv_absorption(nm1);
    double vc1 = vacancies[1];
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "vv_absorption(%llu)%.*s%g *\n", (unsigned long long) nm1, 12, TABS, vva);
    fprintf(stderr, "v_clusters1(%llu)%.*s%g\n", (unsigned long long) nm1, 13, TABS, vc1);
    return print_return(vva * vc1);
    #else
    return
        // (1)
        vv_absorption(nm1) * 
        // (2)
        vacancies[1];
    #endif
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3a
    Point defects concentrations per unit volume given a cluster size (in).
    ** in is only used in characteristic time calculations which rely on cluster sizes up to (in).

                (1)     (2)
    dCi(1)/dt = Gi(1) - Riv * Ci(1) * Cv(1) - 
            (3)
            Ci(1) / (tAdi) - 
            (4)
            Ci(1) / (tAgb) -
            (5)
            Ci(1) / (tAi) +
            (6)
            1 / (tEi)
*/
double i1_cluster_delta(uint64_t in)
{
    return 
        // (1)
        i_defect_production(1) -
        // (2)
        annihilation_rate() * interstitials[1] * vacancies[1] -
        // (3)
        interstitials[1] * i_dislocation_annihilation_time() -
        // (4)
        interstitials[1] * i_grain_boundary_annihilation_time(in) -
        // (5)
        interstitials[1] * i_absorption_time(in) +
        // (6)
        i_emission_time(in);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3a
    Point defects concentrations per unit volume given a cluster size (vn).
    ** vn is only used in characteristic time calculations which rely on cluster sizes up to (vn).

            (1)     (2)
    Cv(1) = Gv(1) - Riv * Ci(1) * Cv(1) - 
            (3)
            Cv(1) / (tAdv) - 
            (4)
            Cv(1) / (tAgb) -
            (5)
            Cv(1) / (tAv) +
            (6)
            1 / (tEv)
*/
double v1_cluster_delta(uint64_t vn)
{
    return 
        // (1)
        v_defect_production(1) -
        // (2)
        annihilation_rate() * interstitials[1] * vacancies[1] -
        // (3)
        vacancies[1] * v_dislocation_annihilation_time() -
        // (4)
        vacancies[1] * v_grain_boundary_annihilation_time(vn) -
        // (5)
        vacancies[1] * v_absorption_time(vn) +
        // (6)
        v_emission_time(vn);
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting an interstitial by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                      (2)                     (3)
    tEi(n) = SUM ( E[i,i](n) * Ci(n) ) + 4 * E[i,i](2) * Ci(2) + B[i,v](2) * Cv(2) * Ci(2)
*/
double i_emission_time(uint64_t nmax)
{
    double time = 0.;
    for (uint64_t in = 2; in < nmax; ++in)
    {
          time +=
        //      // (1)
              ii_emission(in) * interstitials[in];
    }

    time +=
        // (2)
        4 * ii_emission(2) * interstitials[2] +
        // (3)
        iv_absorption(2) * vacancies[2] * interstitials[2];

    return time;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                           (2)                     (3)
    tEv(n) = SUM[n>0] ( E[v,v](n) * Cv(n) ) + 4 * E[v,v](2) * Cv(2) + B[v,i](2) * Cv(2) * Ci(2)
*/
double v_emission_time(uint64_t nmax)
{
    double time = 0.;
    for (uint64_t vn = 2; vn < nmax; ++vn)
    {
        time += 
            // (1)
            vv_emission(vn) * vacancies[vn];
    }

    time +=
        // (2)
        4 * vv_emission(2) * vacancies[2] +
        // (3)
        vi_absorption(2) * vacancies[2] * interstitials[2];

    return time;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing an interstitial by the population of interstital or vacancy
    clusters of size up to (nmax).

                        (1)                              (2)
    tAi(n) = SUM[n>0] ( B[i,i](n) * Ci(n) ) + SUM[n>1] ( B[v,i](n) * Cv(n) )
*/
double i_absorption_time(uint64_t nmax)
{
    double time = ii_absorption(1) * interstitials[1];
    for (uint64_t in = 1; in < nmax; ++in)
    {
        time +=
            // (1)
            ii_absorption(in) * interstitials[in] +
            // (2)
            vi_absorption(in) * vacancies[in];
    }

    return time;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                        (1)                              (2)
    tAv(n) = SUM[n>0] ( B[v,v](n) * Cv(n) ) + SUM[n>1] ( B[i,v](n) * Ci(n) )
*/
double v_absorption_time(uint64_t nmax)
{
    double time = vv_absorption(1) * vacancies[1];
    for (uint64_t vn = 1; vn < nmax; ++vn)
    {
        time +=
            // (1)
            vv_absorption(vn) * vacancies[vn] +
            // (2)
            iv_absorption(vn) * interstitials[vn];
    }

    return time;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3d
    Annihilation rate of vacancies and insterstitals.

          (1)      (2)         (3)
    Riv = 4 * PI * (Di + Dv) * riv
*/
double annihilation_rate()
{
    return 
        // (1)
        4 * M_PI * 
        // (2)
        (material.i_diffusion + material.v_diffusion) *
        // (3)
        material.recombination_radius;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of interstitials on dislocations.

           (1)   (2)    (3)
    tAdi = p  *  Di  *  Zi

    TODO: calculate dislocation density instead of using initial value
*/
double i_dislocation_annihilation_time()
{
    return
        // (1)
        dislocation_density *
        // (2)
        material.i_diffusion *
        // (3)
        material.i_dislocation_bias;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of vacancies on dislocations.

           (1)   (2)    (3)
    tAdv = p  *  Dv  *  Zv

    TODO: calculate dislocation density instead of using initial value
*/
double v_dislocation_annihilation_time()
{
    return
        // (1)
        dislocation_density *
        // (2)
        material.v_diffusion *
        // (3)
        material.v_dislocation_bias;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdi = 6 * Di * sqrt( p * Zi + SUM[n] ( B[i,i](n) * Ci(n) ) + SUM[n] ( B[v,i](n) * Cv(n) ) ) / d

    TODO: calculate dislocation density instead of using initial value
*/
double i_grain_boundary_annihilation_time(uint64_t in)
{
    return
        // (1)
        6 * material.i_diffusion *
        sqrt
        (
            // (2)
            dislocation_density * 
            material.i_dislocation_bias +
            // (3)
            ii_sum_absorption(in) +
            // (4)
            vi_sum_absorption(in)
        ) /
        // (5)
        material.grain_size;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdv = 6 * Di * sqrt( p * Zv + SUM[n] ( B[v,v](n) * Cv(n) ) + SUM[n] ( B[i,v](n) * Ci(n) ) ) / d

    TODO: calculate dislocation density instead of using initial value
*/
double v_grain_boundary_annihilation_time(uint64_t vn)
{
    return
        // (1)
        6 * material.v_diffusion *
        sqrt
        (
            // (2)
            dislocation_density *
            material.v_dislocation_bias +
            // (3)
            vv_sum_absorption(vn) +
            // (4)
            iv_sum_absorption(vn)
        ) /
        // (5)
        material.grain_size;
}

double ii_sum_absorption(uint64_t nmax)
{
    double emission = 0.;
    for (uint64_t vn = 1; vn < nmax; ++vn)
    {
        emission += ii_absorption(vn) * interstitials[vn];
    }

    return emission;
}

double iv_sum_absorption(uint64_t nmax)
{
    double emission = 0.;
    for (uint64_t n = 1; n < nmax; ++n)
    {
        emission += iv_absorption(n) * interstitials[n];
    }

    return emission;
}

double vv_sum_absorption(uint64_t nmax)
{
    double emission = 0.;
    for (uint64_t n = 1; n < nmax; ++n)
    {
        emission += vv_absorption(n) * vacancies[n];
    }

    return emission;
}

double vi_sum_absorption(uint64_t nmax)
{
    double emission = 0.;
    for (uint64_t n = 1; n < nmax; ++n)
    {
        emission += vi_absorption(n) * vacancies[n];
    }

    return emission;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4a
    Rate of emission of an interstitial by an interstital loop of size (n).
*/
double ii_emission(uint64_t n)
{
    #if VPRINT
    double pi2n = 2 * M_PI * cluster_radius(n);
    double ibf = i_bias_factor(n);
    double id = material.i_diffusion / material.atomic_volume;
    double ibe = i_binding_energy(n);
    double evexp =
        exp
        (
            -ibe /
            (BOLTZMANN_EV_KELVIN * reactor.temperature)
        );
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "2 * M_PI * %llu%.*s%g *\n", (unsigned long long) n, 13, TABS, pi2n);
    fprintf(stderr, "i_bias_factor(%llu)%.*s%g *\n", (unsigned long long) n, 12, TABS, ibf);
    fprintf(stderr, "material.i_diffusion / material.atomic_volume%.*s%g *\n", 10, TABS, id);
    fprintf(stderr, "-i_binding_energy(%llu)%.*s%g *\n", (unsigned long long) n, 12, TABS, -ibe);
    fprintf(stderr, "exp(-i_binding_energy(%llu) / (k * reactor.temperature))%.*s%g *\n", (unsigned long long) n, 8, TABS, evexp);
    return print_return(pi2n * ibf * id * evexp);
    #else

    return 
        2 * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        material.i_diffusion * exp(-material.i_migration / BOLTZMANN_EV_KELVIN * reactor.temperature)
        / material.atomic_volume *
        exp
        (
            -i_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor.temperature)
        );
    #endif
}


/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4b
    Rate of absorption of an interstitial by an interstital loop of size (n).
*/
double ii_absorption(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        material.i_diffusion;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4c
    Rate of absorption of an interstitial by a vacancy loop of size (n).
*/
double iv_absorption(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        material.v_diffusion;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4d
    Rate of emission of a vacancy by a vacancy loop of size (n).
*/
double vv_emission(uint64_t n)
{
    #if VPRINT
    double pi2n = 2 * M_PI; // * n;
    double vbf = v_bias_factor(n);
    double vd = material.v_diffusion;
    double vbe = v_binding_energy(n);
    double evexp =
        exp
        (
            -vbe /
            (BOLTZMANN_EV_KELVIN * reactor.temperature)
        );
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "2 * M_PI * %llu%.*s%g *\n", (unsigned long long) n, 13, TABS, pi2n);
    fprintf(stderr, "v_bias_factor(%llu)%.*s%g *\n", (unsigned long long) n, 12, TABS, vbf);
    fprintf(stderr, "material.v_diffusion%.*s%g *\n", 12, TABS, vd);
    fprintf(stderr, "-v_binding_energy(%llu)%.*s%g *\n", (unsigned long long) n, 12, TABS, -vbe);
    fprintf(stderr, "exp(-v_binding_energy(%llu) / (k * reactor.temperature))%.*s%g *\n", (unsigned long long) n, 8, TABS, evexp);
    return print_return(pi2n * vbf * vd * evexp);
    #else
    return 
        2 * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        (material.v_diffusion * exp(-material.v_migration / BOLTZMANN_EV_KELVIN * reactor.temperature)) *
        exp
        (
            -v_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor.temperature)
        );
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4e
    Rate of absorption of a vacancy by a vacancy loop of size (n).
*/
double vv_absorption(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        material.v_diffusion;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4f
    Rate of absorption of a vacancy by an interstitial loop of size (n).
*/
double vi_absorption(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        material.i_diffusion;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Interstitial bias factor.
*/
double i_bias_factor(uint64_t in)
{
    return 1.;

    #if VPRINT
    double idb = material.i_dislocation_bias;
    double llp = 
        sqrt
        (
                material.burgers_vector /
                (8 * M_PI * material.lattice_param)
        ) *
        material.i_loop_bias -
        material.i_dislocation_bias;
    double nexp = 
        1 /
        pow
        (
            in,
            material.i_dislocation_bias_param / 2
        );
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "material.i_dislocation_bias%.*s%g +\n", 11, TABS, idb);
    fprintf(stderr, "sqrt(burgers_vector / (8 * M_PI * material.lattice_param)) * material.i_loop_bias - material.i_dislocation_bias%.*s%g *\n", 1, TABS, llp);
    fprintf(stderr, "1 / pow(%llu, material.i_dislocation_bias_param / 2)%.*s%g\n", (unsigned long long) in, 8, TABS, nexp);
    return print_return(idb + llp * nexp);
    #else
    return 
        material.i_dislocation_bias +
        (
            sqrt
            (
                    material.burgers_vector /
                    (8 * M_PI * material.lattice_param)
            ) *
            material.i_loop_bias -
            material.i_dislocation_bias
        ) *
        1 /
        pow
        (
            in,
            material.i_dislocation_bias_param / 2
        );
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Vacancy bias factor.
*/
double v_bias_factor(uint64_t vn)
{
    return 1.;

    #if VPRINT
    double vdb = material.v_dislocation_bias;
    double llp = 
        sqrt
        (
                material.burgers_vector /
                (8 * M_PI * material.lattice_param)
        ) *
        material.v_loop_bias -
        material.v_dislocation_bias;
    double nexp = 
        1 /
        pow
        (
            vn,
            material.v_dislocation_bias_param / 2
        );
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "material.v_dislocation_bias%.*s%g +\n", 11, TABS, vdb);
    fprintf(stderr, "sqrt(burgers_vector / (8 * M_PI * material.lattice_param)) * material.v_loop_bias - material.v_dislocation_bias%.*s%g *\n", 1, TABS, llp);
    fprintf(stderr, "1 / pow(%llu, material.v_dislocation_bias_param / 2)%.*s%g\n", (unsigned long long) vn, 8, TABS, nexp);
    return print_return(vdb + llp * nexp);
    #else
    return 
        material.v_dislocation_bias +
        (
            sqrt
            (
                    material.burgers_vector /
                    (8 * M_PI * material.lattice_param)
            ) *
            material.v_loop_bias -
            material.v_dislocation_bias
        ) *
        1 /
        pow
        (
            vn,
            material.v_dislocation_bias_param / 2
        );
    #endif
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Interstitial binding energy.
*/
double i_binding_energy(uint64_t in)
{
    #if VPRINT
    double ifo = material.i_formation;
    double factor = (material.i_binding - material.i_formation) / (pow(2., .8) - 1);
    double npow = (pow(in, .8) - pow(in - 1., .8));
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "material.i_formation%.*s%g +\n", 12, TABS, ifo);
    fprintf(stderr, "(material.i_binding - material.i_formation) / (pow(2., .8) - 1)%.*s%g *\n", 6, TABS, factor);
    fprintf(stderr, "(pow(%llu, .8) - pow(%llu, .8))%.*s%g *\n", (unsigned long long) in, (unsigned long long) in - 1, 11, TABS, npow);
    return print_return(ifo + factor * npow);
    #else
    return
        material.i_formation +
        (material.i_binding - material.i_formation) / (pow(2., .8) - 1) *
        (pow(in, .8) - pow(in - 1., .8));
    #endif
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Vacancy binding energy.
*/
double v_binding_energy(uint64_t vn)
{
    #if VPRINT
    double vfo = material.v_formation;
    double factor = (material.v_binding - material.v_formation) / (pow(2., .8) - 1);
    double npow = (pow(vn, .8) - pow(vn - 1., .8));
    fprintf(stderr, "%s\t%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
    fprintf(stderr, "material.v_formation%.*s%g +\n", 12, TABS, vfo);
    fprintf(stderr, "(material.v_binding - material.v_formation) / (pow(2., .8) - 1)%.*s%g *\n", 6, TABS, factor);
    fprintf(stderr, "(pow(%llu, .8) - pow(%llu, .8))%.*s%g *\n", (unsigned long long) vn, (unsigned long long) vn - 1, 11, TABS, npow);
    return print_return(vfo + factor * npow);
    #else
    return
        material.v_formation +
        (material.v_binding - material.v_formation) / (pow(2., .8) - 1) *
        (pow(vn, .8) - pow(vn - 1., .8));
    #endif
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 8
*/
double dislocation_density_delta()
{
    return 
        -reactor.dislocation_density_evolution * 
        pow(material.burgers_vector, 2) *
        pow(dislocation_density, 3./2.);
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg. 346, 7.63
*/
double cluster_radius(uint64_t n)
{
    return pow(sqrt(3) * pow(material.lattice_param, 2) * (double)n / (4 * M_PI), .5);
}
// --------------------------------------------------------------------------------------------
