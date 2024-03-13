#ifndef __METAL__
#include <cmath>
#endif

#include "cluster_dynamics_metal_kernel.hpp"
#include "constants.hpp"

gp_float ClusterDynamicsMetalKernel::i_concentration_derivative(uint64_t n)
    __METALDECL__ {
  return i_defect_production(n) +
         i_demotion_rate(n + 1) * interstitials[n + 1] -
         i_combined_promotion_demotion_rate(n) * interstitials[n] +
         i_promotion_rate(n - 1) * interstitials[n - 1];
}

gp_float ClusterDynamicsMetalKernel::v_concentration_derivative(uint64_t vn)
    __METALDECL__ {
  return v_defect_production(vn) + v_demotion_rate(vn + 1) * vacancies[vn + 1] -
         v_combined_promotion_demotion_rate(vn) * vacancies[vn] +
         v_promotion_rate(vn - 1) * vacancies[vn - 1];
}

gp_float ClusterDynamicsMetalKernel::i1_concentration_derivative()
    __METALDECL__ {
  return i_defect_production(1) -
         annihilation_rate() * interstitials[1] * vacancies[1] -
         interstitials[1] * i_dislocation_annihilation_rate() -
         interstitials[1] * i_grain_boundary_annihilation_rate() -
         interstitials[1] * i_absorption_rate() + i_emission_rate();
}

gp_float ClusterDynamicsMetalKernel::v1_concentration_derivative()
    __METALDECL__ {
  return v_defect_production(1) -
         annihilation_rate() * interstitials[1] * vacancies[1] -
         vacancies[1] * v_dislocation_annihilation_rate() -
         vacancies[1] * v_grain_boundary_annihilation_rate() -
         vacancies[1] * v_absorption_rate() + v_emission_rate();
}

gp_float ClusterDynamicsMetalKernel::dislocation_density_derivative()
    __METALDECL__ {
  gp_float gain = 0.0;
  for (uint64_t n = 1; n < max_cluster_size; ++n) {
    gain += cluster_radius(n) * ii_absorption(n) * interstitials[n] *
            dislocation_promotion_probability(n);
  }

  gain *= 2. * M_PI / material.atomic_volume;

  return gain - reactor.dislocation_density_evolution *
                    mtl_math::pow(material.burgers_vector, 2.) *
                    mtl_math::pow(dislocation_density, 3. / 2.);
}

gp_float ClusterDynamicsMetalKernel::i_defect_production(uint64_t n)
    __METALDECL__ {
  switch (n) {
    case 1:
      return reactor.recombination * reactor.flux *
             (1. - reactor.i_tri - reactor.i_quad);
    case 2:
      return 0.;
    case 3:
      return reactor.recombination * reactor.flux * reactor.i_tri;
    case 4:
      return reactor.recombination * reactor.flux * reactor.i_quad;

    default:
      break;
  }

  return 0.;
}

gp_float ClusterDynamicsMetalKernel::v_defect_production(uint64_t n)
    __METALDECL__ {
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

  return 0.;
}

gp_float ClusterDynamicsMetalKernel::i_demotion_rate(uint64_t n) __METALDECL__ {
  return iv_absorption(n) * vacancies[1] + ii_emission(n);
}

gp_float ClusterDynamicsMetalKernel::v_demotion_rate(uint64_t n) __METALDECL__ {
  return vi_absorption(n) * interstitials[1] + vv_emission(n);
}

gp_float ClusterDynamicsMetalKernel::i_combined_promotion_demotion_rate(
    uint64_t n) __METALDECL__ {
  return iv_absorption(n) * vacancies[1] + ii_absorption(n) * interstitials[1] +
         ii_emission(n);
}

gp_float ClusterDynamicsMetalKernel::v_combined_promotion_demotion_rate(
    uint64_t n) __METALDECL__ {
  return vi_absorption(n) * interstitials[1] + vv_absorption(n) * vacancies[1] +
         vv_emission(n);
}

gp_float ClusterDynamicsMetalKernel::i_promotion_rate(uint64_t n)
    __METALDECL__ {
  return ii_absorption(n) * interstitials[1] *
         (1 - dislocation_promotion_probability(n + 1));
}

gp_float ClusterDynamicsMetalKernel::v_promotion_rate(uint64_t n)
    __METALDECL__ {
  return vv_absorption(n) * vacancies[1];
}

gp_float ClusterDynamicsMetalKernel::i_emission_rate() __METALDECL__ {
  gp_float time = 0.;
  for (uint64_t in = 4; in < max_cluster_size - 1; ++in) {
    time += ii_emission(in) * interstitials[in];
  }

  time += 3. * ii_emission(3) * interstitials[3] +
          2. * iv_absorption(3) * vacancies[1] * interstitials[3];

  return time;
}

