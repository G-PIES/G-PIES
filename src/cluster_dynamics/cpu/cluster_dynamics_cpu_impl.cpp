#include "cluster_dynamics_cpu_impl.hpp"

#include <stdio.h>

#include <cstring>

#include "cluster_dynamics/cluster_dynamics.hpp"
#include "cluster_dynamics/cluster_dynamics_config.hpp"

/** @brief Returns the rate of change of the concentration of interstitial
 * clusters of size (n) . \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 2a
 *
 * \f$
 * \dwn{\frac{dC_i(n)}{dt} =}
 * \ann{1}{G_i(n)}\dwn{+}
 * \ann{2}{a_{i,n+1}C_i(n+1)}\dwn{-}
 * \ann{3}{b_{i,n}C_i(n)}\dwn{+}
 * \ann{4}{c_{i,n-1}C_i(n-1)}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::i_concentration_derivative(size_t n) const {
  return
      // (1)
      i_defect_production(n) / material.atomic_volume
      // (2)
      + i_demotion_rate(n + 1) * interstitials[n + 1]
      // //    (3)
      - i_combined_promotion_demotion_rate(n) * interstitials[n]
      //    (4)
      + i_promotion_rate(n - 1) * interstitials[n - 1];
}

/** @brief Returns the rate of change of the concentration of vacancy clusters
 * of size (n) . \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 2a
 *
 * \f$
 * \dwn{\frac{dC_v(n)}{dt} =}
 * \ann{1}{G_v(n)}\dwn{+}
 * \ann{2}{a_{v,n+1}C_v(n+1)}\dwn{-}
 * \ann{3}{b_{v,n}C_v(n)}\dwn{+}
 * \ann{4}{c_{v,n-1}C_v(n-1)}
 * \f$
 *
 * <b>Notes</b>
 *
 * The Pokor paper's equation 2a only defines the derivative of interstitial
 * cluster concentrations. However, the paper implies that it also works for
 * vacancy clusters in the line immediately preceding the definition of 2a.
 */
gp_float ClusterDynamicsCpuImpl::v_concentration_derivative(size_t n) const {
  return
      // (1)
      v_defect_production(n) / material.atomic_volume
      // (2)
      + v_demotion_rate(n + 1) * vacancies[n + 1]
      // (3)
      - v_combined_promotion_demotion_rate(n) * vacancies[n]
      // (4)
      + v_promotion_rate(n - 1) * vacancies[n - 1];
}

/** @brief Returns the rate of change in the concentration of size 1
 * interstitial clusters, . \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3a
 *
 *  \f$
 *    \dwn{\frac{dC_i(1)}{dt} =}
 *    \ann{1}{G_i(1)}\dwn{-}
 *    \ann{2}{R_{iv}C_i(1)C_v(1)}\dwn{-}
 *    \ann{3}{\frac{C_i(1)}{\tau^a_{d,i}}}\dwn{-}
 *    \ann{4}{\frac{C_i(1)}{\tau^a_{gb,i}}}\dwn{+}
 *    \ann{5}{\frac{C_i(1)}{\tau^a_i}}\dwn{+}
 *    \ann{6}{\frac{1}{\tau^e_i}}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::i1_concentration_derivative() const {
  return
      // (1)
      i_defect_production(1) / material.atomic_volume
      // // (2)
      - annihilation_rate() * interstitials[1] * vacancies[1]
      // (3)
      - interstitials[1] * i_dislocation_annihilation_rate()
      // // (4)
      - interstitials[1] * i_grain_boundary_annihilation_rate()
      // // (5)
      - interstitials[1] * i_absorption_rate()
      // // (6)
      + i_emission_rate();
}

/** @brief Returns the rate of change in the concentration of size 1 vacancy
 * clusters, . \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3a
 *
 *  \f$
 *    \dwn{\frac{dC_v(1)}{dt} =}
 *    \ann{1}{G_v(1)}\dwn{-}
 *    \ann{2}{R_{iv}C_i(1)C_v(1)}\dwn{-}
 *    \ann{3}{\frac{C_v(1)}{\tau^a_{d,v}}}\dwn{-}
 *    \ann{4}{\frac{C_v(1)}{\tau^a_{gb,v}}}\dwn{+}
 *    \ann{5}{\frac{C_v(1)}{\tau^a_v}}\dwn{+}
 *    \ann{6}{\frac{1}{\tau^e_v}}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::v1_concentration_derivative() const {
  return
      // (1)
      v_defect_production(1) / material.atomic_volume
      // (2)
      - annihilation_rate() * interstitials[1] * vacancies[1]
      // (3)
      - vacancies[1] * v_dislocation_annihilation_rate()
      // (4)
      - vacancies[1] * v_grain_boundary_annihilation_rate()
      // (5)
      - vacancies[1] * v_absorption_rate()
      // (6)
      + v_emission_rate();
}

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 8
 */
