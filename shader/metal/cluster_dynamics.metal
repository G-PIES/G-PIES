#include <metal_stdlib>

#include "constants.hpp"
#include "cluster_dynamics_metal_args.hpp"

float i_defect_production(constant ClusterDynamicsMetalArgs&, uint);
float v_defect_production(constant ClusterDynamicsMetalArgs&, uint);
float i_clusters_delta(constant ClusterDynamicsMetalArgs&, uint);
float v_clusters_delta(constant ClusterDynamicsMetalArgs&, uint);
float iemission_vabsorption_np1(constant ClusterDynamicsMetalArgs&, uint);
float vemission_iabsorption_np1(constant ClusterDynamicsMetalArgs&, uint);
float iemission_vabsorption_n(constant ClusterDynamicsMetalArgs&, uint);
float vemission_iabsorption_n(constant ClusterDynamicsMetalArgs&, uint);
float iemission_vabsorption_nm1(constant ClusterDynamicsMetalArgs&, uint);
float vemission_iabsorption_nm1(constant ClusterDynamicsMetalArgs&, uint);
float i1_cluster_delta(constant ClusterDynamicsMetalArgs&);
float v1_cluster_delta(constant ClusterDynamicsMetalArgs&);
float i_emission_time(constant ClusterDynamicsMetalArgs&);
float v_emission_time(constant ClusterDynamicsMetalArgs&);
float i_absorption_time(constant ClusterDynamicsMetalArgs&);
float v_absorption_time(constant ClusterDynamicsMetalArgs&);
float annihilation_rate(constant ClusterDynamicsMetalArgs&);
float i_dislocation_annihilation_time(constant ClusterDynamicsMetalArgs&);
float v_dislocation_annihilation_time(constant ClusterDynamicsMetalArgs&);
float i_grain_boundary_annihilation_time(constant ClusterDynamicsMetalArgs&);
float v_grain_boundary_annihilation_time(constant ClusterDynamicsMetalArgs&);
float ii_emission(constant ClusterDynamicsMetalArgs&, uint);
float vv_emission(constant ClusterDynamicsMetalArgs&, uint);
float ii_absorption(constant ClusterDynamicsMetalArgs&, uint);
float vi_absorption(constant ClusterDynamicsMetalArgs&, uint);
float iv_absorption(constant ClusterDynamicsMetalArgs&, uint);
float vv_absorption(constant ClusterDynamicsMetalArgs&, uint);
float i_bias_factor(constant ClusterDynamicsMetalArgs&, uint);
float v_bias_factor(constant ClusterDynamicsMetalArgs&, uint);
float i_binding_energy(constant ClusterDynamicsMetalArgs&, uint);
float v_binding_energy(constant ClusterDynamicsMetalArgs&, uint);
float i_diffusion(constant ClusterDynamicsMetalArgs&);
float v_diffusion(constant ClusterDynamicsMetalArgs&);
float dislocation_promotion_probability(constant ClusterDynamicsMetalArgs&, uint);
float cluster_radius(constant ClusterDynamicsMetalArgs&, uint);

// Simulation Operation Functions
float dislocation_density_delta(constant ClusterDynamicsMetalArgs&);
float mean_dislocation_cell_radius(constant ClusterDynamicsMetalArgs&);
float ii_sum_absorption(constant ClusterDynamicsMetalArgs&, uint);
float iv_sum_absorption(constant ClusterDynamicsMetalArgs&, uint);
float vv_sum_absorption(constant ClusterDynamicsMetalArgs&, uint);
float vi_sum_absorption(constant ClusterDynamicsMetalArgs&, uint);
void step_init(constant ClusterDynamicsMetalArgs&);
bool validate(constant ClusterDynamicsMetalArgs&, uint);