gp_float ClusterDynamicsMetalKernel::v_emission_rate() __METALDECL__ {
  gp_float time = 0.;
  for (uint64_t vn = 3; vn < max_cluster_size - 1; ++vn) {
    time += vv_emission(vn) * vacancies[vn];
  }

  time += 4. * vv_emission(2) * vacancies[2] +
          vi_absorption(2) * interstitials[1] * vacancies[2];

  return time;
}

gp_float ClusterDynamicsMetalKernel::i_absorption_rate() __METALDECL__ {
  gp_float time = ii_absorption(1) * interstitials[1];
  for (uint64_t in = 2; in < max_cluster_size - 1; ++in) {
    time += ii_absorption(in) * interstitials[in] +
            vi_absorption(in) * vacancies[in];
  }

  return time;
}

gp_float ClusterDynamicsMetalKernel::v_absorption_rate() __METALDECL__ {
  gp_float time = vv_absorption(1) * vacancies[1];
  for (uint64_t vn = 2; vn < max_cluster_size - 1; ++vn) {
    time += vv_absorption(vn) * vacancies[vn] +
            iv_absorption(vn) * interstitials[vn];
  }

  return time;
}

gp_float ClusterDynamicsMetalKernel::annihilation_rate() __METALDECL__ {
  return 4. * M_PI * (i_diffusion_val + v_diffusion_val) *
         material.recombination_radius;
}

gp_float ClusterDynamicsMetalKernel::i_dislocation_annihilation_rate()
    __METALDECL__ {
  return dislocation_density * i_diffusion_val * material.i_dislocation_bias;
}

gp_float ClusterDynamicsMetalKernel::v_dislocation_annihilation_rate()
    __METALDECL__ {
  return dislocation_density * v_diffusion_val * material.v_dislocation_bias;
}

gp_float ClusterDynamicsMetalKernel::i_grain_boundary_annihilation_rate()
    __METALDECL__ {
  return 6. * i_diffusion_val *
         mtl_math::sqrt(dislocation_density * material.i_dislocation_bias +
                        ii_sum_absorption_val + vi_sum_absorption_val) /
         material.grain_size;
}

gp_float ClusterDynamicsMetalKernel::v_grain_boundary_annihilation_rate()
    __METALDECL__ {
  return 6. * v_diffusion_val *
         mtl_math::sqrt(dislocation_density * material.v_dislocation_bias +
                        vv_sum_absorption_val + iv_sum_absorption_val) /
         material.grain_size;
}

