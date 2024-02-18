#include <metal_stdlib>

#include "utils/constants.hpp"
#include "cluster_dynamics_metal_args.hpp"

gp_float i_defect_production(device ClusterDynamicsMetalArgs&, uint);
gp_float v_defect_production(device ClusterDynamicsMetalArgs&, uint);
gp_float i_clusters_delta(device ClusterDynamicsMetalArgs&, uint);
gp_float v_clusters_delta(device ClusterDynamicsMetalArgs&, uint);
gp_float iemission_vabsorption_np1(device ClusterDynamicsMetalArgs&, uint);
gp_float vemission_iabsorption_np1(device ClusterDynamicsMetalArgs&, uint);
gp_float iemission_vabsorption_n(device ClusterDynamicsMetalArgs&, uint);
gp_float vemission_iabsorption_n(device ClusterDynamicsMetalArgs&, uint);
gp_float iemission_vabsorption_nm1(device ClusterDynamicsMetalArgs&, uint);
gp_float vemission_iabsorption_nm1(device ClusterDynamicsMetalArgs&, uint);
gp_float i1_cluster_delta(device ClusterDynamicsMetalArgs&);
gp_float v1_cluster_delta(device ClusterDynamicsMetalArgs&);
gp_float i_emission_time(device ClusterDynamicsMetalArgs&);
gp_float v_emission_time(device ClusterDynamicsMetalArgs&);
gp_float i_absorption_time(device ClusterDynamicsMetalArgs&);
gp_float v_absorption_time(device ClusterDynamicsMetalArgs&);
gp_float annihilation_rate(device ClusterDynamicsMetalArgs&);
gp_float i_dislocation_annihilation_time(device ClusterDynamicsMetalArgs&);
gp_float v_dislocation_annihilation_time(device ClusterDynamicsMetalArgs&);
gp_float i_grain_boundary_annihilation_time(device ClusterDynamicsMetalArgs&);
gp_float v_grain_boundary_annihilation_time(device ClusterDynamicsMetalArgs&);
gp_float ii_emission(device ClusterDynamicsMetalArgs&, uint);
gp_float vv_emission(device ClusterDynamicsMetalArgs&, uint);
gp_float ii_absorption(device ClusterDynamicsMetalArgs&, uint);
gp_float vi_absorption(device ClusterDynamicsMetalArgs&, uint);
gp_float iv_absorption(device ClusterDynamicsMetalArgs&, uint);
gp_float vv_absorption(device ClusterDynamicsMetalArgs&, uint);
gp_float i_bias_factor(device ClusterDynamicsMetalArgs&, uint);
gp_float v_bias_factor(device ClusterDynamicsMetalArgs&, uint);
gp_float i_binding_energy(device ClusterDynamicsMetalArgs&, uint);
gp_float v_binding_energy(device ClusterDynamicsMetalArgs&, uint);
gp_float i_diffusion(device ClusterDynamicsMetalArgs&);
gp_float v_diffusion(device ClusterDynamicsMetalArgs&);
gp_float dislocation_promotion_probability(device ClusterDynamicsMetalArgs&, uint);
gp_float cluster_radius(device ClusterDynamicsMetalArgs&, uint);

// Simulation Operation Functions
gp_float dislocation_density_delta(device ClusterDynamicsMetalArgs&);
gp_float mean_dislocation_cell_radius(device ClusterDynamicsMetalArgs&);
gp_float ii_sum_absorption(device ClusterDynamicsMetalArgs&, uint);
gp_float iv_sum_absorption(device ClusterDynamicsMetalArgs&, uint);
gp_float vv_sum_absorption(device ClusterDynamicsMetalArgs&, uint);
gp_float vi_sum_absorption(device ClusterDynamicsMetalArgs&, uint);
void step_init(device ClusterDynamicsMetalArgs&);
bool validate(device ClusterDynamicsMetalArgs&, uint);

