#include "cluster_dynamics_cuda_impl.hpp"

#include <stdio.h>

#include <algorithm>
#include <cstring>

#include "cluster_dynamics/cluster_dynamics.hpp"

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of interstital defects from the irradiation cascade
    for size (n) clusters.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_defect_production(size_t n) const {
  switch (n) {
    case 1:
      return reactor.recombination * reactor.flux *
             (1. - reactor.i_bi - reactor.i_tri - reactor.i_quad);
    case 2:
      return reactor.recombination * reactor.flux * reactor.i_bi;
    case 3:
      return reactor.recombination * reactor.flux * reactor.i_tri;
    case 4:
      return reactor.recombination * reactor.flux * reactor.i_quad;

    default:
      break;
  }

  // cluster sizes > greater than 4 always zero
  return 0.;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of vacancy defects from the irradiation cascade for
    size (n) clusters.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_defect_production(size_t n) const {
  switch (n) {
    case 1:
      return reactor.recombination * reactor.flux *
             (1. - reactor.v_bi - reactor.v_tri - reactor.v_quad);
    case 2:
      return reactor.recombination * reactor.flux * reactor.v_bi;
    case 3:
      return reactor.recombination * reactor.flux * reactor.v_tri;
    case 4:
      return reactor.recombination * reactor.flux * reactor.v_quad;

    default:
      break;
  }

  // cluster sizes > greater than 4 always zero
  return 0.;
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (in) interstitials per unit volume.

                  (1)     (2)                    (3)              (4)
    dCi(n) / dt = Gi(n) + a[i,n+1] * Ci(n + 1) - b[i,n] * Ci(n) + c[i,n-1] *
    Ci(n-1)
*/
__CUDADECL__ gp_float
ClusterDynamicsImpl::i_concentration_derivative(size_t in) const {

  return
      // (1)
      i_defect_production(in) / material.atomic_volume
      // (2)
      + i_demotion_rate(in + 1) * interstitials[in + 1]
      // (3)
      - i_combined_promotion_demotion_rate(in) * interstitials[in]
      // (4)
      + i_promotion_rate(in - 1) * interstitials[in - 1];
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2a
    The number of clusters that contain (vn) vacancies per unit volume.

                  (1)     (2)                    (3)              (4)
    dCv(n) / dt = Gv(n) + a[v,n+1] * Cv(n + 1) - b[v,n] * Cv(n) + c[v,n-1] *
    Cv(n-1)
*/
__CUDADECL__ gp_float
ClusterDynamicsImpl::v_concentration_derivative(size_t vn) const {
  return
      // (1)
      v_defect_production(vn) / material.atomic_volume
      // (2)
      + v_demotion_rate(vn + 1) * vacancies[vn + 1]
      // (3)
      - v_combined_promotion_demotion_rate(vn) * vacancies[vn]
      // (4)
      + v_promotion_rate(vn - 1) * vacancies[vn - 1];
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption
    of a vacancy by an interstitial loop of size (n),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[i,n+1] = B[i,v](n + 1) * Cv(1) + E[i,i](n + 1)
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_demotion_rate(size_t n) const {
  return
      // (1)
      iv_absorption(n) *
          // (2)
          vacancies[1] +
      // (3)
      ii_emission(n);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2b
    The combined rate of emission of an interstitial and absorption of a vacancy
    by an interstitial loop of size (n),
    both events leading to an interstitial loop of size n.

               (1)             (2)     (3)
    a[v,n+1] = B[v,i](n + 1) * Ci(1) + E[v,v](n + 1)
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_demotion_rate(size_t n) const {
  return
      // (1)
      vi_absorption(n) *
          // (2)
          interstitials[1] +
      // (3)
      vv_emission(n);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2c
    The rate that a loop of size n can evolve toward a loop of size
    n + 1 absorbing an interstitial, or toward a loop of size
    n - 1 absorbing a vacancy or emitting an interstitial.

             (1)                 (2)                 (3)
    b[i,n] = B[i,v](n) * Cv(1) + B[i,i](n) * Ci(1) + E[i,i](n)
*/
__CUDADECL__ gp_float
ClusterDynamicsImpl::i_combined_promotion_demotion_rate(size_t n) const {
  return
      // (1)
      iv_absorption(n) * vacancies[1]
      // (2)
      + ii_absorption(n) * interstitials[1]
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
__CUDADECL__ gp_float
ClusterDynamicsImpl::v_combined_promotion_demotion_rate(size_t n) const {
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

               (1)           (2)     (3)
    c[i,n-1] = B[i,i](n-1) * Ci(1) * P_unf(n)
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_promotion_rate(size_t n) const {
  return
      // (1)
      ii_absorption(n)
      // (2)
      * interstitials[1]
      // (3)
      * (1 - dislocation_promotion_probability(n + 1));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 2d
    The rate that a vacancy loop of size n - 1 can evolve into a
    loop of size n by absorbing a vacancy.

               (1)           (2)
    c[v,n-1] = B[v,v](n-1) * Cv(1)
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_promotion_rate(size_t n) const {
  return
      // (1)
      vv_absorption(n) *
      // (2)
      vacancies[1];
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3a
    Point defects concentrations per unit volume given a cluster size (in).
    ** in is only used in characteristic time calculations which rely on cluster
    sizes up to (in).

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
gp_float ClusterDynamicsImpl::i1_concentration_derivative() const {
  return
      // (1)
      i_defect_production(1) / material.atomic_volume
      // (2)
      - annihilation_rate() * host_interstitials[1] * host_vacancies[1]
      // (3)
      - host_interstitials[1] * i_dislocation_annihilation_rate()
      // (4)
      - host_interstitials[1] * i_grain_boundary_annihilation_rate()
      // (5)
      - host_interstitials[1] * i_absorption_rate()
      // (6)
      + i_emission_rate();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3a
    Point defects concentrations per unit volume given a cluster size (vn).
    ** vn is only used in characteristic time calculations which rely on cluster
    sizes up to (vn).

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
gp_float ClusterDynamicsImpl::v1_concentration_derivative() const {
  return
      // (1)
      v_defect_production(1) / material.atomic_volume
      // (2)
      - annihilation_rate() * host_interstitials[1] * host_vacancies[1]
      // (3)
      - host_vacancies[1] * v_dislocation_annihilation_rate()
      // (4)
      - host_vacancies[1] * v_grain_boundary_annihilation_rate()
      // (5)
      - host_vacancies[1] * v_absorption_rate()
      // (6)
      + v_emission_rate();
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting an interstitial by the population of
    interstital or vacancy clusters of size up to (nmax).

                (1)                      (2)                     (3)
    tEi(n) = SUM ( E[i,i](n) * Ci(n) ) + 4 * E[i,i](2) * Ci(2) + B[i,v](2) *
    Cv(1) * Ci(2)
*/
gp_float ClusterDynamicsImpl::i_emission_rate() const {
  auto self = this->self;
  gp_float time = thrust::transform_reduce(
      indices.begin() + 2, indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->ii_emission(idx) * self->interstitials[idx];
      },
      0.0, thrust::plus<gp_float>());

  time +=
      // (2)
      4. * ii_emission(2) * host_interstitials[2]
      // (3)
      + iv_absorption(2) * host_vacancies[1] * host_interstitials[2];

  return time;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3b
    Characteristic time for emitting a vacancy by the population of interstital
    or vacancy clusters of size up to (nmax).

                (1)                           (2)                     (3)
    tEv(n) = SUM[n>0] ( E[v,v](n) * Cv(n) ) + 4 * E[v,v](2) * Cv(2) + B[v,i](2)
    * Cv(2) * Ci(1)
*/
gp_float ClusterDynamicsImpl::v_emission_rate() const {
  auto self = this->self;
  gp_float time = thrust::transform_reduce(
      indices.begin() + 2, indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->vv_emission(idx) * self->vacancies[idx];
      },
      0.0, thrust::plus<gp_float>());

  time +=
      // (2)
      4. * vv_emission(2) * host_vacancies[2]
      // (3)
      + vi_absorption(2) * host_vacancies[2] * host_interstitials[1];

  return time;
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing an interstitial by the population of
    interstital or vacancy clusters of size up to (nmax).

                        (1)                              (2)
    tAi(n) = SUM[n>0] ( B[i,i](n) * Ci(n) ) + SUM[n>1] ( B[v,i](n) * Cv(n) )
*/
gp_float ClusterDynamicsImpl::i_absorption_rate() const {
  auto self = this->self;
  gp_float time = ii_absorption(1) * host_interstitials[1];
  time += thrust::transform_reduce(
      indices.begin() + 1, indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->ii_absorption(idx) * self->interstitials[idx] +
               self->vi_absorption(idx) * self->vacancies[idx];
      },
      0.0, thrust::plus<gp_float>());

  return time;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3c
    Characteristic time for absorbing a vacancy by the population of interstital
    or vacancy clusters of size up to (nmax).

                        (1)                              (2)
    tAv(n) = SUM[n>0] ( B[v,v](n) * Cv(n) ) + SUM[n>1] ( B[i,v](n) * Ci(n) )
*/
gp_float ClusterDynamicsImpl::v_absorption_rate() const {
  auto self = this->self;
  gp_float time = vv_absorption(1) * host_vacancies[1];
  time += thrust::transform_reduce(
      indices.begin() + 1, indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->vv_absorption(idx) * self->vacancies[idx] +
               self->iv_absorption(idx) * self->interstitials[idx];
      },
      0.0, thrust::plus<gp_float>());

  return time;
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3d
    Annihilation rate of vacancies and insterstitals.

          (1)      (2)         (3)
    Riv = 4 * PI * (Di + Dv) * riv
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::annihilation_rate() const {
  return
      // (1)
      4. * M_PI *
      // (2)
      (i_diffusion_val + v_diffusion_val) *
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
__CUDADECL__ gp_float
ClusterDynamicsImpl::i_dislocation_annihilation_rate() const {
  return
      // (1)
      dislocation_density *
      // (2)
      i_diffusion_val *
      // (3)
      material.i_dislocation_bias;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3e
    Characteristic time for annihilation of vacancies on dislocations.

           (1)   (2)    (3)
    tAdv = p  *  Dv  *  Zv
*/
__CUDADECL__ gp_float
ClusterDynamicsImpl::v_dislocation_annihilation_rate() const {
  return
      // (1)
      dislocation_density *
      // (2)
      v_diffusion_val *
      // (3)
      material.v_dislocation_bias;
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4) (5) tAdi =
    6 * Di * sqrt( p * Zi + SUM[n] ( B[i,i](n) * Ci(n) ) + SUM[n] ( B[v,i](n) *
    Cv(n) ) ) / d
*/
__CUDADECL__ gp_float
ClusterDynamicsImpl::i_grain_boundary_annihilation_rate() const {
  return
      // (1)
      6. * i_diffusion_val *
      sqrt(
          // (2)
          dislocation_density * material.i_dislocation_bias
          // (3)
          + ii_sum_absorption_val
          // (4)
          + vi_sum_absorption_val) /
      // (5)
      material.grain_size;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3f
    Characteristic time for annihilation of interstitials on grain boundaries.

           (1)            (2)      (3)                            (4) (5) tAdv =
    6 * Di * sqrt( p * Zv + SUM[n] ( B[v,v](n) * Cv(n) ) + SUM[n] ( B[i,v](n) *
    Ci(n) ) ) / d
*/
__CUDADECL__ gp_float
ClusterDynamicsImpl::v_grain_boundary_annihilation_rate() const {
  return
      // (1)
      6. * v_diffusion_val *
      sqrt(
          // (2)
          dislocation_density * material.v_dislocation_bias
          // (3)
          + vv_sum_absorption_val
          // (4)
          + iv_sum_absorption_val) /
      // (5)
      material.grain_size;
}

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4a
    Rate of emission of an interstitial by an interstital loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::ii_emission(size_t n) const {
  return 2. * M_PI * cluster_radius(n) * i_bias_factor(n) * i_diffusion_val /
         material.atomic_volume *
         exp(-i_binding_energy(n) /
             (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4b
    Rate of absorption of an interstitial by an interstital loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::ii_absorption(size_t n) const {
  return 2. * M_PI * cluster_radius(n) * i_bias_factor(n) * i_diffusion_val;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4c
    Rate of absorption of an interstitial by a vacancy loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::iv_absorption(size_t n) const {
  return 2. * M_PI * cluster_radius(n) * v_bias_factor(n) * v_diffusion_val;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4d
    Rate of emission of a vacancy by a vacancy loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::vv_emission(size_t n) const {
  return 2. * M_PI * cluster_radius(n) * v_bias_factor(n) * v_diffusion_val *
         exp(-v_binding_energy(n) /
             (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4e
    Rate of absorption of a vacancy by a vacancy loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::vv_absorption(size_t n) const {
  return 2. * M_PI * cluster_radius(n) * v_bias_factor(n) * v_diffusion_val;
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4f
    Rate of absorption of a vacancy by an interstitial loop of size (n).
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::vi_absorption(size_t n) const {
  return 2. * M_PI * cluster_radius(n) * i_bias_factor(n) * i_diffusion_val;
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Interstitial bias factor.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_bias_factor(size_t in) const {
  return material.i_dislocation_bias +
         (std::sqrt(material.burgers_vector /
                    (8. * M_PI * material.lattice_param)) *
              material.i_loop_bias -
          material.i_dislocation_bias) *
             1 / std::pow((gp_float)in, material.i_dislocation_bias_param / 2.);
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 5
    Vacancy bias factor.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_bias_factor(size_t vn) const {
  return material.v_dislocation_bias +
         (std::sqrt(material.burgers_vector /
                    (8. * M_PI * material.lattice_param)) *
              material.v_loop_bias -
          material.v_dislocation_bias) *
             1 / std::pow((gp_float)vn, material.v_dislocation_bias_param / 2.);
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Interstitial binding energy.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::i_binding_energy(size_t in) const {
  return material.i_formation +
         (material.i_binding - material.i_formation) / (std::pow(2., .8) - 1) *
             (std::pow((gp_float)in, .8) - std::pow((gp_float)in - 1., .8));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Vacancy binding energy.
*/
__CUDADECL__ gp_float ClusterDynamicsImpl::v_binding_energy(size_t vn) const {
  return material.v_formation +
         (material.v_binding - material.v_formation) / (std::pow(2., .8) - 1) *
             (std::pow((gp_float)vn, .8) - std::pow((gp_float)vn - 1., .8));
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017),
 * pg 193, 4.59
 */
gp_float ClusterDynamicsImpl::i_diffusion() const {
  return material.i_diffusion_0 *
         std::exp(-material.i_migration /
                  (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017),
 * pg 193, 4.59
 */
gp_float ClusterDynamicsImpl::v_diffusion() const {
  return material.v_diffusion_0 *
         std::exp(-material.v_migration /
                  (BOLTZMANN_EV_KELVIN * reactor.temperature));
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.10
 */
gp_float ClusterDynamicsImpl::mean_dislocation_cell_radius() const {
  auto self = this->self;

  gp_float r_0_factor = thrust::transform_reduce(
      indices.begin(), indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->cluster_radius(idx) * self->interstitials[idx];
      },
      0.0, thrust::plus<gp_float>());

  return 1 /
         std::sqrt((2. * M_PI * M_PI / material.atomic_volume) * r_0_factor +
                   M_PI * dislocation_density);
}

// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.12
 */
__CUDADECL__ gp_float
ClusterDynamicsImpl::dislocation_promotion_probability(size_t n) const {
  return 0;

  gp_float dr = cluster_radius(n + 1) - cluster_radius(n);

  return (2. * cluster_radius(n) * dr + std::pow(dr, 2.)) /
         (std::pow(M_PI * mean_dislocation_radius_val / 2., 2) -
          std::pow(cluster_radius(n), 2.));
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 8
 */
gp_float ClusterDynamicsImpl::dislocation_density_derivative() const {
  return 0;

  // auto self = this->self;
  // gp_float gain = thrust::transform_reduce(
  //     indices.begin(), indices.end(),
  //     [self] __CUDADECL__(const int &idx) {
  //       return self->cluster_radius(idx) *
  //              self->dislocation_promotion_probability(idx) *
  //              self->ii_absorption(idx) * self->interstitials[idx];
  //     },
  //     0.0, thrust::plus<gp_float>());

  // gain *= 2. * M_PI / material.atomic_volume;

  // return gain - reactor.dislocation_density_evolution *
  //                   std::pow(material.burgers_vector, 2.) *
  //                   std::pow(dislocation_density, 3. / 2.);
}

// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017),
 * pg. 346, 7.63
 */
__CUDADECL__ gp_float ClusterDynamicsImpl::cluster_radius(size_t n) const {
  return std::sqrt(std::sqrt(3.) * std::pow(material.lattice_param, 2.) *
                   (gp_float)n / (4. * M_PI));
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  SIMULATION CONTROL FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

void ClusterDynamicsImpl::step_init() {
  i_diffusion_val = i_diffusion();
  v_diffusion_val = v_diffusion();
  mean_dislocation_radius_val = mean_dislocation_cell_radius();
  ii_sum_absorption_val = ii_sum_absorption(max_cluster_size - 1);
  iv_sum_absorption_val = iv_sum_absorption(max_cluster_size - 1);
  vi_sum_absorption_val = vi_sum_absorption(max_cluster_size - 1);
  vv_sum_absorption_val = vv_sum_absorption(max_cluster_size - 1);

  cudaMemcpy(thrust::raw_pointer_cast(self), this, sizeof(ClusterDynamicsImpl),
             cudaMemcpyHostToDevice);
}

ClusterDynamicsImpl::~ClusterDynamicsImpl() { 
  N_VDestroy_Serial(state);
  SUNMatDestroy(jacobian_matrix);
  SUNLinSolFree(linear_solver);
  CVodeFree(&cvodes_memory_block);
  SUNContext_Free(&sun_context);
  thrust::device_free(self); 
}

gp_float ClusterDynamicsImpl::ii_sum_absorption(size_t) const {
  auto self = this->self;
  return thrust::transform_reduce(
      indices.begin(), indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->ii_absorption(idx) * self->interstitials[idx];
      },
      0.0, thrust::plus<gp_float>());
}

gp_float ClusterDynamicsImpl::iv_sum_absorption(size_t) const {
  auto self = this->self;
  return thrust::transform_reduce(
      indices.begin(), indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->iv_absorption(idx) * self->interstitials[idx];
      },
      0.0, thrust::plus<gp_float>());
}

gp_float ClusterDynamicsImpl::vv_sum_absorption(size_t) const {
  auto self = this->self;
  return thrust::transform_reduce(
      indices.begin(), indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->vv_absorption(idx) * self->vacancies[idx];
      },
      0.0, thrust::plus<gp_float>());
}

gp_float ClusterDynamicsImpl::vi_sum_absorption(size_t) const {
  auto self = this->self;
  return thrust::transform_reduce(
      indices.begin(), indices.end(),
      [self] __CUDADECL__(const int &idx) {
        return self->vi_absorption(idx) * self->vacancies[idx];
      },
      0.0, thrust::plus<gp_float>());
}

int ClusterDynamicsImpl::system([[maybe_unused]] double t, N_Vector state_vector, N_Vector derivatives_vector, void* user_data)
{
  ClusterDynamicsImpl* cd = static_cast<ClusterDynamicsImpl*>(user_data);

  gp_float* i_state = N_VGetArrayPointer(state_vector);
  gp_float* v_state = i_state + cd->max_cluster_size + 2;
  cd->dislocation_density = *(v_state + cd->max_cluster_size + 2);

  cd->step_init();
  
  thrust::copy(i_state, i_state + cd->max_cluster_size, cd->interstitials.begin());
  thrust::copy(v_state, v_state + cd->max_cluster_size, cd->vacancies.begin());

  cd->host_interstitials = cd->interstitials;
  cd->host_vacancies = cd->vacancies;

  gp_float* i_derivatives = N_VGetArrayPointer(derivatives_vector);
  gp_float* v_derivatives = i_derivatives + cd->max_cluster_size + 2;
  gp_float* dislocation_derivative = v_derivatives + cd->max_cluster_size + 2;

  N_VConst(0.0, derivatives_vector);

  auto self = cd->self;
  thrust::transform(cd->indices.begin() + 1, cd->indices.end(),
                    cd->device_i_derivatives.begin() + 2,
                    [self] __CUDADECL__(const int &idx) {
                      return self->i_concentration_derivative(idx);
                    });

  thrust::transform(cd->indices.begin() + 1, cd->indices.end(),
                    cd->device_v_derivatives.begin() + 2,
                    [self] __CUDADECL__(const int &idx) {
                      return self->v_concentration_derivative(idx);
                    });
  
  thrust::copy(cd->device_i_derivatives.begin(), cd->device_i_derivatives.end(), i_derivatives);
  thrust::copy(cd->device_v_derivatives.begin(), cd->device_v_derivatives.end(), v_derivatives);

  i_derivatives[1] = cd->i1_concentration_derivative();
  v_derivatives[1] = cd->v1_concentration_derivative();
  *dislocation_derivative = cd->dislocation_density_derivative();

  return 0;
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  PUBLIC INTERFACE FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

// TODO - clean up the uses of random +1/+2/-1/etc throughout the code
ClusterDynamicsImpl::ClusterDynamicsImpl(ClusterDynamicsConfig &config)
    : time(0.0),
      interstitials(config.max_cluster_size + 2, 0.0),
      device_i_derivatives(config.max_cluster_size + 2, 0.0),
      vacancies(config.max_cluster_size + 2, 0.0),
      device_v_derivatives(config.max_cluster_size + 2, 0.0),
      material(*config.material.impl()),
      reactor(*config.reactor.impl()),
      indices(config.max_cluster_size, 0),
      host_interstitials(config.max_cluster_size + 1, 0.0),
      host_vacancies(config.max_cluster_size + 1, 0.0) {

  max_cluster_size = config.max_cluster_size;
  data_validation_on = config.data_validation_on;
  relative_tolerance = config.relative_tolerance;
  absolute_tolerance = config.absolute_tolerance;
  max_num_integration_steps = config.max_num_integration_steps;
  min_integration_step = config.min_integration_step;
  max_integration_step = config.max_integration_step;
  
  dislocation_density = material.dislocation_density_0;
  
  state_size = 2 * (max_cluster_size + 2) + 1;

  /* Create the SUNDIALS context */
  int sunerr = SUNContext_Create(SUN_COMM_NULL, &sun_context);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  /* Create the initial state */
  /// \todo Check errors
  state = N_VNew_Serial(state_size, sun_context);

  /* Initialize State Values */
  N_VConst(0.1, state);
  N_VGetArrayPointer(state)[state_size - 2] = material.dislocation_density_0;

  /* Call CVodeCreate to create the solver memory and specify the
    * Backward Differentiation Formula */
  cvodes_memory_block = CVodeCreate(CV_BDF, sun_context);

  /* Call CVodeInit to initialize the integrator memory and specify the
    * user's right hand side function in y'=f(t,y), the initial time T0, and
    * the initial dependent variable vector y. */
  sunerr = CVodeInit(cvodes_memory_block, system, 0, state);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  /* Call CVodeSVtolerances to specify the scalar relative tolerance
    * and scalar absolute tolerances */
  sunerr = CVodeSStolerances(cvodes_memory_block, relative_tolerance, absolute_tolerance);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  /* Create dense jacobian matrix */
  jacobian_matrix = SUNDenseMatrix(state_size, state_size, sun_context);

  /* Create linear solver object for use by CVode */
  linear_solver = SUNLinSol_Dense(state, jacobian_matrix, sun_context);

  sunerr = CVodeSetUserData(cvodes_memory_block, static_cast<void*>(this));
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  sunerr = CVodeSetMaxNumSteps(cvodes_memory_block, max_num_integration_steps);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  sunerr = CVodeSetMinStep(cvodes_memory_block, min_integration_step);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  sunerr = CVodeSetMaxStep(cvodes_memory_block, max_integration_step);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  // TODO - set to time delta?
  sunerr = CVodeSetInitStep(cvodes_memory_block, 1e-10);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  /* Attach the matrix and linear solver */
  sunerr = CVodeSetLinearSolver(cvodes_memory_block, linear_solver, jacobian_matrix);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  sunerr = CVodeSetInterpolateStopTime(cvodes_memory_block, SUNTRUE);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  ClusterDynamicsImpl *raw_self;
  cudaMalloc(&raw_self, sizeof(ClusterDynamicsImpl));
  cudaMemcpy(raw_self, this, sizeof(ClusterDynamicsImpl),
             cudaMemcpyHostToDevice);
  self = thrust::device_ptr<ClusterDynamicsImpl>(raw_self);
  thrust::sequence(indices.begin(), indices.end(), 1);
}

ClusterDynamicsState ClusterDynamicsImpl::run(gp_float total_time) {
  gp_float out_time;
  const int sunerr = CVode(cvodes_memory_block, time + total_time, state, &out_time, CV_NORMAL);
  if (sunerr) throw ClusterDynamicsException(SUNGetErrMsg(sunerr), ClusterDynamicsState());

  time = out_time;

  gp_float* i_state = N_VGetArrayPointer(state);
  gp_float* v_state = i_state + max_cluster_size + 2;
  dislocation_density = *(v_state + max_cluster_size + 2);
  thrust::copy(i_state, i_state + max_cluster_size, host_interstitials.begin());
  thrust::copy(v_state, v_state + max_cluster_size, host_vacancies.begin());

  return ClusterDynamicsState{
      .time = time,
      .dpa = time * reactor.flux,
      .interstitials = std::vector<gp_float>(host_interstitials.begin(),
                                             host_interstitials.end() - 1),
      .vacancies = std::vector<gp_float>(host_vacancies.begin(),
                                         host_vacancies.end() - 1),
      .dislocation_density = dislocation_density};
}

MaterialImpl ClusterDynamicsImpl::get_material() const { return material; }

void ClusterDynamicsImpl::set_material(const MaterialImpl &material) {
  this->material = MaterialImpl(material);
}

NuclearReactorImpl ClusterDynamicsImpl::get_reactor() const { return reactor; }

void ClusterDynamicsImpl::set_reactor(const NuclearReactorImpl &reactor) {
  this->reactor = NuclearReactorImpl(reactor);
}
