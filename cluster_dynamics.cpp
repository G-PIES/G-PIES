#include <iostream>
#include <math.h>

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
    return
        // (1)
        i_defect_production(in) +
        // (2)
        iemission_vabsorption_np1(in + 1) * interstitials[in + 1] -
        // (3)
        iemission_vabsorption_n(in) * interstitials[in];
        // (4)
        iemission_vabsorption_nm1(in - 1) * interstitials[in - 1];
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (vn) vacancies per unit volume.

                  (1)     (2)                    (3)              (4)
    dCv(n) / dt = Gv(n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(n) + c[v,n-1] * Cv(n-1)
*/
double v_clusters(int vn)
{
    return
        // (1)
        v_defect_production(vn) +
        // (2)
        vemission_iabsorption_np1(vn + 1) * vacancies[vn + 1] -
        // (3)
        vemission_iabsorption_n(vn) * vacancies[vn];
        // (4)
        vemission_iabsorption_nm1(vn - 1) * vacancies[vn - 1];
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
    return
        // (1)
        iv_absorption(np1) *
        // (2)
        v_clusters1(np1) + 
        // (3)
        ii_emission(np1);
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (np1),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[v,n+1] = B[v,i](n + 1) * Ci(1) + E[v,v](n + 1)
*/
double vemission_iabsorption_np1(int np1)
{
    return 
        // (1)
        vi_absorption(np1) * 
        // (2)
        i_clusters1(np1) + 
        // (3)
        vv_emission(np1);
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
    return 
        // (1)
        iv_absorption(n) * v_clusters1(n) + 
        // (2)
        ii_absorption(n) * i_clusters1(n) +
        // (3)
        ii_emission(n);
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
    return 
        // (1)
        vi_absorption(n) * i_clusters1(n) + 
        // (2)
        vv_absorption(n) * v_clusters1(n) +
        // (3)
        vv_emission(n);
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2d
    The rate that an interstitial loop of size n - 1 can evolve into a
    loop of size n by absorbing an interstitial.

               (1)           (2)
    c[i,n-1] = B[i,i](n-1) * Ci(1)
*/
double iemission_vabsorption_nm1(int nm1)
{
    return
        // (1)
        ii_absorption(nm1) *
        // (2)
        i_clusters1(nm1);
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2d
    The rate that a vacancy loop of size n - 1 can evolve into a
    loop of size n by absorbing a vacancy.

               (1)           (2)
    c[v,n-1] = B[v,v](n-1) * Cv(1)
*/
double vemission_iabsorption_nm1(int nm1)
{
    return
        // (1)
        vv_absorption(nm1) * 
        // (2)
        v_clusters1(nm1);
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

                (1)                 (2)                     (3)
    tEi(n) = SUM [ E[i,i](n) * Ci(n) + 4 * E[i,i](2) * Ci(2) + B[i,v](2) * Cv(2) * Ci(2) ]
*/
double i_emission_time(int nmax)
{
    double time = 0.f;
    for (int in = 2; in < nmax; ++in)
    {
        time +=
            // (1)
            ii_emission(in) * interstitials[in] +
            // (2)
            4 * ii_emission(2) * interstitials[2] +
            // (3)
            iv_absorption(2) * vacancies[2] * interstitials[2];
    }

    return 0.f;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                 (2)                     (3)
    tEv(n) = SUM[n>0] ( E[v,v](n) * Cv(n) + 4 * E[v,v](2) * Cv(2) + B[v,i](2) * Cv(2) * Ci(2) )
*/
double v_emission_time(int nmax)
{
    double time = 0.f;
    for (int vn = 2; vn < nmax; ++vn)
    {
        time += 
            // (1)
            vv_emission(vn) * vacancies[vn] +
            // (2)
            4 * vv_emission(2) * vacancies[2] +
            // (3)
            vi_absorption(2) * vacancies[2] * interstitials[2];
    }

    return 0.f;
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
*/
double ii_emission(int in)
{
    // TODO: atomic volume
    float atomic_volume = 1e-30;
    return 
        2 * M_PI * in *
        i_bias_factor(in) *
        material->i_diffusion / atomic_volume *
        exp
        (
            -i_binding_energy(in) /
            (BOLTZMANN_EV_KELVIN * reactor->temperature_kelvin())
        );
}


/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4b
*/
double ii_absorption(int in)
{
    return 
        2 * M_PI * in *
        i_bias_factor(in) *
        material->i_diffusion;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4c
*/
double iv_absorption(int in)
{
    return 
        2 * M_PI * in *
        i_bias_factor(in) *
        material->v_diffusion;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4d
*/
double vv_emission(int vn)
{
    return 
        2 * M_PI * vn *
        v_bias_factor(vn) *
        material->v_diffusion *
        exp
        (
            -v_binding_energy(vn) /
            (BOLTZMANN_EV_KELVIN * reactor->temperature_kelvin())
        );
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4e
*/
double vv_absorption(int vn)
{
    return 
        2 * M_PI * vn *
        v_bias_factor(vn) *
        material->v_diffusion;
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 4f
*/
double vi_absorption(int vn)
{
    return 
        2 * M_PI * vn *
        v_bias_factor(vn) *
        material->i_diffusion;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 5
*/
double i_bias_factor(int in)
{
    // TODO: lattice parameters
    float lattice_param_a = 1e8;
    float burgers_vector = 1e8;

    return 
        material->i_dislocation_bias +
        (
            sqrt
            (
                    burgers_vector /
                    (8 * M_PI * lattice_param_a)
            ) *
            material->i_loop_bias -
            material->i_dislocation_bias
        ) *
        pow
        (
            in,
            -1 * material->i_dislocation_bias_param / 2
        );
}

double v_bias_factor(int vn)
{
    // TODO: lattice parameters
    float lattice_param_a = 1e8;
    float burgers_vector = 1e8;

    return 
        material->v_dislocation_bias +
        (
            sqrt
            (
                    burgers_vector /
                    (8 * M_PI * lattice_param_a)
            ) *
            material->v_loop_bias -
            material->v_dislocation_bias
        ) *
        pow
        (
            vn,
            -1 * material->v_dislocation_bias_param / 2
        );
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 6
*/
double i_binding_energy(int in)
{
    return
        material->i_formation +
        (material->i_binding - material->i_formation) / (pow(2.f, .8f) - 1) *
        (pow(in, .8f) - pow(in - 1, .8f));
}

double v_binding_energy(int vn)
{
    return
        material->v_formation +
        (material->v_binding - material->v_formation) / (pow(2.f, .8f) - 1) *
        (pow(vn, .8f) - pow(vn - 1, .8f));
}
// --------------------------------------------------------------------------------------------