kernel void update_clusters(
    constant ClusterDynamicsMetalArgs& args [[buffer(0)]],
    device float* interstitials_out,
    device float* vacancies_out,
    uint index [[thread_position_in_grid]]
    )
{
    if (index > 1)
    {
        interstitials_out[index] += i_clusters_delta(args, index) * args.delta_time;
        vacancies_out[index] += v_clusters_delta(args, index) * args.delta_time;
    }
}

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of interstital defects from the irradiation cascade for size (args, n) clusters.
*/
float i_defect_production(constant ClusterDynamicsMetalArgs& args, uint n)
{
    switch (n)
    {
        case 1: return args.reactor->recombination * args.reactor->flux *
                       (1. - args.reactor->i_bi - args.reactor->i_tri - args.reactor->i_quad);
        case 2: return args.reactor->recombination * args.reactor->flux * args.reactor->i_bi;
        case 3: return args.reactor->recombination * args.reactor->flux * args.reactor->i_tri;
        case 4: return args.reactor->recombination * args.reactor->flux * args.reactor->i_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.;
};

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of vacancy defects from the irradiation cascade for size (args, n) clusters.
*/
float v_defect_production(constant ClusterDynamicsMetalArgs& args, uint n)
{
    switch (n)
    {
        case 1: return args.reactor->recombination * args.reactor->flux *
                       (1. - args.reactor->v_bi - args.reactor->v_tri - args.reactor->v_quad);
        case 2: return args.reactor->recombination * args.reactor->flux * args.reactor->v_bi;
        case 3: return args.reactor->recombination * args.reactor->flux * args.reactor->v_tri;
        case 4: return args.reactor->recombination * args.reactor->flux * args.reactor->v_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.;
};
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (args, in) args.interstitials per unit volume.

                  (1)     (2)                    (3)              (4)
    dCi(args, n) / dt = Gi(args, n) + a[i,n+1] * Ci(n + 1) - b[i,n] * Ci(args, n) + c[i,n-1] * Ci(n-1)
*/
float i_clusters_delta(constant ClusterDynamicsMetalArgs& args, uint in)
{
    return
        // (1)
        i_defect_production(args, in)
        // (2)
        + iemission_vabsorption_np1(args, in + 1) * args.interstitials[in + 1]
        // (3)
        - iemission_vabsorption_n(args, in) * args.interstitials[in]
        // (4)
        + iemission_vabsorption_nm1(args, in - 1) * args.interstitials[in - 1];
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (args, in) args.vacancies per unit volume.

                  (1)     (2)                    (3)              (4)
    dCv(args, n) / dt = Gv(args, n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(args, n) + c[v,n-1] * Cv(n-1)
*/
float v_clusters_delta(constant ClusterDynamicsMetalArgs& args, uint vn)
{
    return
        // (1)
        v_defect_production(args, vn)
        // (2)
        + vemission_iabsorption_np1(args, vn + 1) * args.vacancies[vn + 1]
        // (3)
        - vemission_iabsorption_n(args, vn) * args.vacancies[vn]
        // (4)
        + vemission_iabsorption_nm1(args, vn - 1) * args.vacancies[vn - 1];
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (args, np1),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[i,n+1] = B[i,v](n + 1) * Cv(1) + E[i,i](n + 1)
*/
float iemission_vabsorption_np1(constant ClusterDynamicsMetalArgs& args, uint np1)
{
    return
        // (1)
        iv_absorption(args, np1) *
        // (2)
        args.vacancies[1] + 
        // (3)
        ii_emission(args, np1);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (args, np1),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[v,n+1] = B[v,i](n + 1) * Ci(1) + E[v,v](n + 1)
*/
float vemission_iabsorption_np1(constant ClusterDynamicsMetalArgs& args, uint np1)
{
    return 
        // (1)
        vi_absorption(args, np1) * 
        // (2)
        args.interstitials[1] + 
        // (3)
        vv_emission(args, np1);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2c
    The rate that a loop of size n can evolve toward a loop of size
    n + 1 absorbing an interstitial, or toward a loop of size
    n - 1 absorbing a vacancy or emitting an interstitial.

             (1)                 (2)                 (3)
    b[i,n] = B[i,v](args, n) * Cv(1) + B[i,i](args, n) * Ci(1) + E[i,i](args, n)
*/
float iemission_vabsorption_n(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return
        // (1)
        iv_absorption(args, n) * args.vacancies[1]
        // (2)
        + ii_absorption(args, n) * args.interstitials[1] * (1 - dislocation_promotion_probability(args, n))
        // (3)
        + ii_emission(args, n);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2c
    The rate that a loop of size n can evolve toward a loop of size
    n + 1 absorbing an vacancy, or toward a loop of size
    n - 1 absorbing an interstitial or emitting a vacancy.

             (1)                 (2)                 (3)
    b[v,n] = B[v,i](args, n) * Ci(1) + B[v,v](args, n) * Cv(1) + E[v,v](args, n)
*/
float vemission_iabsorption_n(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        // (1)
        vi_absorption(args, n) * args.interstitials[1] 
        // (2)
        + vv_absorption(args, n) * args.vacancies[1]
        // (3)
        + vv_emission(args, n);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2d
    The rate that an interstitial loop of size n - 1 can evolve into a
    loop of size n by absorbing an interstitial.

               (1)           (2)
    c[i,n-1] = B[i,i](n-1) * Ci(1)
*/
float iemission_vabsorption_nm1(constant ClusterDynamicsMetalArgs& args, uint nm1)
{
    return
        // (1)
        ii_absorption(args, nm1) *
        // (2)
        args.interstitials[1];
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2d
    The rate that a vacancy loop of size n - 1 can evolve into a
    loop of size n by absorbing a vacancy.

               (1)           (2)
    c[v,n-1] = B[v,v](n-1) * Cv(1)
*/
float vemission_iabsorption_nm1(constant ClusterDynamicsMetalArgs& args, uint nm1)
{
    return
        // (1)
        vv_absorption(args, nm1) * 
        // (2)
        args.vacancies[1];
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3a
    Point defects concentrations per unit volume given a cluster size (args, in).
    ** in is only used in characteristic time calculations which rely on cluster sizes up to (args, in).

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
float i1_cluster_delta(constant ClusterDynamicsMetalArgs& args)
{
    return 
        // (1)
        i_defect_production(args, 1)
        // (2)
        - annihilation_rate(args) * args.interstitials[1] * args.vacancies[1]
        // (3)
        - args.interstitials[1] * i_dislocation_annihilation_time(args)
        // (4)
        - args.interstitials[1] * i_grain_boundary_annihilation_time(args)
        // (5)
        - args.interstitials[1] * i_absorption_time(args)
        // (6)
        + i_emission_time(args);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3a
    Point defects concentrations per unit volume given a cluster size (args, in).
    ** vn is only used in characteristic time calculations which rely on cluster sizes up to (args, in).

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
float v1_cluster_delta(constant ClusterDynamicsMetalArgs& args)
{
    return 
        // (1)
        v_defect_production(args, 1)
        // (2)
        - annihilation_rate(args) * args.interstitials[1] * args.vacancies[1]
        // (3)
        - args.vacancies[1] * v_dislocation_annihilation_time(args)
        // (4)
        - args.vacancies[1] * v_grain_boundary_annihilation_time(args)
        // (5)
        - args.vacancies[1] * v_absorption_time(args)
        // (6)
        + v_emission_time(args);
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting an interstitial by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                      (2)                     (3)
    tEi(args, n) = SUM ( E[i,i](args, n) * Ci(args, n) ) + 4 * E[i,i](2) * Ci(2) + B[i,v](2) * Cv(2) * Ci(2)
*/
float i_emission_time(constant ClusterDynamicsMetalArgs& args)
{
   float time = 0.;
   for (uint in = 3; in < args.concentration_boundary - 1; ++in)
   {
         time +=
         // (1)
         ii_emission(args, in) * args.interstitials[in];
   }

   time +=
      // (2)
      4. * ii_emission(args, 2) * args.interstitials[2]
      // (3)
      + iv_absorption(args, 2) * args.vacancies[2] * args.interstitials[2];

  return time;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                           (2)                     (3)
    tEv(args, n) = SUM[n>0] ( E[v,v](args, n) * Cv(args, n) ) + 4 * E[v,v](2) * Cv(2) + B[v,i](2) * Cv(2) * Ci(2)
*/
float v_emission_time(constant ClusterDynamicsMetalArgs& args)
{
   float time = 0.;
   for (uint vn = 3; vn < args.concentration_boundary - 1; ++vn)
   {
      time += 
         // (1)
         vv_emission(args, vn) * args.vacancies[vn];
   }

   time +=
      // (2)
      4. * vv_emission(args, 2) * args.vacancies[2]
      // (3)
      + vi_absorption(args, 2) * args.vacancies[2] * args.interstitials[2];

   return time;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing an interstitial by the population of interstital or vacancy
    clusters of size up to (nmax).

                        (1)                              (2)
    tAi(args, n) = SUM[n>0] ( B[i,i](args, n) * Ci(args, n) ) + SUM[n>1] ( B[v,i](args, n) * Cv(args, n) )
*/
float i_absorption_time(constant ClusterDynamicsMetalArgs& args)
{
   float time = ii_absorption(args, 1) * args.interstitials[1];
   for (uint in = 2; in < args.concentration_boundary - 1; ++in)
   {
      time +=
         // (1)
         ii_absorption(args, in) * args.interstitials[in]
         // (2)
         + vi_absorption(args, in) * args.vacancies[in];
   }

   return time;
}


/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                        (1)                              (2)
    tAv(args, n) = SUM[n>0] ( B[v,v](args, n) * Cv(args, n) ) + SUM[n>1] ( B[i,v](args, n) * Ci(args, n) )
*/
float v_absorption_time(constant ClusterDynamicsMetalArgs& args)
{
   float time = vv_absorption(args, 1) * args.vacancies[1];
   for (uint vn = 2; vn < args.concentration_boundary - 1; ++vn)
   {
      time +=
         // (1)
         vv_absorption(args, vn) * args.vacancies[vn]
         // (2)
         + iv_absorption(args, vn) * args.interstitials[vn];
   }

   return time;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3d
    Annihilation rate of args.vacancies and insterstitals.

          (1)      (2)         (3)
    Riv = 4 * PI * (Di + Dv) * riv
*/
float annihilation_rate(constant ClusterDynamicsMetalArgs& args)
{
    return 
        // (1)
        4. * M_PI * 
        // (2)
        (i_diffusion(args) + v_diffusion(args)) *
        // (3)
        args.material->recombination_radius;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of args.interstitials on dislocations.

           (1)   (2)    (3)
    tAdi = p  *  Di  *  Zi
*/
float i_dislocation_annihilation_time(constant ClusterDynamicsMetalArgs& args)
{
    return
        // (1)
        args.dislocation_density *
        // (2)
        i_diffusion(args) *
        // (3)
        args.material->i_dislocation_bias;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of args.vacancies on dislocations.

           (1)   (2)    (3)
    tAdv = p  *  Dv  *  Zv
*/
float v_dislocation_annihilation_time(constant ClusterDynamicsMetalArgs& args)
{
    return
        // (1)
        args.dislocation_density *
        // (2)
        v_diffusion(args) *
        // (3)
        args.material->v_dislocation_bias;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of args.interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdi = 6 * Di * metal::sqrt( p * Zi + SUM[n] ( B[i,i](args, n) * Ci(args, n) ) + SUM[n] ( B[v,i](args, n) * Cv(args, n) ) ) / d
*/
float i_grain_boundary_annihilation_time(constant ClusterDynamicsMetalArgs& args)
{
    return
        // (1)
        6. * i_diffusion(args) *
        metal::sqrt
        (
            // (2)
            args.dislocation_density * 
            args.material->i_dislocation_bias
            // (3)
            + args.ii_sum_absorption_val
            // (4)
            + args.vi_sum_absorption_val
        ) /
        // (5)
        args.material->grain_size;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of args.interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdv = 6 * Di * metal::sqrt( p * Zv + SUM[n] ( B[v,v](args, n) * Cv(args, n) ) + SUM[n] ( B[i,v](args, n) * Ci(args, n) ) ) / d
*/
float v_grain_boundary_annihilation_time(constant ClusterDynamicsMetalArgs& args)
{
    return
        // (1)
        6. * v_diffusion(args) *
        metal::sqrt
        (
            // (2)
            args.dislocation_density *
            args.material->v_dislocation_bias
            // (3)
            + args.vv_sum_absorption_val
            // (4)
            + args.iv_sum_absorption_val
        ) /
        // (5)
        args.material->grain_size;
}

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4a
    Rate of emission of an interstitial by an interstital loop of size (args, n).
*/
float ii_emission(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        i_bias_factor(args, n) *
        i_diffusion(args) / args.material->atomic_volume *
        metal::exp
        (
            -i_binding_energy(args, n) /
            (BOLTZMANN_EV_KELVIN * args.reactor->temperature)
        );
}


/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4b
    Rate of absorption of an interstitial by an interstital loop of size (args, n).
*/
float ii_absorption(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        i_bias_factor(args, n) *
        i_diffusion(args);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4c
    Rate of absorption of an interstitial by a vacancy loop of size (args, n).
*/
float iv_absorption(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        v_bias_factor(args, n) *
        v_diffusion(args);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4d
    Rate of emission of a vacancy by a vacancy loop of size (args, n).
*/
float vv_emission(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        v_bias_factor(args, n) *
        v_diffusion(args) *
        metal::exp
        (
            -v_binding_energy(args, n) /
            (BOLTZMANN_EV_KELVIN * args.reactor->temperature)
        );
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4e
    Rate of absorption of a vacancy by a vacancy loop of size (args, n).
*/
float vv_absorption(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        v_bias_factor(args, n) *
        v_diffusion(args);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4f
    Rate of absorption of a vacancy by an interstitial loop of size (args, n).
*/
float vi_absorption(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        i_bias_factor(args, n) *
        i_diffusion(args);
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Interstitial bias factor.
*/
float i_bias_factor(constant ClusterDynamicsMetalArgs& args, uint in)
{
    return 
        args.material->i_dislocation_bias +
        (
            metal::sqrt
            (
                    args.material->burgers_vector /
                    (8. * M_PI * args.material->lattice_param)
            ) *
            args.material->i_loop_bias -
            args.material->i_dislocation_bias
        ) *
        1. /
        metal::pow
        (
            (float)in,
            args.material->i_dislocation_bias_param / 2.
        );
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Vacancy bias factor.
*/
float v_bias_factor(constant ClusterDynamicsMetalArgs& args, uint vn)
{
    return 
        args.material->v_dislocation_bias +
        (
            metal::sqrt
            (
                    args.material->burgers_vector /
                    (8. * M_PI * args.material->lattice_param)
            ) *
            args.material->v_loop_bias -
            args.material->v_dislocation_bias
        ) *
        1. /
        metal::pow
        (
            (float)vn,
            args.material->v_dislocation_bias_param / 2.
        );
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Interstitial binding energy.
*/
float i_binding_energy(constant ClusterDynamicsMetalArgs& args, uint in)
{
    return
        args.material->i_formation
        + (args.material->i_binding - args.material->i_formation) / (metal::pow(2., .8) - 1.) *
        (metal::pow((float)in, .8) - metal::pow((float)in - 1., .8));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Vacancy binding energy.
*/
float v_binding_energy(constant ClusterDynamicsMetalArgs& args, uint vn)
{
    return
        args.material->v_formation
        + (args.material->v_binding - args.material->v_formation) / (metal::pow(2., .8) - 1) *
        (metal::pow((float)vn, .8) - metal::pow((float)vn - 1., .8));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
float i_diffusion(constant ClusterDynamicsMetalArgs& args)
{
    return args.material->i_diffusion_0 * metal::exp(-args.material->i_migration / (BOLTZMANN_EV_KELVIN * args.reactor->temperature));
}

/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
float v_diffusion(constant ClusterDynamicsMetalArgs& args)
{
   return args.material->v_diffusion_0 * metal::exp(-args.material->v_migration / (BOLTZMANN_EV_KELVIN * args.reactor->temperature));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.10
*/
float mean_dislocation_cell_radius(constant ClusterDynamicsMetalArgs& args)
{
   float r_0_factor = 0.;
   for (uint i = 1; i < args.concentration_boundary; ++i)
   {
      r_0_factor += cluster_radius(args, i) * args.interstitials[i];
   }

   return 1 / metal::sqrt((2. * M_PI * M_PI / args.material->atomic_volume) * r_0_factor + M_PI * args.dislocation_density);
}

// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.12
*/
float dislocation_promotion_probability(constant ClusterDynamicsMetalArgs& args, uint n)
{
   float dr = cluster_radius(args, n + 1) - cluster_radius(args, n);

   return (2. * cluster_radius(args, n) * dr + metal::pow(dr, 2.)) 
      / (M_PI * args.mean_dislocation_radius_val / 2. - metal::pow(cluster_radius(args, n), 2.)); 
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 8
*/
float dislocation_density_delta(constant ClusterDynamicsMetalArgs& args)
{
   float gain = 0.0;
   for (uint n = 1; n < args.concentration_boundary; ++n)
   {
      gain += cluster_radius(args, n) * dislocation_promotion_probability(args, n) * ii_absorption(args, n) * args.interstitials[n];
   }

   gain *= 2. * M_PI / args.material->atomic_volume;

   return 
      gain
      - args.reactor->dislocation_density_evolution * 
      metal::pow(args.material->burgers_vector, 2.) *
      metal::pow(args.dislocation_density, 3. / 2.);
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg. 346, 7.63
*/
float cluster_radius(constant ClusterDynamicsMetalArgs& args, uint n)
{
    return metal::sqrt(metal::sqrt(3.) * metal::pow(args.material->lattice_param, 2.) * (float)n / (4. * M_PI));
}
// --------------------------------------------------------------------------------------------


float ii_sum_absorption(constant ClusterDynamicsMetalArgs& args, uint nmax)
{
   float emission = 0.;
   for (uint vn = 1; vn < nmax; ++vn)
   {
      emission += ii_absorption(args, vn) * args.interstitials[vn];
   }

   return emission;
}

float iv_sum_absorption(constant ClusterDynamicsMetalArgs& args, uint nmax)
{
   float emission = 0.;
   for (uint n = 1; n < nmax; ++n)
   {
      emission += iv_absorption(args, n) * args.interstitials[n];
   }

   return emission;
}

float vv_sum_absorption(constant ClusterDynamicsMetalArgs& args, uint nmax)
{
   float emission = 0.;
   for (uint n = 1; n < nmax; ++n)
   {
      emission += vv_absorption(args, n) * args.vacancies[n];
   }

   return emission;
}

float vi_sum_absorption(constant ClusterDynamicsMetalArgs& args, uint nmax)
{
   float emission = 0.;
   for (uint n = 1; n < nmax; ++n)
   {
      emission += vi_absorption(args, n) * args.vacancies[n];
   }

   return emission;
}