kernel void update_clusters(
    device ClusterDynamicsMetalArgs& args,
    device NuclearReactorImpl& reactor,
    device MaterialImpl& material,
    device gp_float* interstitials_in,
    device gp_float* vacancies_in,
    device gp_float* interstitials_out,
    device gp_float* vacancies_out,
    uint index [[thread_position_in_grid]]
    )
{
    if (index > 1)
    {
        args.reactor = &reactor;
        args.material = &material;
        args.interstitials = interstitials_in;
        args.vacancies = vacancies_in;

        interstitials_out[index] += i_clusters_delta(args, index) * args.delta_time;
        vacancies_out[index] += v_clusters_delta(args, index) * args.delta_time;
    }
}

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of interstital defects from the irradiation cascade for size (args, n) clusters.
*/
gp_float i_defect_production(device ClusterDynamicsMetalArgs& args, uint n)
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
gp_float v_defect_production(device ClusterDynamicsMetalArgs& args, uint n)
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
gp_float i_clusters_delta(device ClusterDynamicsMetalArgs& args, uint in)
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
gp_float v_clusters_delta(device ClusterDynamicsMetalArgs& args, uint vn)
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
gp_float iemission_vabsorption_np1(device ClusterDynamicsMetalArgs& args, uint np1)
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
gp_float vemission_iabsorption_np1(device ClusterDynamicsMetalArgs& args, uint np1)
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
gp_float iemission_vabsorption_n(device ClusterDynamicsMetalArgs& args, uint n)
{
    return
        // (1)
        iv_absorption(args, n) * args.vacancies[1]
        // (2)
        + ii_absorption(args, n) * args.interstitials[1]
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
gp_float vemission_iabsorption_n(device ClusterDynamicsMetalArgs& args, uint n)
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
gp_float iemission_vabsorption_nm1(device ClusterDynamicsMetalArgs& args, uint nm1)
{
    return
        // (1)
        ii_absorption(args, nm1)
        // (2)
        * args.interstitials[1]
        // (3)
        * (1 - dislocation_promotion_probability(args, n));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2d
    The rate that a vacancy loop of size n - 1 can evolve into a
    loop of size n by absorbing a vacancy.

               (1)           (2)
    c[v,n-1] = B[v,v](n-1) * Cv(1)
*/
gp_float vemission_iabsorption_nm1(device ClusterDynamicsMetalArgs& args, uint nm1)
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
gp_float i1_cluster_delta(device ClusterDynamicsMetalArgs& args)
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
gp_float v1_cluster_delta(device ClusterDynamicsMetalArgs& args)
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
    tEi(args, n) = SUM ( E[i,i](args, n) * Ci(args, n) ) + 4 * E[i,i](2) * Ci(2) + B[i,v](2) * Cv(1) * Ci(2)
*/
gp_float i_emission_time(device ClusterDynamicsMetalArgs& args)
{
   gp_float time = 0.;
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
      + iv_absorption(args, 2) * args.vacancies[1] * args.interstitials[2];

  return time;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                           (2)                     (3)
    tEv(args, n) = SUM[n>0] ( E[v,v](args, n) * Cv(args, n) ) + 4 * E[v,v](2) * Cv(2) + B[v,i](2) * Cv(2) * Ci(1)
*/
gp_float v_emission_time(device ClusterDynamicsMetalArgs& args)
{
   gp_float time = 0.;
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
      + vi_absorption(args, 2) * args.vacancies[2] * args.interstitials[1];

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
gp_float i_absorption_time(device ClusterDynamicsMetalArgs& args)
{
   gp_float time = ii_absorption(args, 1) * args.interstitials[1];
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
gp_float v_absorption_time(device ClusterDynamicsMetalArgs& args)
{
   gp_float time = vv_absorption(args, 1) * args.vacancies[1];
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
gp_float annihilation_rate(device ClusterDynamicsMetalArgs& args)
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
gp_float i_dislocation_annihilation_time(device ClusterDynamicsMetalArgs& args)
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
gp_float v_dislocation_annihilation_time(device ClusterDynamicsMetalArgs& args)
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
    tAdi = 6 * Di * metal::precise::sqrt( p * Zi + SUM[n] ( B[i,i](args, n) * Ci(args, n) ) + SUM[n] ( B[v,i](args, n) * Cv(args, n) ) ) / d
*/
gp_float i_grain_boundary_annihilation_time(device ClusterDynamicsMetalArgs& args)
{
    return
        // (1)
        6. * i_diffusion(args) *
        metal::precise::sqrt
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
    tAdv = 6 * Di * metal::precise::sqrt( p * Zv + SUM[n] ( B[v,v](args, n) * Cv(args, n) ) + SUM[n] ( B[i,v](args, n) * Ci(args, n) ) ) / d
*/
gp_float v_grain_boundary_annihilation_time(device ClusterDynamicsMetalArgs& args)
{
    return
        // (1)
        6. * v_diffusion(args) *
        metal::precise::sqrt
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
gp_float ii_emission(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        i_bias_factor(args, n) *
        i_diffusion(args) / args.material->atomic_volume *
        metal::precise::exp
        (
            -i_binding_energy(args, n) /
            (BOLTZMANN_EV_KELVIN * args.reactor->temperature)
        );
}


/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4b
    Rate of absorption of an interstitial by an interstital loop of size (args, n).
*/
gp_float ii_absorption(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        i_bias_factor(args, n) *
        i_diffusion(args);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4c
    Rate of absorption of an interstitial by a vacancy loop of size (args, n).
*/
gp_float iv_absorption(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        v_bias_factor(args, n) *
        v_diffusion(args);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4d
    Rate of emission of a vacancy by a vacancy loop of size (args, n).
*/
gp_float vv_emission(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        v_bias_factor(args, n) *
        v_diffusion(args) *
        metal::precise::exp
        (
            -v_binding_energy(args, n) /
            (BOLTZMANN_EV_KELVIN * args.reactor->temperature)
        );
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4e
    Rate of absorption of a vacancy by a vacancy loop of size (args, n).
*/
gp_float vv_absorption(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        v_bias_factor(args, n) *
        v_diffusion(args);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4f
    Rate of absorption of a vacancy by an interstitial loop of size (args, n).
*/
gp_float vi_absorption(device ClusterDynamicsMetalArgs& args, uint n)
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
gp_float i_bias_factor(device ClusterDynamicsMetalArgs& args, uint in)
{
    return 
        args.material->i_dislocation_bias +
        (
            metal::precise::sqrt
            (
                    args.material->burgers_vector /
                    (8. * M_PI * args.material->lattice_param)
            ) *
            args.material->i_loop_bias -
            args.material->i_dislocation_bias
        ) *
        1. /
        metal::precise::pow
        (
            (gp_float)in,
            args.material->i_dislocation_bias_param / 2.
        );
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Vacancy bias factor.
*/
gp_float v_bias_factor(device ClusterDynamicsMetalArgs& args, uint vn)
{
    return 
        args.material->v_dislocation_bias +
        (
            metal::precise::sqrt
            (
                    args.material->burgers_vector /
                    (8. * M_PI * args.material->lattice_param)
            ) *
            args.material->v_loop_bias -
            args.material->v_dislocation_bias
        ) *
        1. /
        metal::precise::pow
        (
            (gp_float)vn,
            args.material->v_dislocation_bias_param / 2.
        );
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Interstitial binding energy.
*/
gp_float i_binding_energy(device ClusterDynamicsMetalArgs& args, uint in)
{
    return
        args.material->i_formation
        + (args.material->i_binding - args.material->i_formation) / (metal::precise::pow(2., .8) - 1.) *
        (metal::precise::pow((gp_float)in, .8) - metal::precise::pow((gp_float)in - 1., .8));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Vacancy binding energy.
*/
gp_float v_binding_energy(device ClusterDynamicsMetalArgs& args, uint vn)
{
    return
        args.material->v_formation
        + (args.material->v_binding - args.material->v_formation) / (metal::precise::pow(2., .8) - 1) *
        (metal::precise::pow((gp_float)vn, .8) - metal::precise::pow((gp_float)vn - 1., .8));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
gp_float i_diffusion(device ClusterDynamicsMetalArgs& args)
{
    return args.material->i_diffusion_0 * metal::precise::exp(-args.material->i_migration / (BOLTZMANN_EV_KELVIN * args.reactor->temperature));
}

/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
gp_float v_diffusion(device ClusterDynamicsMetalArgs& args)
{
   return args.material->v_diffusion_0 * metal::precise::exp(-args.material->v_migration / (BOLTZMANN_EV_KELVIN * args.reactor->temperature));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.10
*/
gp_float mean_dislocation_cell_radius(device ClusterDynamicsMetalArgs& args)
{
   gp_float r_0_factor = 0.;
   for (uint i = 1; i < args.concentration_boundary; ++i)
   {
      r_0_factor += cluster_radius(args, i) * args.interstitials[i];
   }

   return 1 / metal::precise::sqrt((2. * M_PI * M_PI / args.material->atomic_volume) * r_0_factor + M_PI * args.dislocation_density);
}

// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.12
*/
gp_float dislocation_promotion_probability(device ClusterDynamicsMetalArgs& args, uint n)
{
   gp_float dr = cluster_radius(args, n + 1) - cluster_radius(args, n);

   return (2. * cluster_radius(args, n) * dr + metal::precise::pow(dr, 2.)) 
      / (metal::precise::pow(M_PI * args.mean_dislocation_radius_val / 2., 2.) - metal::precise::pow(cluster_radius(args, n), 2.)); 
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 8
*/
gp_float dislocation_density_delta(device ClusterDynamicsMetalArgs& args)
{
   gp_float gain = 0.0;
   for (uint n = 1; n < args.concentration_boundary; ++n)
   {
      gain += cluster_radius(args, n) * dislocation_promotion_probability(args, n) * ii_absorption(args, n) * args.interstitials[n];
   }

   gain *= 2. * M_PI / args.material->atomic_volume;

   return 
      gain
      - args.reactor->dislocation_density_evolution * 
      metal::precise::pow(args.material->burgers_vector, 2.) *
      metal::precise::pow(args.dislocation_density, 3. / 2.);
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg. 346, 7.63
*/
gp_float cluster_radius(device ClusterDynamicsMetalArgs& args, uint n)
{
    return metal::precise::sqrt(metal::precise::sqrt(3.) * metal::precise::pow(args.material->lattice_param, 2.) * (gp_float)n / (4. * M_PI));
}
// --------------------------------------------------------------------------------------------


gp_float ii_sum_absorption(device ClusterDynamicsMetalArgs& args, uint nmax)
{
   gp_float emission = 0.;
   for (uint vn = 1; vn < nmax; ++vn)
   {
      emission += ii_absorption(args, vn) * args.interstitials[vn];
   }

   return emission;
}

gp_float iv_sum_absorption(device ClusterDynamicsMetalArgs& args, uint nmax)
{
   gp_float emission = 0.;
   for (uint n = 1; n < nmax; ++n)
   {
      emission += iv_absorption(args, n) * args.interstitials[n];
   }

   return emission;
}

gp_float vv_sum_absorption(device ClusterDynamicsMetalArgs& args, uint nmax)
{
   gp_float emission = 0.;
   for (uint n = 1; n < nmax; ++n)
   {
      emission += vv_absorption(args, n) * args.vacancies[n];
   }

   return emission;
}

gp_float vi_sum_absorption(device ClusterDynamicsMetalArgs& args, uint nmax)
{
   gp_float emission = 0.;
   for (uint n = 1; n < nmax; ++n)
   {
      emission += vi_absorption(args, n) * args.vacancies[n];
   }

   return emission;
}