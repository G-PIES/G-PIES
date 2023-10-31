#include <iostream>
#include <math.h>

#include "cluster_dynamics.hpp"

// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 1a-1e
*/
double i_defect_production(int cluster_size)
{
    switch (cluster_size)
    {
        case 1: return reactor->recombination * reactor->flux *
                       (1 - reactor->i_bi - reactor->i_tri - reactor->i_quad);
        case 2: return reactor->recombination * reactor->flux * reactor->i_bi;
        case 3: return reactor->recombination * reactor->flux * reactor->i_tri;
        case 4: return reactor->recombination * reactor->flux * reactor->i_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.f;
};

double v_defect_production(int cluster_size)
{
    switch (cluster_size)
    {
        case 1: return reactor->recombination * reactor->flux *
                       (1 - reactor->v_bi - reactor->v_tri - reactor->v_quad);
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
// number of clusters of N interstitials (in) per unit volume
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2a
    calculates the number of clusters of N interstitials or vacancies (in or vn) per unit volume

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
        iemission_vabsorption_n(in) * interstitials[in] +
        // (4)
        iemission_vabsorption_nm1(in - 1) * interstitials[in - 1];
}

double v_clusters(int vn)
{
    return
        // (1)
        v_defect_production(vn) +
        // (2)
        vemission_iabsorption_np1(vn + 1) * vacancies[vn + 1] -
        // (3)
        vemission_iabsorption_n(vn) * vacancies[vn] +
        // (4)
        vemission_iabsorption_nm1(vn - 1) * vacancies[vn - 1];
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2b
*/
double iemission_vabsorption_np1(int np1)
{
    return iv_absorption(np1) * v_clusters1(np1) + ii_emission(np1);
}

double vemission_iabsorption_np1(int np1)
{
    return vi_absorption(np1) * i_clusters1(np1) + vv_emission(np1);
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2c
*/
double iemission_vabsorption_n(int n)
{
    return iv_absorption(n) * v_clusters1(n) + 
           ii_absorption(n) * i_clusters1(n) +
           ii_emission(n);
}

double vemission_iabsorption_n(int n)
{
    return vi_absorption(n) * i_clusters1(n) + 
           vv_absorption(n) * v_clusters1(n) +
           vv_emission(n);
}

/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 2d
*/
double iemission_vabsorption_nm1(int nm1)
{
    return nm1 == 0 ? 0 :
        ii_absorption(nm1) * i_clusters1(nm1);
}

double vemission_iabsorption_nm1(int nm1)
{
    return nm1 == 0 ? 0 :
        vv_absorption(nm1) * v_clusters1(nm1);
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3a
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
*/
double i_emission_time(int nmax)
{
    double time = 0.f;
    for (int in = 2; in < nmax; ++in)
    {
        time +=
            ii_emission(in) * interstitials[in] +
            4 * ii_emission(2) * interstitials[2] +
            iv_absorption(2) * vacancies[2] * interstitials[2];
    }

    return 0.f;
}

double v_emission_time(int nmax)
{
    double time = 0.f;
    for (int vn = 2; vn < nmax; ++vn)
    {
        time += 
            vv_emission(vn) * vacancies[vn] +
            4 * vv_emission(2) * vacancies[2] +
            vi_absorption(2) * vacancies[2] * interstitials[2];
    }

    return 0.f;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3c
*/
double i_absorption_time(int nmax)
{
    double time = ii_absorption(1) * interstitials[1];
    for (int in = 1; in < nmax; ++in)
    {
        time +=
            ii_absorption(in) * interstitials[in] +
            vi_absorption(in) * vacancies[in];
    }

    return time;
}

double v_absorption_time(int nmax)
{
    double time = vv_absorption(1) * vacancies[1];
    for (int vn = 1; vn < nmax; ++vn)
    {
        time +=
            vv_absorption(vn) * vacancies[vn] +
            iv_absorption(vn) * interstitials[vn];
    }

    return time;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3d
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
*/
double i_dislocation_annihilation_time()
{
    return
        // TODO: floating point overflow issue with large densities
        material->dislocation_density_initial *
        material->i_diffusion *
        material->i_dislocation_bias;
}

double v_dislocation_annihilation_time()
{
    return
        // TODO: floating point overflow issue with large densities
        material->dislocation_density_initial *
        material->v_diffusion *
        material->v_dislocation_bias;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor et al. / Journal of Nuclear Materials 326 (2004), 3f
*/
double i_grain_boundary_annihilation_time(int vn)
{
    return
        6 * material->i_diffusion *
        sqrt
        (
            // TODO: floating point overflow issue with large densities
            material->dislocation_density_initial * 
            material->i_dislocation_bias +
            ii_sum_absorption(vn) +
            vi_sum_absorption(vn)
        ) /
        material->grain_size;
}

double v_grain_boundary_annihilation_time(int vn)
{
    return
        6 * material->v_diffusion *
        sqrt
        (
            // TODO: floating point overflow issue with large densities
            material->dislocation_density_initial *
            material->v_dislocation_bias +
            vv_sum_absorption(vn) +
            iv_sum_absorption(vn)
        ) /
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