/** @brief Returns the rate of change of the density of the dislocation network.
 * \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 8
 *  N. Sakaguchi / Acta Materialia 1131 (2001), Equation 3.14
 *
 *  \f$
 *    \dwn{\frac{d\rho}{dt}=}
 *    \ann{1}{ \frac{2 \pi}{\Omega} \sum_{n>0} r_i(n) C_i(n) \beta_{i,i}(n)
 * P_{unf}(n) }\dwn{-} \ann{2}{ Kb^2\rho^{3/2} \vphantom{\sum_{n>0}} } \f$
 *
 * <b>Notes</b>
 *
 * The gain term comes from the Sakaguchi paper's model of dislocation network
 * evolution modified to use notation more similar to the Pokor paper. Every
 * time an interstitial cluster grows by absorbing a size 1 interstitial, it has
 * a \f$P_{unf}(n)\f$ probability of becoming part of the dislocation network
 * instead.
 */
gp_float ClusterDynamicsCpuImpl::dislocation_density_derivative() const {
  gp_float gain = 0.0;
  for (size_t n = 1; n < max_cluster_size; ++n) {
    gain += cluster_radius(n) * ii_absorption(n) * interstitials[n] *
            i_dislocation_loop_unfault_probability(n);
  }

  gain *= 2. * M_PI / material.atomic_volume;

  return
      // (1)
      gain
      // (2)
      - reactor.dislocation_density_evolution *
            std::pow(material.burgers_vector, 2.) *
            std::pow(*dislocation_density, 3. / 2.);
}

/** @brief Returns the rate of production of interstital defects from the
 * irradiation cascade for size (n) clusters. \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
 *
 * \f$
 * \dwn{G_i(1)=}\ann{1}{\eta}
 * \ann{2}{G_{dpa}}\dwn{(1-}\ann{3}{f_{i2}}\dwn{-}\ann{4}{f_{i3})}\ann{5}{f_{i4})}\\
 * G_i(2)=\eta G_{dpa}f_{i2}\\
 * G_i(3)=\eta G_{dpa}f_{i3}\\
 * G_i(4)=\eta G_{dpa}f_{i4}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::i_defect_production(size_t n) const {
  switch (n) {
    //             (1)                     (2)
    case 1:
      return reactor.recombination * reactor.flux *
             //    (3)            (4)             (5)
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

/** @brief Returns the rate of production of vacancies defects from the
 * irradiation cascade for size (n) clusters. \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
 *
 * \f$
 * \dwn{G_v(1)=}\ann{1}{\eta}
 * \ann{2}{G_{dpa}}\dwn{(1-}\ann{3}{f_{v2}}\dwn{-}\ann{4}{f_{v3}}\dwn{-}\ann{5}{f_{v4})}\\
 * G_v(2)=\eta G_{dpa}f_{v2}\\
 * G_v(3)=\eta G_{dpa}f_{v3}\\
 * G_v(4)=\eta G_{dpa}f_{v4}
 * \f$
 *
 * <b>Notes</b>
 *
 * These equations are only implicitly provided in the Pokor paper. The
 * equations for interstitial generation are given followed by the line 'A
 * similar expression is written for \f$G_v(n)\f$.'
 */
