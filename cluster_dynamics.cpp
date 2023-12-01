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
    return
        // (1)
        i_defect_production(in)
        // (2)
        + iemission_vabsorption_np1(in + 1) * interstitials[in + 1]
        // (3)
        - iemission_vabsorption_n(in) * interstitials[in]
        // (4)
        + iemission_vabsorption_nm1(in - 1) * interstitials[in - 1];
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (vn) vacancies per unit volume.

                  (1)     (2)                    (3)              (4)
    dCv(n) / dt = Gv(n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(n) + c[v,n-1] * Cv(n-1)
*/
double v_clusters_delta(uint64_t vn)
{
    return
        // (1)
        v_defect_production(vn)
        // (2)
        + vemission_iabsorption_np1(vn + 1) * vacancies[vn + 1]
        // (3)
        - vemission_iabsorption_n(vn) * vacancies[vn]
        // (4)
        + vemission_iabsorption_nm1(vn - 1) * vacancies[vn - 1];
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
    return
        // (1)
        iv_absorption(np1) *
        // (2)
        vacancies[1] + 
        // (3)
        ii_emission(np1);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (np1),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[v,n+1] = B[v,i](n + 1) * Ci(1) + E[v,v](n + 1)
*/
double vemission_iabsorption_np1(uint64_t np1)
{
    return 
        // (1)
        vi_absorption(np1) * 
        // (2)
        interstitials[1] + 
        // (3)
        vv_emission(np1);
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
    return
        // (1)
        iv_absorption(n) * vacancies[1]
        // (2)
        + ii_absorption(n) * interstitials[1] * (1 - dislocation_promotion_probability(n))
        // (3)
        + ii_emission(n);
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
    return 
        // (1)
        vi_absorption(n) * interstitials[1] 
        // (2)
        + vv_absorption(n) * vacancies[1]
        // (3)
        + vv_emission(n);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2d
    The rate that an interstitial loop of size n - 1 can evolve into a
    loop of size n by absorbing an interstitial.

               (1)           (2)
    c[i,n-1] = B[i,i](n-1) * Ci(1)
*/
double iemission_vabsorption_nm1(uint64_t nm1)
{
    return
        // (1)
        ii_absorption(nm1) *
        // (2)
        interstitials[1];
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2d
    The rate that a vacancy loop of size n - 1 can evolve into a
    loop of size n by absorbing a vacancy.

               (1)           (2)
    c[v,n-1] = B[v,v](n-1) * Cv(1)
*/
double vemission_iabsorption_nm1(uint64_t nm1)
{
    return
        // (1)
        vv_absorption(nm1) * 
        // (2)
        vacancies[1];
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
double i1_cluster_delta(uint64_t nmax)
{
    return 
        // (1)
        i_defect_production(1)
        // (2)
        - annihilation_rate() * interstitials[1] * vacancies[1]
        // (3)
        - interstitials[1] * i_dislocation_annihilation_time()
        // (4)
        - interstitials[1] * i_grain_boundary_annihilation_time(nmax)
        // (5)
        - interstitials[1] * i_absorption_time(nmax)
        // (6)
        + i_emission_time(nmax);
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
double v1_cluster_delta(uint64_t nmax)
{
    return 
        // (1)
        v_defect_production(1)
        // (2)
        - annihilation_rate() * interstitials[1] * vacancies[1]
        // (3)
        - vacancies[1] * v_dislocation_annihilation_time()
        // (4)
        - vacancies[1] * v_grain_boundary_annihilation_time(nmax)
        // (5)
        - vacancies[1] * v_absorption_time(nmax)
        // (6)
        + v_emission_time(nmax);
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
    for (uint64_t in = 3; in < nmax; ++in)
    {
          time +=
        //      // (1)
              ii_emission(in) * interstitials[in];
    }

    time +=
        // (2)
        4 * ii_emission(2) * interstitials[2]
        // (3)
        + iv_absorption(2) * vacancies[2] * interstitials[2];

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
    for (uint64_t vn = 3; vn < nmax; ++vn)
    {
        time += 
            // (1)
            vv_emission(vn) * vacancies[vn];
    }

    time +=
        // (2)
        4 * vv_emission(2) * vacancies[2]
        // (3)
        + vi_absorption(2) * vacancies[2] * interstitials[2];

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
            ii_absorption(in) * interstitials[in]
            // (2)
            + vi_absorption(in) * vacancies[in];
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
            vv_absorption(vn) * vacancies[vn]
            // (2)
            + iv_absorption(vn) * interstitials[vn];
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
        (i_diffusion() + v_diffusion()) *
        // (3)
        material.recombination_radius;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of interstitials on dislocations.

           (1)   (2)    (3)
    tAdi = p  *  Di  *  Zi
*/
double i_dislocation_annihilation_time()
{
    return
        // (1)
        dislocation_density *
        // (2)
        i_diffusion() *
        // (3)
        material.i_dislocation_bias;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of vacancies on dislocations.

           (1)   (2)    (3)
    tAdv = p  *  Dv  *  Zv
*/
double v_dislocation_annihilation_time()
{
    return
        // (1)
        dislocation_density *
        // (2)
        v_diffusion() *
        // (3)
        material.v_dislocation_bias;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdi = 6 * Di * sqrt( p * Zi + SUM[n] ( B[i,i](n) * Ci(n) ) + SUM[n] ( B[v,i](n) * Cv(n) ) ) / d
*/
double i_grain_boundary_annihilation_time(uint64_t in)
{
    return
        // (1)
        6 * i_diffusion() *
        sqrt
        (
            // (2)
            dislocation_density * 
            material.i_dislocation_bias
            // (3)
            + ii_sum_absorption(in)
            // (4)
            + vi_sum_absorption(in)
        ) /
        // (5)
        material.grain_size;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdv = 6 * Di * sqrt( p * Zv + SUM[n] ( B[v,v](n) * Cv(n) ) + SUM[n] ( B[i,v](n) * Ci(n) ) ) / d
*/
double v_grain_boundary_annihilation_time(uint64_t vn)
{
    return
        // (1)
        6 * v_diffusion() *
        sqrt
        (
            // (2)
            dislocation_density *
            material.v_dislocation_bias
            // (3)
            + vv_sum_absorption(vn)
            // (4)
            + iv_sum_absorption(vn)
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
    return 
        2 * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        i_diffusion() / material.atomic_volume *
        exp
        (
            -i_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor.temperature)
        );
}


/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4b
    Rate of absorption of an interstitial by an interstital loop of size (n).
*/
double ii_absorption(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        i_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4c
    Rate of absorption of an interstitial by a vacancy loop of size (n).
*/
double iv_absorption(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        v_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4d
    Rate of emission of a vacancy by a vacancy loop of size (n).
*/
double vv_emission(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        v_diffusion() *
        exp
        (
            -v_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor.temperature)
        );
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4e
    Rate of absorption of a vacancy by a vacancy loop of size (n).
*/
double vv_absorption(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        v_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4f
    Rate of absorption of a vacancy by an interstitial loop of size (n).
*/
double vi_absorption(uint64_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        i_diffusion();
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Interstitial bias factor.
*/
double i_bias_factor(uint64_t in)
{
    return 
        material.i_dislocation_bias +
        (
            std::sqrt
            (
                    material.burgers_vector /
                    (8 * M_PI * material.lattice_param)
            ) *
            material.i_loop_bias -
            material.i_dislocation_bias
        ) *
        1 /
        std::pow
        (
            in,
            material.i_dislocation_bias_param / 2.
        );
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Vacancy bias factor.
*/
double v_bias_factor(uint64_t vn)
{
    return 
        material.v_dislocation_bias +
        (
            std::sqrt
            (
                    material.burgers_vector /
                    (8 * M_PI * material.lattice_param)
            ) *
            material.v_loop_bias -
            material.v_dislocation_bias
        ) *
        1 /
        std::pow
        (
            vn,
            material.v_dislocation_bias_param / 2.
        );
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Interstitial binding energy.
*/
double i_binding_energy(uint64_t in)
{
    return
        material.i_formation
        + (material.i_binding - material.i_formation) / (std::pow(2., .8) - 1) *
        (std::pow(in, .8) - std::pow(in - 1., .8));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Vacancy binding energy.
*/
double v_binding_energy(uint64_t vn)
{
    return
        material.v_formation
        + (material.v_binding - material.v_formation) / (std::pow(2., .8) - 1) *
        (std::pow(vn, .8) - std::pow(vn - 1., .8));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
double i_diffusion()
{
    return material.i_diffusion_0 * std::exp(-material.i_migration / (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
double v_diffusion()
{
    return material.v_diffusion_0 * std::exp(-material.v_migration / (BOLTZMANN_EV_KELVIN * reactor.temperature));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.10
*/
double mean_dislocation_cell_radius(uint64_t n)
{
    double r_0_factor = 0.;
    for (uint64_t i = 1; i < concentration_boundary; ++i)
    {
        r_0_factor += cluster_radius(i) * interstitials[i];
    }

    return 1 / std::sqrt((2 * M_PI * M_PI / material.atomic_volume) * r_0_factor + M_PI * dislocation_density);
}

/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.12
*/
double dislocation_promotion_probability(uint64_t n)
{
    double dr = cluster_radius(n + 1) - cluster_radius(n);

    double r_0 = mean_dislocation_cell_radius(n);

    return (2 * cluster_radius(n) * dr + std::pow(dr, 2)) 
         / (M_PI * r_0 / 2. - std::pow(cluster_radius(n), 2)); 
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 8
*/
double dislocation_density_delta()
{
    double gain = 0.0;
    for (uint64_t n = 1; n < concentration_boundary; ++n)
    {
        gain += dislocation_promotion_probability(n) * ii_absorption(n) * interstitials[n];
    }

    return 
        gain
        - reactor.dislocation_density_evolution * 
        std::pow(material.burgers_vector, 2) *
        std::pow(dislocation_density, 3./2.);
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg. 346, 7.63
*/
double cluster_radius(uint64_t n)
{
    return std::sqrt(std::sqrt(3) * std::pow(material.lattice_param, 2) * (double)n / (4 * M_PI));
}
// --------------------------------------------------------------------------------------------