gp_float ClusterDynamicsMetalKernel::ii_emission(uint64_t n) __METALDECL__ {
  return 2. * M_PI * cluster_radius(n) * i_bias_factor(n) * i_diffusion_val /
         material.atomic_volume *
         mtl_math::exp(-i_binding_energy(n) /
                       (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

gp_float ClusterDynamicsMetalKernel::ii_absorption(uint64_t n) __METALDECL__ {
  return 2. * M_PI * cluster_radius(n) * i_bias_factor(n) * i_diffusion_val;
}

gp_float ClusterDynamicsMetalKernel::iv_absorption(uint64_t n) __METALDECL__ {
  return 2. * M_PI * cluster_radius(n) * v_bias_factor(n) * v_diffusion_val;
}

gp_float ClusterDynamicsMetalKernel::vv_emission(uint64_t n) __METALDECL__ {
  return 2. * M_PI * cluster_radius(n) * v_bias_factor(n) * v_diffusion_val *
         mtl_math::exp(-v_binding_energy(n) /
                       (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

gp_float ClusterDynamicsMetalKernel::vv_absorption(uint64_t n) __METALDECL__ {
  return 2. * M_PI * cluster_radius(n) * v_bias_factor(n) * v_diffusion_val;
}

gp_float ClusterDynamicsMetalKernel::vi_absorption(uint64_t n) __METALDECL__ {
  return 2. * M_PI * cluster_radius(n) * i_bias_factor(n) * i_diffusion_val;
}

gp_float ClusterDynamicsMetalKernel::i_bias_factor(uint64_t n) __METALDECL__ {
  return material.i_dislocation_bias +
         (mtl_math::sqrt(material.burgers_vector /
                         (8. * M_PI * material.lattice_param)) *
              material.i_loop_bias -
          material.i_dislocation_bias) *
             1. /
             mtl_math::pow((gp_float)n, material.i_dislocation_bias_param / 2.);
}

gp_float ClusterDynamicsMetalKernel::v_bias_factor(uint64_t n) __METALDECL__ {
  return material.v_dislocation_bias +
         (mtl_math::sqrt(material.burgers_vector /
                         (8. * M_PI * material.lattice_param)) *
              material.v_loop_bias -
          material.v_dislocation_bias) *
             1. /
             mtl_math::pow((gp_float)n, material.v_dislocation_bias_param / 2.);
}

gp_float ClusterDynamicsMetalKernel::i_binding_energy(uint64_t n)
    __METALDECL__ {
  return material.i_formation + (material.i_binding - material.i_formation) /
                                    (mtl_math::pow(2., .8) - 1.) *
                                    (mtl_math::pow((gp_float)n, .8) -
                                     mtl_math::pow((gp_float)n - 1., .8));
}

gp_float ClusterDynamicsMetalKernel::v_binding_energy(uint64_t n)
    __METALDECL__ {
  return material.v_formation + (material.v_binding - material.v_formation) /
                                    (mtl_math::pow(2., .8) - 1) *
                                    (mtl_math::pow((gp_float)n, .8) -
                                     mtl_math::pow((gp_float)n - 1., .8));
}

gp_float ClusterDynamicsMetalKernel::i_diffusion() __METALDECL__ {
  return material.i_diffusion_0 *
         mtl_math::exp(-material.i_migration /
                       (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

gp_float ClusterDynamicsMetalKernel::v_diffusion() __METALDECL__ {
  return material.v_diffusion_0 *
         mtl_math::exp(-material.v_migration /
                       (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

gp_float ClusterDynamicsMetalKernel::mean_dislocation_cell_radius()
    __METALDECL__ {
  gp_float r_0_factor = 0.;
  for (uint64_t i = 1; i < max_cluster_size; ++i) {
    r_0_factor += cluster_radius(i) * interstitials[i];
  }

  return 1 / mtl_math::sqrt((2. * M_PI * M_PI / material.atomic_volume) *
                                r_0_factor +
                            M_PI * dislocation_density);
}

gp_float ClusterDynamicsMetalKernel::dislocation_promotion_probability(
    uint64_t n) __METALDECL__ {
  gp_float dr = cluster_radius(n + 1) - cluster_radius(n);

  return (2. * cluster_radius(n) * dr + mtl_math::pow(dr, 2.)) /
         (mtl_math::pow(M_PI * mean_dislocation_radius_val / 2., 2) -
          mtl_math::pow(cluster_radius(n), 2.));
}

gp_float ClusterDynamicsMetalKernel::cluster_radius(uint64_t n) __METALDECL__ {
  return mtl_math::sqrt(mtl_math::sqrt(3.) *
                        mtl_math::pow(material.lattice_param, 2.) *
                        (gp_float)n / (4. * M_PI));
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/*
 *  SIMULATION CONTROL FUNCTIONS
 */
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

void ClusterDynamicsMetalKernel::step_init() __METALDECL__ {
  i_diffusion_val = i_diffusion();
  v_diffusion_val = v_diffusion();
  ii_sum_absorption_val = ii_sum_absorption(max_cluster_size - 1);
  iv_sum_absorption_val = iv_sum_absorption(max_cluster_size - 1);
  vi_sum_absorption_val = vi_sum_absorption(max_cluster_size - 1);
  vv_sum_absorption_val = vv_sum_absorption(max_cluster_size - 1);
  mean_dislocation_radius_val = mean_dislocation_cell_radius();
}

void ClusterDynamicsMetalKernel::update_clusters_1(gp_float delta_time)
    __METALDECL__ {
  interstitials[1] += i1_concentration_derivative() * delta_time;
  vacancies[1] += v1_concentration_derivative() * delta_time;
}

void ClusterDynamicsMetalKernel::update_dislocation_density(gp_float delta_time)
    __METALDECL__ {
  dislocation_density += dislocation_density_derivative() * delta_time;
}

gp_float ClusterDynamicsMetalKernel::ii_sum_absorption(uint64_t nmax)
    __METALDECL__ {
  gp_float emission = 0.;
  for (uint64_t vn = 1; vn < nmax; ++vn) {
    emission += ii_absorption(vn) * interstitials[vn];
  }

  return emission;
}

gp_float ClusterDynamicsMetalKernel::iv_sum_absorption(uint64_t nmax)
    __METALDECL__ {
  gp_float emission = 0.;
  for (uint64_t n = 1; n < nmax; ++n) {
    emission += iv_absorption(n) * interstitials[n];
  }

  return emission;
}

gp_float ClusterDynamicsMetalKernel::vv_sum_absorption(uint64_t nmax)
    __METALDECL__ {
  gp_float emission = 0.;
  for (uint64_t n = 1; n < nmax; ++n) {
    emission += vv_absorption(n) * vacancies[n];
  }

  return emission;
}

gp_float ClusterDynamicsMetalKernel::vi_sum_absorption(uint64_t nmax)
    __METALDECL__ {
  gp_float emission = 0.;
  for (uint64_t n = 1; n < nmax; ++n) {
    emission += vi_absorption(n) * vacancies[n];
  }

  return emission;
}
