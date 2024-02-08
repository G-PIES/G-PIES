#include <stdio.h>
#include <cstring>

#include "cluster_dynamics_impl.hpp"



/** @brief Returns the rate of change of the concentration of interstitial clusters of size (n)
 * . \todo Document units
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
gp_float ClusterDynamicsImpl::i_concentration_derivative(size_t n) const
{
    return
        // (1)
        i_defect_production(n)
        // (2)
        + iemission_vabsorption_np1(n + 1) * interstitials[n + 1]
        // // (3)
        - iemission_vabsorption_n(n) * interstitials[n]
        // // (4)
        + iemission_vabsorption_nm1(n - 1) * interstitials[n - 1];
}



/** @brief Returns the rate of change of the concentration of vacancy clusters of size (n)
 * . \todo Document units
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
 * The Pokor paper's equation 2a only defines the derivative of interstitial cluster concentrations. However, the
 * paper implies that it also works for vacancy clusters in the line immediately preceding
 * the definition of 2a.
*/
gp_float ClusterDynamicsImpl::v_concentration_derivative(size_t vn) const
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



/** @brief Returns the rate of change in the concentration of size 1 interstitial clusters,
 * . \todo Document units
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
gp_float ClusterDynamicsImpl::i1_concentration_derivative() const
{
    return 
        // (1)
        i_defect_production(1)
        // (2)
        - annihilation_rate() * interstitials[1] * vacancies[1]
        // (3)
        - interstitials[1] * i_dislocation_annihilation_rate()
        // (4)
        - interstitials[1] * i_grain_boundary_annihilation_rate()
        // (5)
        - interstitials[1] * i_absorption_rate()
        // (6)
        + i_emission_rate();
}



/** @brief Returns the rate of change in the concentration of size 1 vacancy clusters,
 * . \todo Document units
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
gp_float ClusterDynamicsImpl::v1_concentration_derivative() const
{
    return 
        // (1)
        v_defect_production(1)
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
/** @brief Returns the rate of change of the density of the dislocation network. \todo Document units
 * 
 *  <hr>
 * 
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 8
 *  N. Sakaguchi / Acta Materialia 1131 (2001), Equation 3.14
 * 
 *  \f$
 *    \dwn{\frac{d\rho}{dt}=}
 *    \ann{1}{ \frac{2 \pi}{\Omega} \sum_{n>0} r_i(n) C_i(n) \beta_{i,i}(n) P_{unf}(n) }\dwn{-}
 *    \ann{2}{ Kb^2\rho^{3/2} \vphantom{\sum_{n>0}} }
 *  \f$
 * 
 * <b>Notes</b>
 * 
 * The gain term comes from the Sakaguchi paper's model of dislocation network evolution modified to use
 * notation more similar to the Pokor paper. Every time an
 * interstitial cluster grows by absorbing a size 1 interstitial, it has a \f$P_{unf}(n)\f$ probability of
 * becoming part of the dislocation network instead.
*/
gp_float ClusterDynamicsImpl::dislocation_density_derivative() const
{
   gp_float gain = 0.0;
   for (size_t n = 1; n < concentration_boundary; ++n)
   {
      gain += cluster_radius(n) * ii_absorption(n) * interstitials[n] * dislocation_promotion_probability(n);
   }

   gain *= 2. * M_PI / material.atomic_volume;

   return 
    // (1)
      gain
    // (2)
      - reactor.dislocation_density_evolution * std::pow(material.burgers_vector, 2.) * std::pow(dislocation_density, 3. / 2.);
}



