#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"

// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of interstital defects from the irradiation cascade for size (n) clusters.
*/
double i_defect_production(int n)
{
    switch (n)
    {
        case 1: return reactor->recombination * reactor->flux *
                       (1.f - reactor->i_bi - reactor->i_tri - reactor->i_quad);
        case 2: return reactor->recombination * reactor->flux * reactor->i_bi;
        case 3: return reactor->recombination * reactor->flux * reactor->i_tri;
        case 4: return reactor->recombination * reactor->flux * reactor->i_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.f;
};

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of vacancy defects from the irradiation cascade for size (n) clusters.
*/
double v_defect_production(int n)
{
    switch (n)
    {
        case 1: return reactor->recombination * reactor->flux *
                       (1.f - reactor->v_bi - reactor->v_tri - reactor->v_quad);
        case 2: return reactor->recombination * reactor->flux * reactor->v_bi;
        case 3: return reactor->recombination * reactor->flux * reactor->v_tri;
        case 4: return reactor->recombination * reactor->flux * reactor->v_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.f;
};
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (in) interstitials per unit volume.

                  (1)     (2)                    (3)              (4)
    dCi(n) / dt = Gi(n) + a[i,n+1] * Ci(n + 1) - b[i,n] * Ci(n) + c[i,n-1] * Ci(n-1)
*/
double i_clusters(int in)
{
    #if VPRINT
    double g = i_defect_production(in);
    double np1 = iemission_vabsorption_np1(in + 1);
    double n = iemission_vabsorption_n(in);
    double nm1 = in == 0 ? 0.f : iemission_vabsorption_nm1(in - 1);
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "i_defect_production(%d)%.*s%8.20f +\n", in, 12, TABS, g);
    fprintf(stdout, "iemission_vabsorption_np1(%d) * interstitials[%d]%.*s%8.20f * %8.20f -\n", in + 1, in + 1, 9, TABS, np1, interstitials[in + 1]);
    fprintf(stdout, "iemission_vabsorption_n(%d) * interstitials[%d]%.*s%8.20f * %8.20f", in, in, 9, TABS, n, interstitials[in]);
    if (in > 1) fprintf(stdout, " +\niemission_vabsorption_nm1(%d) * interstitials[%d]%.*s%8.20f * %8.20f",  in - 1, in - 1, 9, TABS, nm1, interstitials[in - 1]);
    fprintf(stdout, "\n");
    return print_return(g + np1 * interstitials[in + 1] - n * interstitials[in] + in == 1 ? 0.f : nm1 * interstitials[in - 1]);
    #else
    return
        // (1)
        i_defect_production(in) +
        // (2)
        iemission_vabsorption_np1(in + 1) * interstitials[in + 1] -
        // (3)
        iemission_vabsorption_n(in) * interstitials[in] +
        in == 1 ? 0.f :
        // (4)
        iemission_vabsorption_nm1(in - 1) * interstitials[in - 1];
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (vn) vacancies per unit volume.

                  (1)     (2)                    (3)              (4)
    dCv(n) / dt = Gv(n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(n) + c[v,n-1] * Cv(n-1)
*/
double v_clusters(int vn)
{
    #if VPRINT
    double g = v_defect_production(vn);
    double np1 = vemission_iabsorption_np1(vn + 1);
    double n = vemission_iabsorption_n(vn);
    double nm1 = vn == 0 ? 0.f : vemission_iabsorption_nm1(vn - 1);
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "v_defect_production(%d)%.*s%8.20f +\n", vn, 12, TABS, g);
    fprintf(stdout, "vemission_iabsorption_np1(%d) * vacancies[%d]%.*s%8.20f * %8.20f -\n",  vn + 1, vn + 1, 9, TABS, np1, vacancies[vn + 1]);
    fprintf(stdout, "vemission_iabsorption_n(%d) * vacancies[%d]%.*s%8.20f * %8.20f", vn, vn, 9, TABS, n, vacancies[vn]);
    if (vn > 1) fprintf(stdout, " +\nvemission_iabsorption_nm1(%d) * vacancies[%d]%.*s%8.20f * %8.20f", vn - 1, vn - 1, 9, TABS, nm1, vacancies[vn - 1]);
    return print_return(g + np1 * vacancies[vn + 1] - n * vacancies[vn] + vn == 1 ? 0.f : nm1 * vacancies[vn - 1]);
    #else
    return
        // (1)
        v_defect_production(vn) +
        // (2)
        vemission_iabsorption_np1(vn + 1) * vacancies[vn + 1] -
        // (3)
        vemission_iabsorption_n(vn) * vacancies[vn] +
        vn == 1 ? 0.f :
        // (4)
        vemission_iabsorption_nm1(vn - 1) * vacancies[vn - 1];
    #endif
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (np1),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[i,n+1] = B[i,v](n + 1) * Cv(1) + E[i,i](n + 1)
*/
double iemission_vabsorption_np1(int np1)
{
    #if VPRINT
    double iva = iv_absorption(np1);
    double vc1 = v_clusters1(np1);
    double iie = ii_emission(np1);
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "iv_absorption(%d)%.*s%8.20f *\n", np1, 12, TABS, iva);
    fprintf(stdout, "v_clusters1(%d)%.*s%8.20f +\n", np1, 13, TABS, vc1);
    fprintf(stdout, "ii_emission(%d)%.*s%8.20f\n", np1, 13, TABS, iie);
    return print_return(iva * vc1 + iie);
    #else
    return
        // (1)
        iv_absorption(np1) *
        // (2)
        v_clusters1(np1) + 
        // (3)
        ii_emission(np1);
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (np1),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[v,n+1] = B[v,i](n + 1) * Ci(1) + E[v,v](n + 1)
*/
double vemission_iabsorption_np1(int np1)
{
    #if VPRINT
    double via = vi_absorption(np1);
    double ic1 = i_clusters1(np1);
    double vve = vv_emission(np1);
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "vi_absorption(%d)%.*s%8.20f *\n", np1, 12, TABS, via);
    fprintf(stdout, "i_clusters1(%d)%.*s%8.20f +\n", np1, 13, TABS, ic1);
    fprintf(stdout, "vv_emission(%d)%.*s%8.20f\n", np1, 13, TABS, vve);
    return print_return(via * ic1 + vve);
    #else
    return 
        // (1)
        vi_absorption(np1) * 
        // (2)
        i_clusters1(np1) + 
        // (3)
        vv_emission(np1);
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2c
    The rate that a loop of size n can evolve toward a loop of size
    n + 1 absorbing an interstitial, or toward a loop of size
    n - 1 absorbing a vacancy or emitting an interstitial.

             (1)                 (2)                 (3)
    b[i,n] = B[i,v](n) * Cv(1) + B[i,i](n) * Ci(1) + E[i,i](n)
*/
double iemission_vabsorption_n(int n)
{
    #if VPRINT
    double iva = iv_absorption(n);
    double vc1 = v_clusters1(n);
    double iia = ii_absorption(n);
    double ic1 = i_clusters1(n);
    double iie = ii_emission(n);
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "iv_absorption(%d) * v_clusters1(%d)%.*s%8.20f * %8.20f +\n", n, n, 10, TABS, iva, vc1);
    fprintf(stdout, "ii_absorption(%d) * i_clusters1(%d)%.*s%8.20f * %8.20f +\n", n, n, 10, TABS, iia, ic1);
    fprintf(stdout, "ii_emission(%d)%.*s%8.20f\n", n, 13, TABS, iie);
    return print_return(iva * vc1 + iia * ic1 + iie);
    #else
    return 
        // (1)
        iv_absorption(n) * v_clusters1(n) + 
        // (2)
        ii_absorption(n) * i_clusters1(n) +
        // (3)
        ii_emission(n);
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2c
    The rate that a loop of size n can evolve toward a loop of size
    n + 1 absorbing an vacancy, or toward a loop of size
    n - 1 absorbing an interstitial or emitting a vacancy.

             (1)                 (2)                 (3)
    b[v,n] = B[v,i](n) * Ci(1) + B[v,v](n) * Cv(1) + E[v,v](n)
*/
double vemission_iabsorption_n(int n)
{
    #if VPRINT
    double via = vi_absorption(n);
    double ic1 = i_clusters1(n);
    double vva = vv_absorption(n);
    double vc1 = v_clusters1(n);
    double vve = vv_emission(n);
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "vi_absorption(%d) * i_clusters1(%d)%.*s%8.20f * %8.20f +\n", n, n, 10, TABS, via, ic1);
    fprintf(stdout, "vv_absorption(%d) * v_clusters1(%d)%.*s%8.20f * %8.20f +\n", n, n, 10, TABS, vva, vc1);
    fprintf(stdout, "vv_emission(%d)%.*s%8.20f\n", n, 13, TABS, vve);
    return print_return(via * ic1 + vva * vc1 + vve);
    #else
    return 
        // (1)
        vi_absorption(n) * i_clusters1(n) + 
        // (2)
        vv_absorption(n) * v_clusters1(n) +
        // (3)
        vv_emission(n);
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2d
    The rate that an interstitial loop of size n - 1 can evolve into a
    loop of size n by absorbing an interstitial.

               (1)           (2)
    c[i,n-1] = B[i,i](n-1) * Ci(1)
*/
double iemission_vabsorption_nm1(int nm1)
{
    #if VPRINT
    double iia = ii_absorption(nm1);
    double ic1 = i_clusters1(nm1);
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "ii_absorption(%d)%.*s%8.20f *\n", nm1, 12, TABS, iia);
    fprintf(stdout, "i_clusters1(%d)%.*s%8.20f\n", nm1, 13, TABS, ic1);
    return print_return(iia * ic1);
    #else
    return
        // (1)
        ii_absorption(nm1) *
        // (2)
        i_clusters1(nm1);
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2d
    The rate that a vacancy loop of size n - 1 can evolve into a
    loop of size n by absorbing a vacancy.

               (1)           (2)
    c[v,n-1] = B[v,v](n-1) * Cv(1)
*/
double vemission_iabsorption_nm1(int nm1)
{
    #if VPRINT
    double vva = vv_absorption(nm1);
    double vc1 = v_clusters1(nm1);
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "vv_absorption(%d)%.*s%8.20f *\n", nm1, 12, TABS, vva);
    fprintf(stdout, "v_clusters1(%d)%.*s%8.20f\n", nm1, 13, TABS, vc1);
    return print_return(vva * vc1);
    #else
    return
        // (1)
        vv_absorption(nm1) * 
        // (2)
        v_clusters1(nm1);
    #endif
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3a
    Point defects concentrations per unit volume given a cluster size (in).
    ** in is only used in characteristic time calculations which rely on cluster sizes up to (in).

            (1)     (2)
    Ci(1) = Gi(1) - Riv * Ci(1) * Cv(1) - 
            (3)
            Ci(1) / (tAdi) - 
            (4)
            Ci(1) / (tAgb) -
            (5)
            Ci(1) / (tAi) +
            (6)
            1 / (tEi)
*/
double i_clusters1(int in)
{
    return interstitials[1];
    return 
        // (1)
        i_defect_production(1) -
        // (2)
        annihilation_rate() * interstitials[1] * vacancies[1] -
        // (3)
        interstitials[1] * v_dislocation_annihilation_time() -
        // (4)
        interstitials[1] * v_grain_boundary_annihilation_time(in) -
        // (5)
        interstitials[1] * v_absorption_time(in) +
        // (6)
        i_emission_time(in);
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3a
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
double v_clusters1(int vn)
{
    return vacancies[1];
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
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting an interstitial by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                      (2)                     (3)
    tEi(n) = SUM ( E[i,i](n) * Ci(n) ) + 4 * E[i,i](2) * Ci(2) + B[i,v](2) * Cv(2) * Ci(2)
*/
double i_emission_time(int nmax)
{
    double time = 0.f;
    for (int in = 2; in < nmax; ++in)
    {
        time +=
            // (1)
            ii_emission(in) * interstitials[in];
    }

    time +=
        // (2)
        4 * ii_emission(2) * interstitials[2] +
        // (3)
        iv_absorption(2) * vacancies[2] * interstitials[2];

    return time;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                           (2)                     (3)
    tEv(n) = SUM[n>0] ( E[v,v](n) * Cv(n) ) + 4 * E[v,v](2) * Cv(2) + B[v,i](2) * Cv(2) * Ci(2)
*/
double v_emission_time(int nmax)
{
    double time = 0.f;
    for (int vn = 2; vn < nmax; ++vn)
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
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing an interstitial by the population of interstital or vacancy
    clusters of size up to (nmax).

                        (1)                              (2)
    tAi(n) = SUM[n>0] ( B[i,i](n) * Ci(n) ) + SUM[n>1] ( B[v,i](n) * Cv(n) )
*/
double i_absorption_time(int nmax)
{
    double time = ii_absorption(1) * interstitials[1];
    for (int in = 1; in < nmax; ++in)
    {
        time +=
            // (1)
            ii_absorption(in) * interstitials[in] +
            // (2)
            vi_absorption(in) * vacancies[in];
    }

    return time;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                        (1)                              (2)
    tAv(n) = SUM[n>0] ( B[v,v](n) * Cv(n) ) + SUM[n>1] ( B[i,v](n) * Ci(n) )
*/
double v_absorption_time(int nmax)
{
    double time = vv_absorption(1) * vacancies[1];
    for (int vn = 1; vn < nmax; ++vn)
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
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3d
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
        (material->i_diffusion + material->v_diffusion) *
        // (3)
        material->recombination_radius;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of interstitials on dislocations.

           (1)   (2)    (3)
    tAdi = p  *  Di  *  Zi

    TODO: calculate dislocation density instead of using initial value
*/
double i_dislocation_annihilation_time()
{
    return
        // (1)
        material->dislocation_density_initial *
        // (2)
        material->i_diffusion *
        // (3)
        material->i_dislocation_bias;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of vacancies on dislocations.

           (1)   (2)    (3)
    tAdv = p  *  Dv  *  Zv

    TODO: calculate dislocation density instead of using initial value
*/
double v_dislocation_annihilation_time()
{
    return
        // (1)
        material->dislocation_density_initial *
        // (2)
        material->v_diffusion *
        // (3)
        material->v_dislocation_bias;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdi = 6 * Di * sqrt( p * Zi + SUM[n] ( B[i,i](n) * Ci(n) ) + SUM[n] ( B[v,i](n) * Cv(n) ) ) / d

    TODO: calculate dislocation density instead of using initial value
*/
double i_grain_boundary_annihilation_time(int in)
{
    return
        // (1)
        6 * material->i_diffusion *
        sqrt
        (
            // (2)
            material->dislocation_density_initial * 
            material->i_dislocation_bias +
            // (3)
            ii_sum_absorption(in) +
            // (4)
            vi_sum_absorption(in)
        ) /
        // (5)
        material->grain_size;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdv = 6 * Di * sqrt( p * Zv + SUM[n] ( B[v,v](n) * Cv(n) ) + SUM[n] ( B[i,v](n) * Ci(n) ) ) / d

    TODO: calculate dislocation density instead of using initial value
*/
double v_grain_boundary_annihilation_time(int vn)
{
    return
        // (1)
        6 * material->v_diffusion *
        sqrt
        (
            // (2)
            material->dislocation_density_initial *
            material->v_dislocation_bias +
            // (3)
            vv_sum_absorption(vn) +
            // (4)
            iv_sum_absorption(vn)
        ) /
        // (5)
        material->grain_size;
}

double ii_sum_absorption(int nmax)
{
    double emission = 0.f;
    for (int vn = 1; vn < nmax; ++vn)
    {
        emission += ii_absorption(vn) * interstitials[vn];
    }

    return emission;
}

double iv_sum_absorption(int nmax)
{
    double emission = 0.f;
    for (int n = 1; n < nmax; ++n)
    {
        emission += iv_absorption(n) * interstitials[n];
    }

    return emission;
}

double vv_sum_absorption(int nmax)
{
    double emission = 0.f;
    for (int n = 1; n < nmax; ++n)
    {
        emission += vv_absorption(n) * vacancies[n];
    }

    return emission;
}

double vi_sum_absorption(int nmax)
{
    double emission = 0.f;
    for (int n = 1; n < nmax; ++n)
    {
        emission += vi_absorption(n) * vacancies[n];
    }

    return emission;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4a
    Rate of emission of an interstitial by an interstital loop of size (n).
*/
double ii_emission(int n)
{
    // TODO: average atomic volume of SA304: .7 m^3/kmol
    // converted to cm^3/kmol
    float atomic_volume = .7e6;
    #if VPRINT
    double pi2n = 2 * M_PI * n;
    double ibf = i_bias_factor(n);
    double id = material->i_diffusion / atomic_volume;
    double ibe = i_binding_energy(n);
    double evexp =
        exp
        (
            -ibe /
            (BOLTZMANN_EV_KELVIN * reactor->temperature)
        );
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "2 * M_PI * %d%.*s%8.20f *\n", n, 13, TABS, pi2n);
    fprintf(stdout, "i_bias_factor(%d)%.*s%8.20f *\n", n, 12, TABS, ibf);
    fprintf(stdout, "material->i_diffusion / atomic_volume%.*s%8.20f *\n", 10, TABS, id);
    fprintf(stdout, "-i_binding_energy(%d)%.*s%8.20f *\n", n, 12, TABS, -ibe);
    fprintf(stdout, "exp(-i_binding_energy(%d) / (k * reactor->temperature))%.*s%8.20f *\n", n, 8, TABS, evexp);
    return print_return(pi2n * ibf * id * evexp);
    #else
    return 
        2 * M_PI * n *
        i_bias_factor(n) *
        material->i_diffusion / atomic_volume *
        exp
        (
            -i_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor->temperature)
        );
    #endif
}


/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4b
    Rate of absorption of an interstitial by an interstital loop of size (n).
*/
double ii_absorption(int n)
{
    return 
        2 * M_PI * n *
        i_bias_factor(n) *
        material->i_diffusion;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4c
    Rate of absorption of an interstitial by a vacancy loop of size (n).
*/
double iv_absorption(int n)
{
    return 
        2 * M_PI * n *
        i_bias_factor(n) *
        material->v_diffusion;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4d
    Rate of emission of a vacancy by a vacancy loop of size (n).
*/
double vv_emission(int n)
{
    #if VPRINT
    double pi2n = 2 * M_PI * n;
    double vbf = v_bias_factor(n);
    double vd = material->v_diffusion;
    double vbe = v_binding_energy(n);
    double evexp =
        exp
        (
            -vbe /
            (BOLTZMANN_EV_KELVIN * reactor->temperature)
        );
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "2 * M_PI * %d%.*s%8.20f *\n", n, 13, TABS, pi2n);
    fprintf(stdout, "v_bias_factor(%d)%.*s%8.20f *\n", n, 12, TABS, vbf);
    fprintf(stdout, "material->v_diffusion%.*s%8.20f *\n", 12, TABS, vd);
    fprintf(stdout, "-v_binding_energy(%d)%.*s%8.20f *\n", n, 12, TABS, -vbe);
    fprintf(stdout, "exp(-v_binding_energy(%d) / (k * reactor->temperature))%.*s%8.20f *\n", n, 8, TABS, evexp);
    return print_return(pi2n * vbf * vd * evexp);
    #else
    return 
        2 * M_PI * n *
        v_bias_factor(n) *
        material->v_diffusion *
        exp
        (
            -v_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor->temperature)
        );
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4e
    Rate of absorption of a vacancy by a vacancy loop of size (n).
*/
double vv_absorption(int n)
{
    return 
        2 * M_PI * n *
        v_bias_factor(n) *
        material->v_diffusion;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4f
    Rate of absorption of a vacancy by an interstitial loop of size (n).
*/
double vi_absorption(int n)
{
    return 
        2 * M_PI * n *
        v_bias_factor(n) *
        material->i_diffusion;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 5
    Interstitial bias factor.
*/
double i_bias_factor(int in)
{
    // TODO
    // lattice parameters (picometeres)
    // Chromium: 291 pm
    // Nickel: 352.4 pm
    float lattice_param = 291.f * 1e-10;

    // TODO
    // Face-Centered Cubic Lattice (fcc) burgers vector magnitude
    float burgers_vector = lattice_param / pow(2, .5f);

    #if VPRINT
    double idb = material->i_dislocation_bias;
    double llp = 
        sqrt
        (
                burgers_vector /
                (8 * M_PI * lattice_param)
        ) *
        material->i_loop_bias -
        material->i_dislocation_bias;
    double nexp = 
        1 /
        pow
        (
            in,
            material->i_dislocation_bias_param / 2
        );
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "material->i_dislocation_bias%.*s%8.20f +\n", 11, TABS, idb);
    fprintf(stdout, "sqrt(burgers_vector / (8 * M_PI * lattice_param)) * material->i_loop_bias - material->i_dislocation_bias%.*s%8.20f *\n", 1, TABS, llp);
    fprintf(stdout, "1 / pow(%d, material->i_dislocation_bias_param / 2)%.*s%8.20f\n", in, 8, TABS, nexp);
    return print_return(idb + llp * nexp);
    #else
    return 
        material->i_dislocation_bias +
        (
            sqrt
            (
                    burgers_vector /
                    (8 * M_PI * lattice_param)
            ) *
            material->i_loop_bias -
            material->i_dislocation_bias
        ) *
        1 /
        pow
        (
            in,
            material->i_dislocation_bias_param / 2
        );
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 5
    Vacancy bias factor.
*/
double v_bias_factor(int vn)
{
    // TODO
    // lattice parameters (picometeres)
    // Chromium: 291 pm
    // Nickel: 352.4 pm
    float lattice_param = 291.f * 1e-10;

    // TODO
    // Face-Centered Cubic Lattice (fcc) burgers vector magnitude
    float burgers_vector = lattice_param / pow(2, .5f);

    #if VPRINT
    double vdb = material->v_dislocation_bias;
    double llp = 
        sqrt
        (
                burgers_vector /
                (8 * M_PI * lattice_param)
        ) *
        material->v_loop_bias -
        material->v_dislocation_bias;
    double nexp = 
        1 /
        pow
        (
            vn,
            material->v_dislocation_bias_param / 2
        );
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "material->v_dislocation_bias%.*s%8.20f +\n", 11, TABS, vdb);
    fprintf(stdout, "sqrt(burgers_vector / (8 * M_PI * lattice_param)) * material->v_loop_bias - material->v_dislocation_bias%.*s%8.20f *\n", 1, TABS, llp);
    fprintf(stdout, "1 / pow(%d, material->v_dislocation_bias_param / 2)%.*s%8.20f\n", vn, 8, TABS, nexp);
    return print_return(vdb + llp * nexp);
    #else
    return 
        material->v_dislocation_bias +
        (
            sqrt
            (
                    burgers_vector /
                    (8 * M_PI * lattice_param)
            ) *
            material->v_loop_bias -
            material->v_dislocation_bias
        ) *
        1 /
        pow
        (
            vn,
            material->v_dislocation_bias_param / 2
        );
    #endif
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 6
    Interstitial binding energy.
*/
double i_binding_energy(int in)
{
    #if VPRINT
    double ifo = material->i_formation;
    double factor = (material->i_binding - material->i_formation) / (pow(2.f, .8f) - 1);
    double npow = (pow(in, .8f) - pow(in - 1, .8f));
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "material->i_formation%.*s%8.20f +\n", 12, TABS, ifo);
    fprintf(stdout, "(material->i_binding - material->i_formation) / (pow(2.f, .8f) - 1)%.*s%8.20f *\n", 6, TABS, factor);
    fprintf(stdout, "(pow(%d, .8f) - pow(%d, .8f))%.*s%8.20f *\n", in, in - 1, 11, TABS, npow);
    return print_return(ifo + factor * npow);
    #else
    return
        material->i_formation +
        (material->i_binding - material->i_formation) / (pow(2.f, .8f) - 1) *
        (pow(in, .8f) - pow(in - 1, .8f));
    #endif
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 6
    Vacancy binding energy.
*/
double v_binding_energy(int vn)
{
    #if VPRINT
    double vfo = material->v_formation;
    double factor = (material->v_binding - material->v_formation) / (pow(2.f, .8f) - 1);
    double npow = (pow(vn, .8f) - pow(vn - 1, .8f));
    fprintf(stdout, "%s\n", __FUNCTION__);
    fprintf(stdout, "material->v_formation%.*s%8.20f +\n", 12, TABS, vfo);
    fprintf(stdout, "(material->v_binding - material->v_formation) / (pow(2.f, .8f) - 1)%.*s%8.20f *\n", 6, TABS, factor);
    fprintf(stdout, "(pow(%d, .8f) - pow(%d, .8f))%.*s%8.20f *\n", vn, vn - 1, 11, TABS, npow);
    return print_return(vfo + factor * npow);
    #else
    return
        material->v_formation +
        (material->v_binding - material->v_formation) / (pow(2.f, .8f) - 1) *
        (pow(vn, .8f) - pow(vn - 1, .8f));
    #endif
}
// --------------------------------------------------------------------------------------------