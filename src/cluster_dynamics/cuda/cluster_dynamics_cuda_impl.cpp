#include <stdio.h>
#include <cstring>

#include "cluster_dynamics_cuda_impl.hpp"

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of interstital defects from the irradiation cascade for size (n) clusters.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_defect_production(size_t n)
{
    switch (n)
    {
        case 1: return reactor.get_recombination() * reactor.get_flux() *
                       (1. - reactor.get_i_bi() - reactor.get_i_tri() - reactor.get_i_quad());
        case 2: return reactor.get_recombination() * reactor.get_flux() * reactor.get_i_bi();
        case 3: return reactor.get_recombination() * reactor.get_flux() * reactor.get_i_tri();
        case 4: return reactor.get_recombination() * reactor.get_flux() * reactor.get_i_quad();

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.;
};

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of vacancy defects from the irradiation cascade for size (n) clusters.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_defect_production(size_t n)
{
    switch (n)
    {
        case 1: return reactor.get_recombination() * reactor.get_flux() *
                       (1. - reactor.get_v_bi() - reactor.get_v_tri() - reactor.get_v_quad());
        case 2: return reactor.get_recombination() * reactor.get_flux() * reactor.get_v_bi();
        case 3: return reactor.get_recombination() * reactor.get_flux() * reactor.get_v_tri();
        case 4: return reactor.get_recombination() * reactor.get_flux() * reactor.get_v_quad();

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
__CUDADECL__ gp_float ClusterDynamicsImpl::i_clusters_delta(size_t in)
{
    return
        // (1)
        i_defect_production(in)
        // (2)
        + iemission_vabsorption_np1(in + 1) * interstitials[in + 1]
        // // (3)
        - iemission_vabsorption_n(in) * interstitials[in]
        // // (4)
        + iemission_vabsorption_nm1(in - 1) * interstitials[in - 1];
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (vn) vacancies per unit volume.

                  (1)     (2)                    (3)              (4)
    dCv(n) / dt = Gv(n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(n) + c[v,n-1] * Cv(n-1)
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_clusters_delta(size_t vn)
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
__CUDADECL__ gp_float ClusterDynamicsImpl::iemission_vabsorption_np1(size_t np1)
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
__CUDADECL__ gp_float ClusterDynamicsImpl::vemission_iabsorption_np1(size_t np1)
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
__CUDADECL__ gp_float ClusterDynamicsImpl::iemission_vabsorption_n(size_t n)
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
__CUDADECL__ gp_float ClusterDynamicsImpl::vemission_iabsorption_n(size_t n)
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
__CUDADECL__ gp_float ClusterDynamicsImpl::iemission_vabsorption_nm1(size_t nm1)
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
__CUDADECL__ gp_float ClusterDynamicsImpl::vemission_iabsorption_nm1(size_t nm1)
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
gp_float ClusterDynamicsImpl::i1_cluster_delta()
{
    return 
        // (1)
        i_defect_production(1)
        // (2)
        - annihilation_rate() * interstitials[1] * vacancies[1]
        // (3)
        - interstitials[1] * i_dislocation_annihilation_time()
        // (4)
        - interstitials[1] * i_grain_boundary_annihilation_time()
        // (5)
        - interstitials[1] * i_absorption_time()
        // (6)
        + i_emission_time();
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
gp_float ClusterDynamicsImpl::v1_cluster_delta()
{
    return 
        // (1)
        v_defect_production(1)
        // (2)
        - annihilation_rate() * interstitials[1] * vacancies[1]
        // (3)
        - vacancies[1] * v_dislocation_annihilation_time()
        // (4)
        - vacancies[1] * v_grain_boundary_annihilation_time()
        // (5)
        - vacancies[1] * v_absorption_time();
        // (6)
        + v_emission_time();
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting an interstitial by the population of interstital or vacancy
    clusters of size up to (nmax).

                (1)                      (2)                     (3)
    tEi(n) = SUM ( E[i,i](n) * Ci(n) ) + 4 * E[i,i](2) * Ci(2) + B[i,v](2) * Cv(2) * Ci(2)
*/
gp_float ClusterDynamicsImpl::i_emission_time()
{
  auto self = this->self;
  gp_float time = thrust::transform_reduce(indices.begin() + 2, indices.end(), [self] __CUDADECL__ (const int& idx){
    return self->ii_emission(idx) * self->interstitials[idx];
  }, 0.0, thrust::plus<gp_float>());

  time +=
      // (2)
      4. * ii_emission(2) * interstitials[2]
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
gp_float ClusterDynamicsImpl::v_emission_time()
{
  auto self = this->self;
  gp_float time = thrust::transform_reduce(indices.begin() + 2, indices.end(), [self] __CUDADECL__ (const int& idx){
    return self->vv_emission(idx) * self->vacancies[idx];
  }, 0.0, thrust::plus<gp_float>());

  time +=
      // (2)
      4. * vv_emission(2) * vacancies[2]
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
gp_float ClusterDynamicsImpl::i_absorption_time()
{
  auto self = this->self;
  gp_float time = ii_absorption(1) * interstitials[1];
  time += thrust::transform_reduce( indices.begin() + 1, indices.end(), 
  [self] __CUDADECL__ (const int& idx) {
    return self->ii_absorption(idx) * self->interstitials[idx] + self->vi_absorption(idx) * self->vacancies[idx];
  }, 
  0.0, thrust::plus<gp_float>());

  return time;
}


/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing a vacancy by the population of interstital or vacancy
    clusters of size up to (nmax).

                        (1)                              (2)
    tAv(n) = SUM[n>0] ( B[v,v](n) * Cv(n) ) + SUM[n>1] ( B[i,v](n) * Ci(n) )
*/
gp_float ClusterDynamicsImpl::v_absorption_time()
{
  auto self = this->self;
  gp_float time = vv_absorption(1) * vacancies[1];
  time += thrust::transform_reduce(indices.begin() + 1, indices.end(), [self] __CUDADECL__ (const int& idx) {
    return self->vv_absorption(idx) * self->vacancies[idx] + self->iv_absorption(idx) * self->interstitials[idx];
  }, 0.0, thrust::plus<gp_float>());

  return time;
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3d
    Annihilation rate of vacancies and insterstitals.

          (1)      (2)         (3)
    Riv = 4 * PI * (Di + Dv) * riv
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::annihilation_rate()
{
    return 
        // (1)
        4. * M_PI * 
        // (2)
        (i_diffusion() + v_diffusion()) *
        // (3)
        material.get_recombination_radius();
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of interstitials on dislocations.

           (1)   (2)    (3)
    tAdi = p  *  Di  *  Zi
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_dislocation_annihilation_time()
{
    return
        // (1)
        dislocation_density *
        // (2)
        i_diffusion() *
        // (3)
        material.get_i_dislocation_bias();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of vacancies on dislocations.

           (1)   (2)    (3)
    tAdv = p  *  Dv  *  Zv
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_dislocation_annihilation_time()
{
    return
        // (1)
        dislocation_density *
        // (2)
        v_diffusion() *
        // (3)
        material.get_v_dislocation_bias();
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdi = 6 * Di * sqrt( p * Zi + SUM[n] ( B[i,i](n) * Ci(n) ) + SUM[n] ( B[v,i](n) * Cv(n) ) ) / d
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_grain_boundary_annihilation_time()
{
    return
        // (1)
        6. * i_diffusion() *
        sqrt
        (
            // (2)
            dislocation_density * 
            material.get_i_dislocation_bias()
            // (3)
            + ii_sum_absorption_val
            // (4)
            + vi_sum_absorption_val
        ) /
        // (5)
        material.get_grain_size();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4)                              (5)
    tAdv = 6 * Di * sqrt( p * Zv + SUM[n] ( B[v,v](n) * Cv(n) ) + SUM[n] ( B[i,v](n) * Ci(n) ) ) / d
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_grain_boundary_annihilation_time()
{
    return
        // (1)
        6. * v_diffusion() *
        sqrt
        (
            // (2)
            dislocation_density *
            material.get_v_dislocation_bias()
            // (3)
            + vv_sum_absorption_val
            // (4)
            + iv_sum_absorption_val
        ) /
        // (5)
        material.get_grain_size();
}

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4a
    Rate of emission of an interstitial by an interstital loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::ii_emission(size_t n)
{
    return 
        2. * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        i_diffusion() / material.get_atomic_volume() *
        exp
        (
            -i_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor.get_temperature())
        );
}


/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4b
    Rate of absorption of an interstitial by an interstital loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::ii_absorption(size_t n)
{
    return 
        2. * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        i_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4c
    Rate of absorption of an interstitial by a vacancy loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::iv_absorption(size_t n)
{
    return 
        2. * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        v_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4d
    Rate of emission of a vacancy by a vacancy loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::vv_emission(size_t n)
{
    return 
        2. * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        v_diffusion() *
        exp
        (
            -v_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor.get_temperature())
        );
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4e
    Rate of absorption of a vacancy by a vacancy loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::vv_absorption(size_t n)
{
    return 
        2. * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        v_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4f
    Rate of absorption of a vacancy by an interstitial loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::vi_absorption(size_t n)
{
    return 
        2. * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        i_diffusion();
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Interstitial bias factor.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_bias_factor(size_t in)
{
    return 
        material.get_i_dislocation_bias() +
        (
            std::sqrt
            (
                    material.get_burgers_vector() /
                    (8. * M_PI * material.get_lattice_param())
            ) *
            material.get_i_loop_bias() -
            material.get_i_dislocation_bias()
        ) *
        1 /
        std::pow
        (
            (gp_float)in,
            material.get_i_dislocation_bias_param() / 2.
        );
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Vacancy bias factor.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_bias_factor(size_t vn)
{
    return 
        material.get_v_dislocation_bias() +
        (
            std::sqrt
            (
                    material.get_burgers_vector() /
                    (8. * M_PI * material.get_lattice_param())
            ) *
            material.get_v_loop_bias() -
            material.get_v_dislocation_bias()
        ) *
        1 /
        std::pow
        (
            (gp_float)vn,
            material.get_v_dislocation_bias_param() / 2.
        );
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Interstitial binding energy.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_binding_energy(size_t in)
{
    return
        material.get_i_formation()
        + (material.get_i_binding() - material.get_i_formation()) / (std::pow(2., .8) - 1) *
        (std::pow((gp_float)in, .8) - std::pow((gp_float)in - 1., .8));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Vacancy binding energy.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_binding_energy(size_t vn)
{
    return
        material.get_v_formation()
        + (material.get_v_binding() - material.get_v_formation()) / (std::pow(2., .8) - 1) *
        (std::pow((gp_float)vn, .8) - std::pow((gp_float)vn - 1., .8));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
__CUDADECL__ gp_float ClusterDynamicsImpl::i_diffusion()
{
    return material.get_i_diffusion_0() * std::exp(-material.get_i_migration() / (BOLTZMANN_EV_KELVIN * reactor.get_temperature()));
}

/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
__CUDADECL__ gp_float ClusterDynamicsImpl::v_diffusion()
{
    return material.get_v_diffusion_0() * std::exp(-material.get_v_migration() / (BOLTZMANN_EV_KELVIN * reactor.get_temperature()));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.10
*/
gp_float ClusterDynamicsImpl::mean_dislocation_cell_radius()
{
  auto self = this->self;

  gp_float r_0_factor = thrust::transform_reduce(indices.begin(), indices.end(), 
  [self] __CUDADECL__ (const int& idx){
    return self->cluster_radius(idx) * self->interstitials[idx];
  }, 0.0, thrust::plus<gp_float>());

  return 1 / std::sqrt((2. * M_PI * M_PI / material.get_atomic_volume()) * r_0_factor + M_PI * dislocation_density);
}

// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.12
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::dislocation_promotion_probability(size_t n)
{
    gp_float dr = cluster_radius(n + 1) - cluster_radius(n);

    return (2. * cluster_radius(n) * dr + std::pow(dr, 2.)) 
         / (M_PI * mean_dislocation_radius_val / 2. - std::pow(cluster_radius(n), 2.)); 
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 8
*/
gp_float ClusterDynamicsImpl::dislocation_density_delta()
{
  auto self = this->self;
  gp_float gain = thrust::transform_reduce(
    indices.begin(), indices.end(), [self] __CUDADECL__ (const int& idx){
      return self->cluster_radius(idx) * self->dislocation_promotion_probability(idx) * self->ii_absorption(idx) * self->interstitials[idx];
    }, 
    0.0, thrust::plus<gp_float>());

  gain *= 2. * M_PI / material.get_atomic_volume();

  return 
      gain
      - reactor.get_dislocation_density_evolution() * 
      std::pow(material.get_burgers_vector(), 2.) *
      std::pow(dislocation_density, 3. / 2.);
}

// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg. 346, 7.63
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::cluster_radius(size_t n)
{
    return std::sqrt(std::sqrt(3.) * std::pow(material.get_lattice_param(), 2.) * (gp_float)n / (4. * M_PI));
}
// --------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  SIMULATION CONTROL FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

void ClusterDynamicsImpl::step_init()
{
  mean_dislocation_radius_val = mean_dislocation_cell_radius();
  ii_sum_absorption_val = ii_sum_absorption(concentration_boundary - 1);
  iv_sum_absorption_val = iv_sum_absorption(concentration_boundary - 1);
  vi_sum_absorption_val = vi_sum_absorption(concentration_boundary - 1);
  vv_sum_absorption_val = vv_sum_absorption(concentration_boundary - 1);
}

bool ClusterDynamicsImpl::step(gp_float delta_time)
{
  step_init();

  bool state_is_valid = update_clusters_1(delta_time);
  update_clusters(delta_time);
  dislocation_density += dislocation_density_delta() * delta_time;

  interstitials = interstitials_temp;
  vacancies = vacancies_temp;

  cudaMemcpy(thrust::raw_pointer_cast(self), this, sizeof(ClusterDynamicsImpl), cudaMemcpyHostToDevice);

  return state_is_valid;
}

bool ClusterDynamicsImpl::update_clusters_1(gp_float delta_time)
{
  interstitials_temp[1] += i1_cluster_delta() * delta_time;
  vacancies_temp[1] += v1_cluster_delta() * delta_time;
  return validate(1);
}

bool ClusterDynamicsImpl::update_clusters(gp_float delta_time)
{
  auto self = this->self;
  thrust::transform(indices.begin() + 1, indices.end(), interstitials_temp.begin() + 2, 
    [self, delta_time] __CUDADECL__ (const int& idx){
      return self->interstitials[idx] + self->i_clusters_delta(idx) * delta_time;
    });
  thrust::transform(indices.begin() + 1, indices.end(), vacancies_temp.begin() + 2, 
    [self, delta_time] __CUDADECL__ (const int& idx){
      return self->vacancies[idx] + self->v_clusters_delta(idx) * delta_time;
    });

  return true; // TODO - Validate results
}

ClusterDynamicsImpl::~ClusterDynamicsImpl()
{
  thrust::device_free(self);
}

gp_float ClusterDynamicsImpl::ii_sum_absorption(size_t nmax)
{
  auto self = this->self;
  return thrust::transform_reduce(indices.begin(), indices.end(),
    [self] __CUDADECL__ (const int& idx) { 
      return self->ii_absorption(idx) * self->interstitials[idx];
    }, 
    0.0, thrust::plus<gp_float>());
}

gp_float ClusterDynamicsImpl::iv_sum_absorption(size_t nmax)
{
  auto self = this->self;
  return thrust::transform_reduce(indices.begin(), indices.end(), 
    [self] __CUDADECL__ (const int& idx) {
        return self->iv_absorption(idx) * self->interstitials[idx];
    }, 
    0.0, thrust::plus<gp_float>());
}

gp_float ClusterDynamicsImpl::vv_sum_absorption(size_t nmax)
{
  auto self = this->self;
  return thrust::transform_reduce(indices.begin(), indices.end(), 
    [self] __CUDADECL__ (const int& idx) {
        return self->vv_absorption(idx) * self->vacancies[idx];
    }, 
    0.0, thrust::plus<gp_float>());
}

gp_float ClusterDynamicsImpl::vi_sum_absorption(size_t nmax)
{
  auto self = this->self;
  return thrust::transform_reduce(indices.begin(), indices.end(), 
    [self] __CUDADECL__ (const int& idx) {
        return self->vi_absorption(idx) * self->vacancies[idx];
    }, 
    0.0, thrust::plus<gp_float>());
}

bool ClusterDynamicsImpl::validate(size_t n)
{
    return 
        !std::isnan(interstitials_temp[n]) &&
        !std::isinf(interstitials_temp[n]) &&
        !std::isnan(vacancies_temp[n]) &&
        !std::isinf(vacancies_temp[n]) &&
        !(interstitials_temp[n] < 0) &&
        !(vacancies_temp[n] < 0);
}





// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  PUBLIC INTERFACE FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------



// TODO - clean up the uses of random +1/+2/-1/etc throughout the code
ClusterDynamicsImpl::ClusterDynamicsImpl(size_t concentration_boundary, const NuclearReactor& reactor, const Material& material)
  : concentration_boundary(concentration_boundary), reactor(reactor), material(material),
    interstitials(concentration_boundary + 1, 0.0), vacancies(concentration_boundary + 1, 0.0),
    interstitials_temp(concentration_boundary + 1, 0.0), vacancies_temp(concentration_boundary + 1, 0.0),
    indices(concentration_boundary - 1, 0.0), dislocation_density(material.get_dislocation_density_0()), time(0.0)
{
  ClusterDynamicsImpl* ptr;
  cudaMalloc(&ptr, sizeof(ClusterDynamicsImpl));
  self = thrust::device_ptr<ClusterDynamicsImpl>(ptr);
  cudaMemcpy(thrust::raw_pointer_cast(self), this, sizeof(ClusterDynamicsImpl), cudaMemcpyHostToDevice);
  thrust::sequence(indices.begin(), indices.end(), 1);
}

ClusterDynamicsState ClusterDynamicsImpl::run(gp_float delta_time, gp_float total_time)
{
    bool state_is_valid = true;

    for (gp_float endtime = time + total_time; time < endtime; time += delta_time)
    {
        state_is_valid = step(delta_time);
        if (!state_is_valid) break;
    }

    thrust::host_vector<gp_float> is(interstitials);
    thrust::host_vector<gp_float> vs(vacancies);

    return ClusterDynamicsState 
    {
        .valid = state_is_valid,
        .time = time,
        .interstitials = std::vector<gp_float>(is.begin(), is.end() - 1),
        .vacancies = std::vector<gp_float>(vs.begin(), vs.end() - 1),
        .dislocation_density = dislocation_density
    };
}

Material ClusterDynamicsImpl::get_material()
{
    return material;
}

void ClusterDynamicsImpl::set_material(const Material& material)
{
    this->material = Material(material);
}

NuclearReactor ClusterDynamicsImpl::get_reactor()
{
    return reactor;
}

void ClusterDynamicsImpl::set_reactor(const NuclearReactor& reactor)
{
    this->reactor = NuclearReactor(reactor);
}