/** @brief Returns the rate of production of interstital defects from the irradiation cascade for 
 * size (n) clusters. \todo Document units
 * 
 * <hr>
 * 
 * C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
 * 
 * \f$
 * \dwn{G_i(1)=}\ann{1}{\eta} \ann{2}{G_{dpa}}\dwn{(1-}\ann{3}{f_{i2}}\dwn{-}\ann{4}{f_{i3}}\dwn{-}\ann{5}{f_{i4})}\\
 * G_i(2)=\eta G_{dpa}f_{i2}\\
 * G_i(3)=\eta G_{dpa}f_{i3}\\
 * G_i(4)=\eta G_{dpa}f_{i4}
 * \f$
*/
gp_float ClusterDynamicsImpl::i_defect_production(size_t n) const
{
    switch (n)
    {
        //             (1)                     (2)
        case 1: return reactor.recombination * reactor.flux *
                       //    (3)            (4)             (5)
                       (1. - reactor.i_bi - reactor.i_tri - reactor.i_quad);
        case 2: return reactor.recombination * reactor.flux * reactor.i_bi;
        case 3: return reactor.recombination * reactor.flux * reactor.i_tri;
        case 4: return reactor.recombination * reactor.flux * reactor.i_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.;
}



/** @brief Returns the rate of production of vacancies defects from the irradiation cascade for 
 *  size (n) clusters. \todo Document units
 * 
 * <hr>
 * 
 * C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
 * 
 * \f$
 * \dwn{G_v(1)=}\ann{1}{\eta} \ann{2}{G_{dpa}}\dwn{(1-}\ann{3}{f_{v2}}\dwn{-}\ann{4}{f_{v3}}\dwn{-}\ann{5}{f_{v4})}\\
 * G_v(2)=\eta G_{dpa}f_{v2}\\
 * G_v(3)=\eta G_{dpa}f_{v3}\\
 * G_v(4)=\eta G_{dpa}f_{v4}
 * \f$
 * 
 * <b>Notes</b>
 * 
 * These equations are only implicitly provided in the Pokor paper. The equations for interstitial
 * generation are given followed by the line 'A similar expression is written for \f$G_v(n)\f$.'
*/
gp_float ClusterDynamicsImpl::v_defect_production(size_t n) const
{
    switch (n)
    {
        //             (1)                     (2)
        case 1: return reactor.recombination * reactor.flux *
                       //    (3)            (4)             (5)
                       (1. - reactor.v_bi - reactor.v_tri - reactor.v_quad);
        case 2: return reactor.recombination * reactor.flux * reactor.v_bi;
        case 3: return reactor.recombination * reactor.flux * reactor.v_tri;
        case 4: return reactor.recombination * reactor.flux * reactor.v_quad;

        default: break;
    }

    // cluster sizes > greater than 4 always zero
    return 0.;
}



/** @brief Returns the combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (np1)
 *. \todo Document units, both events leading to an interstitial loop of size n.
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
gp_float ClusterDynamicsImpl::iemission_vabsorption_np1(size_t np1) const
{
    return
        // (1)
        iv_absorption(np1) *
        // (2)
        vacancies[1] + 
        // (3)
        ii_emission(np1);
}



/** @brief Returns the combined rate of emission of an interstitial and absorption of a vacancy by an interstitial loop of size (np1)
 * . \todo Document units, both events leading to an interstitial loop of size n.
 * 
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
gp_float ClusterDynamicsImpl::vemission_iabsorption_np1(size_t np1) const
{
    return 
        // (1)
        vi_absorption(np1) * 
        // (2)
        interstitials[1] + 
        // (3)
        vv_emission(np1);
}



/** @brief Returns the rate that an interstitial cluster of size n can evolve toward a cluster of size
 *  n + 1 absorbing an interstitial, or toward a cluster of size
 *  n - 1 absorbing a vacancy or emitting an interstitial,
 * . \todo Document units
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
gp_float ClusterDynamicsImpl::iemission_vabsorption_n(size_t n) const
{
    return
        // (1)
        iv_absorption(n) * vacancies[1]
        // (2)
        + ii_absorption(n) * interstitials[1] 
        // (3)
        + ii_emission(n);
}



/** @brief Returns the rate that a vacancy cluster of size n can evolve toward a cluster of size
 *  n + 1 absorbing a vacancy, or toward a cluster of size
 *  n - 1 absorbing an interstitial or emitting an vacancy.
 *  \todo Document units
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
gp_float ClusterDynamicsImpl::vemission_iabsorption_n(size_t n) const
{
    return 
        // (1)
        vi_absorption(n) * interstitials[1] 
        // (2)
        + vv_absorption(n) * vacancies[1]
        // (3)
        + vv_emission(n);
}



/** @brief Returns the rate that an interstitial cluster of size n - 1 can evolve into a
 *  cluster of size n by absorbing an interstitial. \todo Document units
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
 *  modified to accomodate the dislocation network evolution model of the Sakaguchi paper. 
 *  \f$K_{1(j)}\f$ from the Sakaguchi paper corresponds to \f$\beta_{i,i}(n)C_i(1)\f$
 *  in the Pokor paper, both representing the rate of promotion to the next cluster size.
 *  Dr. Chen suggested using \f$P_{unf}(n)\f$, in a similar way to the Sakaguchi paper
 *  to determine what percentage of promoted clusters become part of the dislocation network.
*/
gp_float ClusterDynamicsImpl::iemission_vabsorption_nm1(size_t nm1) const
{
    return
        // (1)
        ii_absorption(nm1) 
        // (2)
        * interstitials[1]
        // (3)
        * (1 - dislocation_promotion_probability(nm1 + 1));
}



/** @brief Returns the rate that a vacancy cluster of size n - 1 can evolve into a
 *  cluster of size n by absorbing a vacancy,. \todo Document units
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
gp_float ClusterDynamicsImpl::vemission_iabsorption_nm1(size_t nm1) const
{
    return
        // (1)
        vv_absorption(nm1) * 
        // (2)
        vacancies[1];
}



/** @brief Returns one over the characteristic time for emitting an interstitial by 
 *  the population of interstital or vacancy clusters,. \todo Document units
 * 
 *  <hr>
 * 
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3b
 *  
 *  \f$
 *    \dwn{\frac{1}{\tau^e_i(n)} = }
 *    \ann{1}{\sum_{n>2} \alpha_{i,i}(n) C_i(n)}\dwn{+}
 *    \ann{2}{4 \alpha_{i,i}(2) C_i(2)}\dwn{+}
 *    \ann{3}{\beta_{i,v}(2) C_v(1) C_i(2)}
 *  \f$
 * 
 *  <b>Notes</b>
 * 
 *  (1) Combines the rates of emission of single interstitials by interstitial clusters of size > 2.
 * 
 *  (2) Represents the rate of emission of single interstitials by interstitial clusters of size = 2.
 *  This cannot be included in (1) because size 2 clusters create 2 single interstitials when they emit
 *  one. We don't yet understand why this coefficient is a 4 instead of a 2, and we suspect it is a mistake
 *  because this doesn't conserve the number of interstitial defects in the system. This seems to be
 *  related with a bug in our simulation where the number of interstitial defects is not conserved even
 *  when you turn off defect generation.
 * 
 *  (3) Represents the rate that interstitials are produced by interstitial clusters of size 2 absorbing a
 *  vacancy.
*/
gp_float ClusterDynamicsImpl::i_emission_rate() const
{
   gp_float time = 0.;
   for (size_t in = 3; in < concentration_boundary - 1; ++in)
   {
         time +=
         // (1)
         ii_emission(in) * interstitials[in];
   }

   time +=
      // (2)
      4. * ii_emission(2) * interstitials[2]
      // (3)
      + iv_absorption(2) * vacancies[1] * interstitials[2];

  return time;
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
 *  (1) Combines the rates of emission of single vacancies by vacancy clusters of size > 2.
 * 
 *  (2) Represents the rate of emission of single vacancies by vacancy clusters of size = 2.
 *  This cannot be included in (1) because size 2 clusters create 2 single vacancies when they emit
 *  one. There is a bug which is possibly related to this term, see i_emission_rate().
 * 
 *  (3) Represents the rate that vacancies are produced by vacancy clusters of size 2 absorbing a
 *  vacancy.
*/
gp_float ClusterDynamicsImpl::v_emission_rate() const
{
   gp_float time = 0.;
   for (size_t vn = 3; vn < concentration_boundary - 1; ++vn)
   {
      time += 
         // (1)
         vv_emission(vn) * vacancies[vn];
   }

   time +=
      // (2)
      4. * vv_emission(2) * vacancies[2]
      // (3)
      + vi_absorption(2) * interstitials[1] * vacancies[2];

   return time;
}



/** @brief Returns 1 over the characteristic time for absorbing an interstitial by 
 *  the population of interstital or vacancy clusters,. \todo Document units
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
*/
gp_float ClusterDynamicsImpl::i_absorption_rate() const
{
   gp_float time = ii_absorption(1) * interstitials[1];
   for (size_t in = 2; in < concentration_boundary - 1; ++in)
   {
      time +=
         // (1)
         ii_absorption(in) * interstitials[in]
         // (2)
         + vi_absorption(in) * vacancies[in];
   }

   return time;
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
gp_float ClusterDynamicsImpl::v_absorption_rate() const
{
   gp_float time = vv_absorption(1) * vacancies[1];
   for (size_t vn = 2; vn < concentration_boundary - 1; ++vn)
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

/** @brief Returns the annihilation rate of single vacancies and insterstitals. \todo Document units
 *  <hr>
 *  C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3d
 *  
 *  \f$
 *  \dwn{R_{iv} =}
 *  \ann{1}{4\pi}
 *  \ann{2}{(D_i + D_v)}
 *  \ann{3}{r_{iv}}
 *  \f$
*/
gp_float ClusterDynamicsImpl::annihilation_rate() const
{
    return 
        // (1)
        4. * M_PI * 
        // (2)
        (i_diffusion_val + v_diffusion_val) *
        // (3)
        material.recombination_radius;
}
// --------------------------------------------------------------------------------------------



/** @brief Returns the 1 over the characteristic time for annihilation of interstitials on dislocations. \todo Document units
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
gp_float ClusterDynamicsImpl::i_dislocation_annihilation_rate() const
{
    return
        // (1)
        dislocation_density *
        // (2)
        i_diffusion_val *
        // (3)
        material.i_dislocation_bias;
}





/** @brief Returns the 1 over the characteristic time for annihilation of vacancies on dislocations. \todo Document units
 * 
 * <hr>
 * 
 * This equation is based on an altered version of Equation 3e in the Pokor paper which uses the vacancy version of each variable.
 * See the line in the Pokor paper following this equation's definition for some justification.
 * 
 * C. Pokor / Journal of Nuclear Materials 326 (2004), Equation 3e
 * 
 * \f$\LARGE\frac{1}{\tau^a_{d,v}} = \rho D_v Z_v\f$
 * 
 * (1) \f$\rho\f$ 🡆 `dislocation_density`
 * 
 * (2) \f$D_v\f$ 🡆 `v_diffusion_val`
 * 
 * (3) \f$Z_v\f$ 🡆 `material.v_dislocation_bias`
*/
gp_float ClusterDynamicsImpl::v_dislocation_annihilation_rate() const
{
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
/** @brief Returns 1 over the characteristic time for annihilation of interstitials on grain boundaries
 * . \todo Document units
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
gp_float ClusterDynamicsImpl::i_grain_boundary_annihilation_rate() const
{
    return
        // (1)
        6. * i_diffusion_val *
        sqrt
        (
            // (2)
            dislocation_density * 
            material.i_dislocation_bias
            // (3)
            + ii_sum_absorption_val
            // (4)
            + vi_sum_absorption_val
        ) /
        // (5)
        material.grain_size;
}


// --------------------------------------------------------------------------------------------
/** @brief Returns 1 over the characteristic time for annihilation of vacancies on grain boundaries
 * . \todo Document units
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
gp_float ClusterDynamicsImpl::v_grain_boundary_annihilation_rate() const
{
    return
        // (1)
        6. * v_diffusion_val *
        sqrt
        (
            // (2)
            dislocation_density *
            material.v_dislocation_bias
            // (3)
            + vv_sum_absorption_val
            // (4)
            + iv_sum_absorption_val
        ) /
        // (5)
        material.grain_size;
}



/** @brief Returns the rate of emission of an interstitial by an interstital cluster of size (n)
 * . \todo Document units
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
gp_float ClusterDynamicsImpl::ii_emission(size_t n) const
{
    return 
        // (1)
        2. * M_PI * cluster_radius(n) *
        // (2)
        i_bias_factor(n) *
        // (3)
        i_diffusion_val / material.atomic_volume *
        // (4)
        exp
        (
            -i_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor.temperature)
        );
}




/** @brief Returns the rate of absorption of an interstitial by an interstital cluster of size (n),
 *. \todo Document units
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
gp_float ClusterDynamicsImpl::ii_absorption(size_t n) const
{
    return 
        // (1)
        2. * M_PI * cluster_radius(n) *
        // (2)
        i_bias_factor(n) *
        // (3)
        i_diffusion_val;
}



/** @brief Returns the rate of absorption of a vacancy by an interstitial cluster of size (n).
 * \todo Document units
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
gp_float ClusterDynamicsImpl::iv_absorption(size_t n) const
{
    return 
        // (1)
        2. * M_PI * cluster_radius(n) *
        // (2)
        v_bias_factor(n) *
        // (3)
        v_diffusion_val;
}



/** @brief Returns the rate of emission of an interstitial by an interstital cluster of size (n).
 *  \todo Document units
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
gp_float ClusterDynamicsImpl::vv_emission(size_t n) const
{
    return 
        // (1)
        2. * M_PI * cluster_radius(n) *
        // (2)
        v_bias_factor(n) *
        // (3)
        v_diffusion_val *
        // (4)
        exp
        (
            -v_binding_energy(n) /
            (BOLTZMANN_EV_KELVIN * reactor.temperature)
        );
}



/** @brief Returns the rate of absorption of a vacancy by a vacancy cluster of size (n).
 *  \todo Document units
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
gp_float ClusterDynamicsImpl::vv_absorption(size_t n) const
{
    return 
        // (1)
        2. * M_PI * cluster_radius(n) *
        // (2)
        v_bias_factor(n) *
        // (3)
        v_diffusion_val;
}



/** @brief Returns the rate of absorption of an interstitial by a vacancy cluster of size (n).
 *  \todo Document units
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
gp_float ClusterDynamicsImpl::vi_absorption(size_t n) const
{
    return 
        2. * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        i_diffusion_val;
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
gp_float ClusterDynamicsImpl::i_bias_factor(size_t n) const
{
    return 
        // (1)
        material.i_dislocation_bias +
        (
            // (2)
            std::sqrt
            (
                    material.burgers_vector /
                    (8. * M_PI * material.lattice_param)
            ) *
            // (3)
            material.i_loop_bias -
            // (4)
            material.i_dislocation_bias
        ) *
        // (5)
        1. /
        std::pow
        (
            (gp_float)n,
            material.i_dislocation_bias_param / 2.
        );
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
gp_float ClusterDynamicsImpl::v_bias_factor(size_t n) const
{
    return 
        material.v_dislocation_bias +
        (
            std::sqrt
            (
                    material.burgers_vector /
                    (8. * M_PI * material.lattice_param)
            ) *
            material.v_loop_bias -
            material.v_dislocation_bias
        ) *
        1. /
        std::pow
        (
            (gp_float)n,
            material.v_dislocation_bias_param / 2.
        );
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
gp_float ClusterDynamicsImpl::i_binding_energy(size_t n) const
{
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
gp_float ClusterDynamicsImpl::v_binding_energy(size_t n) const
{
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
 *  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, Equation 4.59
 * 
 *  \f$
 *    \dwn{D_i =}
 *    \ann{1}{D_{i0} \vphantom{\frac{-E_{mi}}{kT}} }
 *    \ann{2}{exp(\frac{-E_{mi}}{kT}) }
 *  \f$
*/
gp_float ClusterDynamicsImpl::i_diffusion() const
{
    //     (1)                      (2)
    return material.i_diffusion_0 * std::exp(-material.i_migration / (BOLTZMANN_EV_KELVIN * reactor.temperature));
}



/** @brief Returns the diffusion coefficient for single vacancies. 
 *  \todo Document units
 * 
 *  <hr>
 * 
 *  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, Equation 4.59
 * 
 *  \f$
 *    \dwn{D_v =}
 *    \ann{1}{D_{v0} \vphantom{\frac{-E_{mv}}{kT}} }
 *    \ann{2}{exp(\frac{-E_{mv}}{kT}) }
 *  \f$
*/  
gp_float ClusterDynamicsImpl::v_diffusion() const
{
   //     (1)                      (2)
   return material.v_diffusion_0 * std::exp(-material.v_migration / (BOLTZMANN_EV_KELVIN * reactor.temperature));
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
gp_float ClusterDynamicsImpl::mean_dislocation_cell_radius() const
{
   gp_float r_0_factor = 0.;
   for (size_t i = 1; i < concentration_boundary; ++i)
   {
      r_0_factor += cluster_radius(i) * interstitials[i];
   }

                     // (1)                                           (2)          (3)                       
   return 1 / std::sqrt((2. * M_PI * M_PI / material.atomic_volume) * r_0_factor + M_PI * dislocation_density);
}



// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.12
*/
/** @brief Returns the probability that an interstitial cluster of size n will unfurl to join the dislocation
 *  network when it grows to size n + 1. 
 *  \todo Document units
 *  \todo Consider reworking the equation to remove the squaring of mean_dislocation_radius_val. 
 *  We also do a sqrt to calculate mean_dislocation_radius_val and they'll cancel out.
 * 
 *  <hr>
 * 
 *  N. Sakaguchi / Acta Materialia 1131 (2001), Equation 3.12
 * 
 *  \f$
 *    \dwn{P_{unf}(n) =}
 *    \frac
 *       {\ann{1}{2r_i(n)(r_i(n+1) - r_i(n))} \dwn{+} \ann{2}{(r_i(n+1)-r_i(n))^2}}
 *       {\ann{3}{(\pi r_0 / 2)^2} \dwn{-} \ann{4}{r_i(n)^2}}
 *  \f$
*/
gp_float ClusterDynamicsImpl::dislocation_promotion_probability(size_t n) const
{
   gp_float dr = cluster_radius(n + 1) - cluster_radius(n);

   //      (1)                           (2)
   return (2. * cluster_radius(n) * dr + std::pow(dr, 2.)) 
   //    (3)                                                    (4)
      / (std::pow(M_PI * mean_dislocation_radius_val / 2., 2) - std::pow(cluster_radius(n), 2.)); 
}



// --------------------------------------------------------------------------------------------
/*  
*/
/** @brief Returns the cluster radius of a cluster of size (n) in cm.
 *  
 * <hr>
 * 
 * G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg. 346, Equation 7.63
 * 
 * \f$ 
 * r_i = (\frac{\sqrt{3}a^2n}{4 \pi})^{-1/2}
 * \f$
*/
gp_float ClusterDynamicsImpl::cluster_radius(size_t n) const
{
    return std::sqrt(std::sqrt(3.) * std::pow(material.lattice_param, 2.) * (gp_float)n / (4. * M_PI));
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
  i_diffusion_val = i_diffusion();
  v_diffusion_val = v_diffusion();
  ii_sum_absorption_val = ii_sum_absorption(concentration_boundary - 1);
  iv_sum_absorption_val = iv_sum_absorption(concentration_boundary - 1);
  vi_sum_absorption_val = vi_sum_absorption(concentration_boundary - 1);
  vv_sum_absorption_val = vv_sum_absorption(concentration_boundary - 1);
  mean_dislocation_radius_val = mean_dislocation_cell_radius();
}

bool ClusterDynamicsImpl::step(gp_float delta_time)
{
  step_init();

  bool state_is_valid = update_clusters_1(delta_time);
  update_clusters(delta_time);
  dislocation_density += dislocation_density_derivative() * delta_time;

  interstitials = interstitials_temp;
  vacancies = vacancies_temp;

  return state_is_valid;
}

bool ClusterDynamicsImpl::update_clusters_1(gp_float delta_time)
{
  interstitials_temp[1] += i1_concentration_derivative() * delta_time;
  vacancies_temp[1] += v1_concentration_derivative() * delta_time;
  return validate(1);
}

bool ClusterDynamicsImpl::update_clusters(gp_float delta_time)
{
   bool state_is_valid = true;

   for (size_t n = 2; n < concentration_boundary; ++n)
   {
      interstitials_temp[n] += i_concentration_derivative(n) * delta_time;
      vacancies_temp[n] += v_concentration_derivative(n) * delta_time;

      state_is_valid = state_is_valid && validate(n);
   }

   return state_is_valid;
}

ClusterDynamicsImpl::~ClusterDynamicsImpl()
{
}

gp_float ClusterDynamicsImpl::ii_sum_absorption(size_t nmax) const
{
   gp_float emission = 0.;
   for (size_t vn = 1; vn < nmax; ++vn)
   {
      emission += ii_absorption(vn) * interstitials[vn];
   }

   return emission;
}

gp_float ClusterDynamicsImpl::iv_sum_absorption(size_t nmax) const
{
   gp_float emission = 0.;
   for (size_t n = 1; n < nmax; ++n)
   {
      emission += iv_absorption(n) * interstitials[n];
   }

   return emission;
}

gp_float ClusterDynamicsImpl::vv_sum_absorption(size_t nmax) const
{
   gp_float emission = 0.;
   for (size_t n = 1; n < nmax; ++n)
   {
      emission += vv_absorption(n) * vacancies[n];
   }

   return emission;
}

gp_float ClusterDynamicsImpl::vi_sum_absorption(size_t nmax) const
{
   gp_float emission = 0.;
   for (size_t n = 1; n < nmax; ++n)
   {
      emission += vi_absorption(n) * vacancies[n];
   }

   return emission;
}

bool ClusterDynamicsImpl::validate(size_t n) const
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



//!< \todo Clean up the uses of random +1/+2/-1/etc throughout the code
ClusterDynamicsImpl::ClusterDynamicsImpl(size_t concentration_boundary, const NuclearReactorImpl& reactor, const MaterialImpl& material)
  : time(0.0),
    interstitials(concentration_boundary + 1, 0.0), interstitials_temp(concentration_boundary + 1, 0.0),
    vacancies(concentration_boundary + 1, 0.0), vacancies_temp(concentration_boundary + 1, 0.0),
    concentration_boundary(concentration_boundary), dislocation_density(material.dislocation_density_0), 
    material(material), reactor(reactor)
{
}

ClusterDynamicsState ClusterDynamicsImpl::run(gp_float delta_time, gp_float total_time)
{
    bool state_is_valid = true;

    for (gp_float endtime = time + total_time; time < endtime; time += delta_time)
    {
        state_is_valid = step(delta_time);
        if (!state_is_valid) break;
    }

    return ClusterDynamicsState 
    {
        .valid = state_is_valid,
        .time = time,
        .interstitials = std::vector<gp_float>(interstitials.begin(), interstitials.end() - 1),
        .vacancies = std::vector<gp_float>(vacancies.begin(), vacancies.end() - 1),
        .dislocation_density = dislocation_density
    };
}

MaterialImpl ClusterDynamicsImpl::get_material() const
{
    return material;
}

void ClusterDynamicsImpl::set_material(const MaterialImpl& material)
{
    this->material = MaterialImpl(material);
}

NuclearReactorImpl ClusterDynamicsImpl::get_reactor() const
{
    return reactor;
}

void ClusterDynamicsImpl::set_reactor(const NuclearReactorImpl& reactor)
{
    this->reactor = NuclearReactorImpl(reactor);
}