gp_float ClusterDynamicsCpuImpl::v_defect_production(size_t n) const {
  switch (n) {
    //             (1)                     (2)
    case 1:
      return reactor.recombination * reactor.flux *
             //    (3)            (4)             (5)
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

/** @brief Returns the combined rate of emission of an interstitial and
 * absorption of a vacancy by an interstitial loop of size (n).
 * \todo Document units, both events
 * leading to an interstitial loop of size n.
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 2b
 *
 * \f$
 *  \dwn{a_{i,n-1} = }
 *  \ann{1}{\beta_{i,v}(n+1)}\ann{2}{C_v(1)}\dwn{+}
 *  \ann{3}{\alpha_{i,i}(n+1)}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::i_demotion_rate(size_t n) const {
  return
      // (1)
      iv_absorption(n) *
          // (2)
          vacancies[1] +
      // (3)
      ii_emission(n);
}

/** @brief Returns the combined rate of emission of an interstitial and
 * absorption of a vacancy by an interstitial loop of size (n),
 * both events leading to an interstitial loop of size n.
 * \todo Document units
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 2b
 *
 * \f$
 *  \dwn{a_{v,n-1} = }
 *  \ann{1}{\beta_{v,i}(n+1)}\ann{2}{C_i(1)}\dwn{+}
 *  \ann{3}{\alpha_{v,v}(n+1)}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::v_demotion_rate(size_t n) const {
  return
      // (1)
      vi_absorption(n) *
          // (2)
          interstitials[1] +
      // (3)
      vv_emission(n);
}

/** @brief Returns the rate that an interstitial cluster of size n can evolve
 * toward a cluster of size n + 1 absorbing an interstitial, or toward a cluster
 * of size n - 1 absorbing a vacancy or emitting an interstitial, . \todo
 * Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 2c
 *
 *  \f$
 *    \dwn{b_{i,n} =}
 *    \ann{1}{\beta_{i,v}(n)C_v(1)}\dwn{+}
 *    \ann{2}{\beta_{i,i}(n)C_i(1)}\dwn{+}
 *    \ann{4}{\alpha_{i,i}(n)}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::i_combined_promotion_demotion_rate(
    size_t n) const {
  return
      // (1)
      iv_absorption(n) * vacancies[1]
      // (2)
      + ii_absorption(n) * interstitials[1]
      // (3)
      + ii_emission(n);
}

/** @brief Returns the rate that a vacancy cluster of size n can evolve toward a
 * cluster of size n + 1 absorbing a vacancy, or toward a cluster of size n - 1
 * absorbing an interstitial or emitting an vacancy. \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 2c
 *  N. Sakaguchi / Acta Materialia 1131 (2001), Equation 3.14
 *
 *  \f$
 *    \dwn{b_{v,n} =}
 *    \ann{1}{\beta_{v,i}(n)C_i(1)}\dwn{+}
 *    \ann{2}{\beta_{v,v}(n)C_v(1)}
 *    \ann{3}{\alpha_{v,v}(n)}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::v_combined_promotion_demotion_rate(
    size_t n) const {
  return
      // (1)
      vi_absorption(n) * interstitials[1]
      // (2)
      + vv_absorption(n) * vacancies[1]
      // (3)
      + vv_emission(n);
}

/** @brief Returns the rate that an interstitial cluster of size n - 1 can
 * evolve into a cluster of size n by absorbing an interstitial. \todo Document
 * units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 2d
 * N. Sakaguchi / Acta Materialia 1131 (2001), Equation 3.14
 *
 * \f$
 *   \dwn{c_{i,n-1} =}
 *   \ann{1}{\beta_{i,i}(n-1)}
 *   \ann{2}{C_i(1)}
 *   \ann{3}{(1-P_{unf}(n))}
 * \f$
 *
 *  <b>Notes</b>
 *
 *  This equation uses the cluster evolution term from the Pokor paper
 *  modified to accomodate the dislocation network evolution model of the
 *  Sakaguchi paper. \f$K_{1(j)}\f$ from the Sakaguchi paper corresponds
 *  to \f$\beta_{i,i}(n)C_i(1)\f$ in the Pokor paper, both representing
 *  the rate of promotion to the next cluster size. Dr. Chen suggested
 *  using \f$P_{unf}(n)\f$, in a similar way to the Sakaguchi paper to
 *  determine what percentage of promoted clusters become part of the
 *  dislocation network.
 */
gp_float ClusterDynamicsCpuImpl::i_promotion_rate(size_t n) const {
  return
      // (1)
      ii_absorption(n)
      // (2)
      * interstitials[1]
      // (3)
      * (1 - i_dislocation_loop_unfault_probability(n + 1));
}

/** @brief Returns the rate that a vacancy cluster of size n - 1 can evolve into
 * a cluster of size n by absorbing a vacancy,. \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 2d
 *
 * \f$
 *   \dwn{c_{i,n-1} =}
 *   \ann{1}{\beta_{v,v}(n-1)}\dwn{*}
 *   \ann{2}{C_v(1)}
 * \f$
 *
 */
gp_float ClusterDynamicsCpuImpl::v_promotion_rate(size_t n) const {
  return
      // (1)
      vv_absorption(n) *
      // (2)
      vacancies[1];
}

/** @brief Returns one over the characteristic time for emitting an interstitial
 * by the population of interstital or vacancy clusters,. \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3b
 *
 *  \f$
 *    \dwn{\frac{1}{\tau^e_i(n)} = }
 *    \ann{1}{\sum_{n>2} \alpha_{i,i}(n) C_i(n)}\dwn{+}
 *    \ann{2}{2 \alpha_{i,i}(2) C_i(2)}\dwn{+}
 *    \ann{3}{\beta_{i,v}(2) C_v(1) C_i(2)}
 *  \f$
 *
 *  <b>Notes</b>
 *
 *  (1) Combines the rates of emission of single interstitials by interstitial
 * clusters of size > 2.
 *
 *  (2) Represents the rate of emission of single interstitials by interstitial
 * clusters of size = 2, which will generate 2 interstitials.
 *
 *  (3) Represents the rate that interstitials are produced by interstitial
 * clusters of size 2 absorbing a vacancy.
 */
gp_float ClusterDynamicsCpuImpl::i_emission_rate() const {
  gp_float rate = 0.;
  for (size_t in = 3; in < max_cluster_size - 1; ++in) {
    rate +=
        // (1)
        ii_emission(in) * interstitials[in];
  }

  rate +=
      // (2)
      2. * ii_emission(2) * interstitials[2]
      // (3)
      + iv_absorption(2) * vacancies[1] * interstitials[2];

  return rate;
}

/** @brief Returns one over the characteristic time for emitting an vacancy by
 *  the population of interstital or vacancy clusters,. \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3b
 *
 *  \f$
 *    \dwn{\frac{1}{\tau^e_v(n)} = }
 *    \ann{1}{\sum_{n>2} \alpha_{v,v}(n) C_v(n)}\dwn{+}
 *    \ann{2}{4 \alpha_{v,v}(2) C_v(2)}\dwn{+}
 *    \ann{3}{\beta_{v,i}(2) C_i(1) C_v(2)}
 *  \f$
 *
 *  <b>Notes</b>
 *
 *  (1) Combines the rates of emission of single vacancies by vacancy clusters
 * of size > 2.
 *
 *  (2) Represents the rate of emission of single vacancies by vacancy clusters
 * of size = 2, which results in 2 vacancies.
 *
 *  (3) Represents the rate that vacancies are produced by vacancy clusters of
 * size 2 absorbing a vacancy.
 */
gp_float ClusterDynamicsCpuImpl::v_emission_rate() const {
  gp_float rate = 0.;
  for (size_t vn = 3; vn < max_cluster_size - 1; ++vn) {
    rate +=
        // (1)
        vv_emission(vn) * vacancies[vn];
  }

  rate +=
      // (2)
      2. * vv_emission(2) * vacancies[2]
      // (3)
      + vi_absorption(2) * interstitials[1] * vacancies[2];

  return rate;
}

/** @brief Returns 1 over the characteristic time for absorbing an interstitial
 * by the population of interstital or vacancy clusters,. \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3c
 *
 *  \f$
 *    \dwn{\frac{1}{\tau^a_i(n)} =}
 *    \ann{1}{\sum_{n>0} \beta_{i,i}(n) C_i(n)}\dwn{+}
 *    \ann{2}{\sum_{n>1} \beta_{v,i}(n) C_v(n)}
 *  \f$
 *
 */
gp_float ClusterDynamicsCpuImpl::i_absorption_rate() const {
  gp_float rate = 0.0;
  rate += ii_absorption(1) * interstitials[1];
  for (size_t in = 2; in < max_cluster_size - 1; ++in) {
    rate +=
        // (1)
        ii_absorption(in) * interstitials[in]
        // (2)
        + vi_absorption(in) * vacancies[in];
  }

  return rate;
}

/** @brief Returns 1 over the characteristic time for absorbing a vacancy by
 *  the population of interstital or vacancy clusters,. \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3c
 *
 *  \f$
 *    \dwn{\frac{1}{\tau^a_v(n)} =}
 *    \ann{1}{\sum_{n>0} \beta_{v,v}(n) C_v(n)}\dwn{+}
 *    \ann{2}{\sum_{n>1} \beta_{i,v}(n) C_i(n)}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::v_absorption_rate() const {
  gp_float rate = vv_absorption(1) * vacancies[1];
  for (size_t vn = 2; vn < max_cluster_size - 1; ++vn) {
    rate +=
        // (1)
        vv_absorption(vn) * vacancies[vn]
        // (2)
        + iv_absorption(vn) * interstitials[vn];
  }

  return rate;
}

// --------------------------------------------------------------------------------------------

/** @brief Returns the annihilation rate of single vacancies and insterstitals.
 * \todo Document units <hr> C. Pokor / Journal of Nuclear Materials 326 (2004),
 * Equation 3d
 *
 *  \f$
 *  \dwn{R_{iv} =}
 *  \ann{1}{4\pi}
 *  \ann{2}{(D_i + D_v)}
 *  \ann{3}{r_{iv}}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::annihilation_rate() const {
  return
      // (1)
      4. * M_PI *
      // (2)
      (i_diffusion_val + v_diffusion_val) *
      // (3)
      material.recombination_radius;
}
// --------------------------------------------------------------------------------------------

/** @brief Returns the 1 over the characteristic time for annihilation of
 * interstitials on dislocations. \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3e
 *
 * \f$
 * \Large \ann{}{\frac{1}{\tau^a_{d,i}} =}
 * \ann{1}{\rho}
 * \ann{2}{D_i}
 * \ann{3}{Z_i}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::i_dislocation_annihilation_rate() const {
  return
      // (1)
      *dislocation_density *
      // (2)
      i_diffusion_val *
      // (3)
      material.i_dislocation_bias;
}

/** @brief Returns the 1 over the characteristic time for annihilation of
 * vacancies on dislocations. \todo Document units
 *
 * <hr>
 *
 * This equation is based on an altered version of Equation 3e in the Pokor
 * paper which uses the vacancy version of each variable. See the line in the
 * Pokor paper following this equation's definition for some justification.
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3e
 *
 * \f$
 * \dwn{\frac{1}{\tau^a_{d,v}} =}
 * \ann{1}{\rho \vphantom{D_v}}
 * \ann{2}{D_v}
 * \ann{3}{Z_v}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::v_dislocation_annihilation_rate() const {
  return
      // (1)
      *dislocation_density *
      // (2)
      v_diffusion_val *
      // (3)
      material.v_dislocation_bias;
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/** @brief Returns 1 over the characteristic time for annihilation of
 * interstitials on grain boundaries . \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3f
 *
 *  \f$
 *  \LARGE\frac{1}{\tau^a_{gb,i}} =
 *  \frac
 *    {\ann{1}{6D_i}\sqrt{
 *      \ann{2}{\rho Z_i} \ann{}{+}
 *      \ann{3}{\sum_n \beta_{i,i}(N) C_i(n)} \ann{}{+}
 *      \ann{4}{\sum_n \beta_{v,i}(N) C_v(n)}}}
 *    {\ann{5}{d}}
 *  \f$
 *
 */
gp_float ClusterDynamicsCpuImpl::i_grain_boundary_annihilation_rate() const {
  return
      // (1)
      6. * i_diffusion_val *
      sqrt(
          // (2)
          *dislocation_density * material.i_dislocation_bias
          // (3)
          + ii_sum_absorption_val
          // (4)
          + vi_sum_absorption_val) /
      // (5)
      material.grain_size;
}

// --------------------------------------------------------------------------------------------
/** @brief Returns 1 over the characteristic time for annihilation of vacancies
 * on grain boundaries . \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3f
 *
 *  \f$
 *  \LARGE\frac{1}{\tau^a_{gb,v}} =
 *  \frac
 *    {\ann{1}{6D_v}\sqrt{
 *      \ann{2}{\rho Z_v} \ann{}{+}
 *      \ann{3}{\sum_n \beta_{v,v}(N) C_v(n)} \ann{}{+}
 *      \ann{4}{\sum_n \beta_{i,v}(N) C_i(n)}}}
 *    {\ann{5}{d}}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::v_grain_boundary_annihilation_rate() const {
  return
      // (1)
      6. * v_diffusion_val *
      sqrt(
          // (2)
          *dislocation_density * material.v_dislocation_bias
          // (3)
          + vv_sum_absorption_val
          // (4)
          + iv_sum_absorption_val) /
      // (5)
      material.grain_size;
}

/** @brief Returns the rate of emission of an interstitial by an interstital
 * cluster of size (n) . \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 4a
 *
 *  \f$
 *    \dwn{\alpha_{i,i}(n) = }
 *    \ann{1}{2 \pi r_i(n)\vphantom{\frac{Z_t}{Z_t}}}
 *    \ann{2}{Z_{ic}\vphantom{\frac{Z_t}{Z_t}}}
 *    \ann{3}{\frac{D_i}{V_{at}}}
 *    \ann{4}{exp(-\frac{E_bi(n)}{kT})}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::ii_emission(size_t n) const {
  return
      // (1)
      2. * M_PI * cluster_radius(n) *
      // (2)
      i_bias_factor(n) *
      // (3)
      i_diffusion_val / material.atomic_volume *
      // (4)
      exp(-i_binding_energy(n) / (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/** @brief Returns the rate of absorption of an interstitial by an interstital
 *cluster of size (n), . \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 4b
 *
 * \f$
 *     \dwn{\beta_{i,i} =}
 *     \ann{1}{2 \pi r_i(n)}
 *     \ann{2}{Z_ic}
 *     \ann{3}{D_i}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::ii_absorption(size_t n) const {
  return
      // (1)
      2. * M_PI * cluster_radius(n) *
      // (2)
      i_bias_factor(n) *
      // (3)
      i_diffusion_val;
}

/** @brief Returns the rate of absorption of a vacancy by an interstitial
 * cluster of size (n). \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 4c
 *
 * \f$
 *     \dwn{\beta_{i,v} =}
 *     \ann{1}{2 \pi r_i(n)}
 *     \ann{2}{Z_vc}
 *     \ann{3}{D_v}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::iv_absorption(size_t n) const {
  return
      // (1)
      2. * M_PI * cluster_radius(n) *
      // (2)
      v_bias_factor(n) *
      // (3)
      v_diffusion_val;
}

/** @brief Returns the rate of emission of an interstitial by an interstital
 * cluster of size (n). \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 4a
 *
 *  \f$
 *    \dwn{\alpha_{v,v}(n) = }
 *    \ann{1}{2 \pi r_v(n)\vphantom{\frac{Z_t}{Z_t}}}
 *    \ann{2}{Z_{vc}\vphantom{\frac{Z_t}{Z_t}}}
 *    \ann{3}{\frac{D_v}{V_{at}}}
 *    \ann{4}{exp(-\frac{E_bv(n)}{kT})}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::vv_emission(size_t n) const {
  return
      // (1)
      2. * M_PI * cluster_radius(n) *
      // (2)
      v_bias_factor(n) *
      // (3)
      v_diffusion_val *
      // (4)
      exp(-v_binding_energy(n) / (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/** @brief Returns the rate of absorption of a vacancy by a vacancy cluster of
 * size (n). \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 4e
 *
 * \f$
 *     \dwn{\beta_{v,v} =}
 *     \ann{1}{2 \pi r_v(n)}
 *     \ann{2}{Z_vc}
 *     \ann{3}{D_v}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::vv_absorption(size_t n) const {
  return
      // (1)
      2. * M_PI * cluster_radius(n) *
      // (2)
      v_bias_factor(n) *
      // (3)
      v_diffusion_val;
}

/** @brief Returns the rate of absorption of an interstitial by a vacancy
 * cluster of size (n). \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 4f
 *
 * \f$
 *     \dwn{\beta_{v,i} =}
 *     \ann{1}{2 \pi r_v(n)}
 *     \ann{2}{Z_ic}
 *     \ann{3}{D_i}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::vi_absorption(size_t n) const {
  return 2. * M_PI * cluster_radius(n) * i_bias_factor(n) * i_diffusion_val;
}

/** @brief Returns the bias factor for an interstitial cluster of size (n).
 *  \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 5
 *
 * \f$
 *    \dwn{Z_{ic} =}
 *    \ann{1}{Z_i\vphantom{\sqrt{\frac{b}{b}}}}\dwn{+(}
 *    \ann{2}{\sqrt{\frac{b}{8 \pi a}}}
 *    \ann{3}{Z_{li}\vphantom{\sqrt{\frac{b}{b}}}}\dwn{-}
 *    \ann{4}{Z_i\vphantom{\sqrt{\frac{b}{b}}}}\dwn{)}
 *    \ann{5}{n^{-a_{li}/2}\vphantom{\sqrt{\frac{b}{b}}}}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::i_bias_factor(size_t n) const {
  return
      // (1)
      material.i_dislocation_bias +
      (
          // (2)
          std::sqrt(material.burgers_vector /
                    (8. * M_PI * material.lattice_param)) *
              // (3)
              material.i_loop_bias -
          // (4)
          material.i_dislocation_bias) *
          // (5)
          1. / std::pow((gp_float)n, material.i_dislocation_bias_param / 2.);
}

/** @brief Returns the bias factor of a vacancy cluster of size (n).
 *  \todo Document units
 *
 * <hr>
 *
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 5
 *
 * \f$
 *    \dwn{Z_{vc} =}
 *    \ann{1}{Z_i\vphantom{\sqrt{\frac{b}{b}}}}\dwn{+(}
 *    \ann{2}{\sqrt{\frac{b}{8 \pi a}}}
 *    \ann{3}{Z_{lv}\vphantom{\sqrt{\frac{b}{b}}}}\dwn{-}
 *    \ann{4}{Z_v\vphantom{\sqrt{\frac{b}{b}}}}\dwn{)}
 *    \ann{5}{n^{-a_{lv}/2}\vphantom{\sqrt{\frac{b}{b}}}}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::v_bias_factor(size_t n) const {
  return material.v_dislocation_bias +
         (std::sqrt(material.burgers_vector /
                    (8. * M_PI * material.lattice_param)) *
              material.v_loop_bias -
          material.v_dislocation_bias) *
             1. / std::pow((gp_float)n, material.v_dislocation_bias_param / 2.);
}

/** @brief Returnst the binding energy for an interstitial cluster of size (n).
 *  \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 6
 *
 *  \f$
 *    \dwn{E_{bi}(n) = }
 *    \ann{1}{E_{fi} \vphantom{\frac{E_f}{2^8.}} }\dwn{+}
 *    \ann{2}{\frac{E_{b2i}-E_{fi}}{2^{0.8}-1}}
 *    \ann{3}{(n^{0.8}-(n-1)^{0.8} \vphantom{\frac{E_f}{2^8.}})}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::i_binding_energy(size_t n) const {
  return
      // (1)
      material.i_formation
      // (2)
      + (material.i_binding - material.i_formation) / (std::pow(2., .8) - 1.) *
            // (3)
            (std::pow((gp_float)n, .8) - std::pow((gp_float)n - 1., .8));
}

/** @brief Returnst the binding energy for a vacancy cluster of size (n).
 *  \todo Document units
 *
 *  <hr>
 *
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 6
 *
 *  \f$
 *    \dwn{E_{bv}(n) = }
 *    \ann{1}{E_{fv} \vphantom{\frac{E_f}{2^8.}} }\dwn{+}
 *    \ann{2}{\frac{E_{b2v}-E_{fv}}{2^{0.8}-1}}
 *    \ann{3}{(n^{0.8}-(n-1)^{0.8} \vphantom{\frac{E_f}{2^8.}})}
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::v_binding_energy(size_t n) const {
  return
      // (1)
      material.v_formation
      // (2)
      + (material.v_binding - material.v_formation) / (std::pow(2., .8) - 1) *
            // (3)
            (std::pow((gp_float)n, .8) - std::pow((gp_float)n - 1., .8));
}

/** @brief Returns the diffusion coefficient for single interstitials.
 *  \todo Document units
 *
 *  <hr>
 *
 *  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017),
 * pg 193, Equation 4.59
 *
 *  \f$
 *    \dwn{D_i =}
 *    \ann{1}{D_{i0} \vphantom{\frac{-E_{mi}}{kT}} }
 *    \ann{2}{exp(\frac{-E_{mi}}{kT}) }
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::i_diffusion() const {
  //     (1)                      (2)
  return material.i_diffusion_0 *
         std::exp(-material.i_migration /
                  (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/** @brief Returns the diffusion coefficient for single vacancies.
 *  \todo Document units
 *
 *  <hr>
 *
 *  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017),
 * pg 193, Equation 4.59
 *
 *  \f$
 *    \dwn{D_v =}
 *    \ann{1}{D_{v0} \vphantom{\frac{-E_{mv}}{kT}} }
 *    \ann{2}{exp(\frac{-E_{mv}}{kT}) }
 *  \f$
 */
gp_float ClusterDynamicsCpuImpl::v_diffusion() const {
  //     (1)                      (2)
  return material.v_diffusion_0 *
         std::exp(-material.v_migration /
                  (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/** Returns the mean dislocation cell radius of the system.
 *  \todo Document units
 *
 * <hr>
 *
 * N. Sakaguchi / Acta Materialia 1131 (2001), Equation 3.10
 *
 * \f$
 *   \dwn{r_0 = (}
 *       \ann{1}{\frac{2 \pi ^ 2}{\Omega}}
 *       \ann{2}{\vphantom{\frac{2^2}{\Omega}} \sum_{n>0} r_i(n) C_i(n)}\dwn{+}
 *       \ann{3}{\vphantom{\frac{2^2}{\Omega}} \pi \rho}
 *    \dwn{)^{-1/2}}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::mean_dislocation_cell_radius() const {
  gp_float r_0_factor = 0.;
  for (size_t i = 1; i < max_cluster_size; ++i) {
    r_0_factor += cluster_radius(i) * interstitials[i];
  }

  // (1)                                           (2)          (3)
  return 1 /
         std::sqrt((2. * M_PI * M_PI / material.atomic_volume) * r_0_factor +
                   M_PI * (*dislocation_density));
}

// --------------------------------------------------------------------------------------------
/*  TODO: find a source for the Arrhenius equation
 */
/** @brief Returns the probability an interstitial dislocation loop will unfault
 * into a dislocation network using the arrhenius equation.
 */
gp_float ClusterDynamicsCpuImpl::i_dislocation_loop_unfault_probability(
    [[maybe_unused]] size_t n) const {
  gp_float energy_barrier = faulted_dislocation_loop_energy_barrier(n);
  gp_float arrhenius =
      exp(-energy_barrier / (BOLTZMANN_EV_KELVIN * reactor.temperature));

  return arrhenius;
}

/*  TODO: find a source for the energy barrier equation
 */
gp_float ClusterDynamicsCpuImpl::faulted_dislocation_loop_energy_barrier(
    [[maybe_unused]] size_t n) const {
  return material.i_binding + material.i_migration;
}

// --------------------------------------------------------------------------------------------
/*
 */
/** @brief Returns the cluster radius of a cluster of size (n) in cm.
 *
 * <hr>
 *
 * G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017),
 * pg. 346, Equation 7.63
 *
 * \f$
 * r_i = (\frac{\sqrt{3}a^2n}{4 \pi})^{-1/2}
 * \f$
 */
gp_float ClusterDynamicsCpuImpl::cluster_radius(size_t n) const {
  return std::sqrt(std::sqrt(3.) * std::pow(material.lattice_param, 2.) *
                   (gp_float)n / (4. * M_PI));
}

gp_float ClusterDynamicsCpuImpl::ii_sum_absorption(size_t nmax) const {
  gp_float emission = 0.;
  for (size_t n = 1; n < nmax; ++n) {
    emission += ii_absorption(n) * interstitials[n];
  }

  return emission;
}

gp_float ClusterDynamicsCpuImpl::iv_sum_absorption(size_t nmax) const {
  gp_float emission = 0.;
  for (size_t n = 1; n < nmax; ++n) {
    emission += iv_absorption(n) * interstitials[n];
  }

  return emission;
}

gp_float ClusterDynamicsCpuImpl::vv_sum_absorption(size_t nmax) const {
  gp_float emission = 0.;
  for (size_t n = 1; n < nmax; ++n) {
    emission += vv_absorption(n) * vacancies[n];
  }

  return emission;
}

gp_float ClusterDynamicsCpuImpl::vi_sum_absorption(size_t nmax) const {
  gp_float emission = 0.;
  for (size_t n = 1; n < nmax; ++n) {
    emission += vi_absorption(n) * vacancies[n];
  }

  return emission;
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  SIMULATION CONTROL FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

void ClusterDynamicsCpuImpl::step_init() {
  i_diffusion_val = i_diffusion();
  v_diffusion_val = v_diffusion();
  ii_sum_absorption_val = ii_sum_absorption(max_cluster_size - 1);
  iv_sum_absorption_val = iv_sum_absorption(max_cluster_size - 1);
  vi_sum_absorption_val = vi_sum_absorption(max_cluster_size - 1);
  vv_sum_absorption_val = vv_sum_absorption(max_cluster_size - 1);
  mean_dislocation_radius_val = mean_dislocation_cell_radius();
}

int ClusterDynamicsCpuImpl::system([[maybe_unused]] double t, N_Vector v_state,
                                   N_Vector v_state_derivatives,
                                   void* user_data) {
  ClusterDynamicsCpuImpl* cd = static_cast<ClusterDynamicsCpuImpl*>(user_data);
  cd->interstitials = N_VGetArrayPointer(v_state);
  cd->vacancies = cd->interstitials + cd->max_cluster_size + 2;
  cd->dislocation_density = cd->vacancies + cd->max_cluster_size + 2;

  cd->step_init();

  double* i_derivatives = N_VGetArrayPointer(v_state_derivatives);
  double* v_derivatives = i_derivatives + cd->max_cluster_size + 2;
  double* dislocation_derivative = v_derivatives + cd->max_cluster_size + 2;

  N_VConst(0.0, v_state_derivatives);

  i_derivatives[1] = cd->i1_concentration_derivative();
  for (size_t i = 2; i <= cd->max_cluster_size; ++i) {
    i_derivatives[i] = cd->i_concentration_derivative(i);
  }

  v_derivatives[1] = cd->v1_concentration_derivative();
  for (size_t i = 2; i <= cd->max_cluster_size; ++i) {
    v_derivatives[i] = cd->v_concentration_derivative(i);
  }
  *dislocation_derivative = cd->dislocation_density_derivative();

  return 0;
}

void ClusterDynamicsCpuImpl::validate(size_t n) const {
  if (!data_validation_on) return;

  if (std::isnan(interstitials[n]) || std::isnan(vacancies[n]) ||
      std::isinf(interstitials[n]) || std::isinf(vacancies[n]) ||
      interstitials[n] < 0. || vacancies[n] < 0.) {
    throw ClusterDynamicsException(
        "Simulation Validation Failed For Cluster Size " + std::to_string(n) +
            ".",
        ClusterDynamicsState{
            .time = time,
            .dpa = time * reactor.flux,
            .interstitials = std::vector<gp_float>(
                interstitials, interstitials + max_cluster_size),
            .vacancies =
                std::vector<gp_float>(vacancies, vacancies + max_cluster_size),
            .dislocation_density = (*dislocation_density)});
  }
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  PUBLIC INTERFACE FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

//!< \todo Clean up the uses of random +1/+2/-1/etc throughout the code
ClusterDynamicsCpuImpl::ClusterDynamicsCpuImpl(ClusterDynamicsConfig& config)
    : time(0.0),
      max_cluster_size(config.max_cluster_size),
      material(*config.material.impl()),
      reactor(*config.reactor.impl()) {
  max_cluster_size = config.max_cluster_size;
  data_validation_on = config.data_validation_on;
  relative_tolerance = config.relative_tolerance;
  absolute_tolerance = config.absolute_tolerance;
  max_num_integration_steps = config.max_num_integration_steps;
  min_integration_step = config.min_integration_step;
  max_integration_step = config.max_integration_step;

  state_size = 2 * (max_cluster_size + 2) + 1;

  /* Create the SUNDIALS context */
  int sunerr = SUNContext_Create(SUN_COMM_NULL, &sun_context);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  /* Create the initial state */
  /// \todo Check errors
  state = N_VNew_Serial(state_size, sun_context);

  /* Set State Aliases */
  interstitials = N_VGetArrayPointer(state);
  vacancies = interstitials + max_cluster_size + 2;
  dislocation_density = vacancies + max_cluster_size + 2;

  /* Initialize State Values */
  for (size_t i = 0; i < max_cluster_size; ++i) {
    interstitials[i] = config.init_interstitials[i];
    vacancies[i] = config.init_vacancies[i];
  }

  *dislocation_density = material.dislocation_density_0;
  interstitials[max_cluster_size + 1] = 0.0;
  vacancies[max_cluster_size + 1] = 0.0;

  /* Call CVodeCreate to create the solver memory and specify the
   * Backward Differentiation Formula */
  cvodes_memory_block = CVodeCreate(CV_BDF, sun_context);

  /* Call CVodeInit to initialize the integrator memory and specify the
   * user's right hand side function in y'=f(t,y), the initial time T0, and
   * the initial dependent variable vector y. */
  sunerr = CVodeInit(cvodes_memory_block, system, 0, state);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  /* Call CVodeSVtolerances to specify the scalar relative tolerance
   * and scalar absolute tolerances */
  sunerr = CVodeSStolerances(cvodes_memory_block, relative_tolerance,
                             absolute_tolerance);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  /* Create dense jacobian matrix */
  jacobian_matrix = SUNDenseMatrix(state_size, state_size, sun_context);

  /* Create dense SUNLinearSolver object for use by CVode */
  linear_solver = SUNLinSol_Dense(state, jacobian_matrix, sun_context);

  sunerr = CVodeSetUserData(cvodes_memory_block, static_cast<void*>(this));
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  sunerr = CVodeSetMaxNumSteps(cvodes_memory_block, max_num_integration_steps);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  sunerr = CVodeSetMinStep(cvodes_memory_block, min_integration_step);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  sunerr = CVodeSetMaxStep(cvodes_memory_block, max_integration_step);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  // TODO - set to time delta?
  sunerr = CVodeSetInitStep(cvodes_memory_block, 1e-5);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  /* Attach the matrix and linear solver */
  sunerr =
      CVodeSetLinearSolver(cvodes_memory_block, linear_solver, jacobian_matrix);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  // CVodeSetInterpolateStopTime(cvodes_memory_block, 1);
}

ClusterDynamicsCpuImpl::~ClusterDynamicsCpuImpl() {
  N_VDestroy_Serial(state);
  SUNMatDestroy(jacobian_matrix);
  SUNLinSolFree(linear_solver);
  CVodeFree(&cvodes_memory_block);
  SUNContext_Free(&sun_context);
}

ClusterDynamicsState ClusterDynamicsCpuImpl::run(gp_float total_time) {
  double out_time;
  const int sunerr = CVode(cvodes_memory_block, time + total_time, state,
                           &out_time, CV_NORMAL);
  if (sunerr)
    throw ClusterDynamicsException(SUNGetErrMsg(sunerr),
                                   ClusterDynamicsState());

  time = out_time;

  interstitials = N_VGetArrayPointer(state);
  vacancies = interstitials + max_cluster_size + 2;
  dislocation_density = vacancies + max_cluster_size + 2;

  return ClusterDynamicsState{
      .time = time,
      .dpa = time * reactor.flux,
      .interstitials =
          std::vector<double>(interstitials, interstitials + max_cluster_size),
      .vacancies = std::vector<double>(vacancies, vacancies + max_cluster_size),
      .dislocation_density = *dislocation_density};
}

MaterialImpl ClusterDynamicsCpuImpl::get_material() const { return material; }

void ClusterDynamicsCpuImpl::set_material(const MaterialImpl& material) {
  this->material = MaterialImpl(material);
}

NuclearReactorImpl ClusterDynamicsCpuImpl::get_reactor() const {
  return reactor;
}

void ClusterDynamicsCpuImpl::set_reactor(const NuclearReactorImpl& reactor) {
  this->reactor = NuclearReactorImpl(reactor);
}
