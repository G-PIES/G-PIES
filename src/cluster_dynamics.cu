#include <stdio.h>
#include "material.hpp"
#include "nuclear_reactor.hpp"

class CDSimulation
{
public:
  double* interstitials;
  double* interstitials_temp;

  double* vacancies;
  double* vacancies_temp;

  size_t concentration_boundary;
  double dislocation_density;

  Material material;
  NuclearReactor reactor;

  __device__  double i_defect_production(size_t);
  __device__  double v_defect_production(size_t);
  __device__  double i_clusters_delta(size_t);
  __device__  double v_clusters_delta(size_t);
  __device__  double iemission_vabsorption_np1(size_t);
  __device__  double vemission_iabsorption_np1(size_t);
  __device__  double iemission_vabsorption_n(size_t);
  __device__  double vemission_iabsorption_n(size_t);
  __device__  double iemission_vabsorption_nm1(size_t);
  __device__  double vemission_iabsorption_nm1(size_t);
  __device__  double i1_cluster_delta(size_t);
  __device__  double v1_cluster_delta(size_t);
  __device__  double i_emission_time(size_t);
  __device__  double v_emission_time(size_t);
  __device__  double i_absorption_time(size_t);
  __device__  double v_absorption_time(size_t);
  __device__  double annihilation_rate();
  __device__  double i_dislocation_annihilation_time();
  __device__  double v_dislocation_annihilation_time();
  __device__  double i_grain_boundary_annihilation_time(size_t);
  __device__  double v_grain_boundary_annihilation_time(size_t);
  __device__  double ii_sum_absorption(size_t);
  __device__  double iv_sum_absorption(size_t);
  __device__  double vv_sum_absorption(size_t);
  __device__  double vi_sum_absorption(size_t);
  __device__  double ii_emission(size_t);
  __device__  double ii_absorption(size_t);
  __device__  double iv_absorption(size_t);
  __device__  double vv_emission(size_t);
  __device__  double vv_absorption(size_t);
  __device__  double vi_absorption(size_t);
  __device__  double i_bias_factor(size_t);
  __device__  double v_bias_factor(size_t);
  __device__  double i_binding_energy(size_t);
  __device__  double v_binding_energy(size_t);
  __device__  double i_diffusion();
  __device__  double v_diffusion();
  __device__  double mean_dislocation_cell_radius(size_t);
  __device__  double dislocation_promotion_probability(size_t);
  __device__  double dislocation_density_delta();
  __device__  double cluster_radius(size_t);
public:
  CDSimulation(size_t concentration_boundary, double* is_in, double* vs_in, double* is_out, double* vs_out, 
               double dislocation_density, Material material, NuclearReactor reactor)
    : concentration_boundary(concentration_boundary), interstitials(is_in), vacancies(vs_in), interstitials_temp(is_out), vacancies_temp(vs_out), 
      dislocation_density(dislocation_density), material(material), reactor(reactor)
  {
  }

  __device__ double update_clusters(size_t n, double delta_time);
};

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 1a-1e
    The rate of production of interstital defects from the irradiation cascade for size (n) clusters.
*/
__device__ double CDSimulation::i_defect_production(size_t n)
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
__device__ double CDSimulation::v_defect_production(size_t n)
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
__device__ double CDSimulation::i_clusters_delta(size_t in)
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
__device__ double CDSimulation::v_clusters_delta(size_t vn)
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
__device__ double CDSimulation::iemission_vabsorption_np1(size_t np1)
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
__device__ double CDSimulation::vemission_iabsorption_np1(size_t np1)
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
__device__ double CDSimulation::iemission_vabsorption_n(size_t n)
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
__device__ double CDSimulation::vemission_iabsorption_n(size_t n)
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
__device__ double CDSimulation::iemission_vabsorption_nm1(size_t nm1)
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
__device__ double CDSimulation::vemission_iabsorption_nm1(size_t nm1)
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
__device__ double CDSimulation::i1_cluster_delta(size_t nmax)
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
__device__ double CDSimulation::v1_cluster_delta(size_t nmax)
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
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 3d
    Annihilation rate of vacancies and insterstitals.

          (1)      (2)         (3)
    Riv = 4 * PI * (Di + Dv) * riv
*/
__device__ double CDSimulation::annihilation_rate()
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
__device__ double CDSimulation::i_dislocation_annihilation_time()
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
__device__ double CDSimulation::v_dislocation_annihilation_time()
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
__device__ double CDSimulation::i_grain_boundary_annihilation_time(size_t in)
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
__device__ double CDSimulation::v_grain_boundary_annihilation_time(size_t vn)
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

// --------------------------------------------------------------------------------------------
/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4a
    Rate of emission of an interstitial by an interstital loop of size (n).
*/
__device__ double CDSimulation::ii_emission(size_t n)
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
__device__ double CDSimulation::ii_absorption(size_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        i_bias_factor(n) *
        i_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4c
    Rate of absorption of an interstitial by a vacancy loop of size (n).
*/
__device__ double CDSimulation::iv_absorption(size_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        v_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4d
    Rate of emission of a vacancy by a vacancy loop of size (n).
*/
__device__ double CDSimulation::vv_emission(size_t n)
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
__device__ double CDSimulation::vv_absorption(size_t n)
{
    return 
        2 * M_PI * cluster_radius(n) *
        v_bias_factor(n) *
        v_diffusion();
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 4f
    Rate of absorption of a vacancy by an interstitial loop of size (n).
*/
__device__ double CDSimulation::vi_absorption(size_t n)
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
__device__ double CDSimulation::i_bias_factor(size_t in)
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
__device__ double CDSimulation::v_bias_factor(size_t vn)
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
__device__ double CDSimulation::i_binding_energy(size_t in)
{
    return
        material.i_formation
        + (material.i_binding - material.i_formation) / (std::pow(2., .8) - 1) *
        (std::pow(in, .8) - std::pow(in - 1., .8));
}

/*  C. Pokor / Journal of Nuclear Materials 326 (2004), 6
    Vacancy binding energy.
*/
__device__ double CDSimulation::v_binding_energy(size_t vn)
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
__device__ double CDSimulation::i_diffusion()
{
    return material.i_diffusion_0 * std::exp(-material.i_migration / (BOLTZMANN_EV_KELVIN * reactor.temperature));
}

/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg 193, 4.59
*/  
__device__ double CDSimulation::v_diffusion()
{
    return material.v_diffusion_0 * std::exp(-material.v_migration / (BOLTZMANN_EV_KELVIN * reactor.temperature));
}
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.10
*/
__device__ double CDSimulation::mean_dislocation_cell_radius(size_t n)
{
    double r_0_factor = 0.;
    for (size_t i = 1; i < concentration_boundary; ++i)
    {
        r_0_factor += cluster_radius(i) * interstitials[i];
    }

    return 1 / std::sqrt((2 * M_PI * M_PI / material.atomic_volume) * r_0_factor + M_PI * dislocation_density);
}

// --------------------------------------------------------------------------------------------
/*  N. Sakaguchi / Acta Materialia 1131 (2001), 3.12
*/
__device__ double CDSimulation::dislocation_promotion_probability(size_t n)
{
    double dr = cluster_radius(n + 1) - cluster_radius(n);

    double r_0 = mean_dislocation_cell_radius(n);

    return (2 * cluster_radius(n) * dr + std::pow(dr, 2)) 
         / (M_PI * r_0 / 2. - std::pow(cluster_radius(n), 2)); 
}
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
/*  G. Was / Fundamentals of Radiation Materials Science (2nd Edition) (2017), pg. 346, 7.63
*/
__device__ double CDSimulation::cluster_radius(size_t n)
{
    return std::sqrt(std::sqrt(3.0) * std::pow(material.lattice_param, 2) * (double)n / (4 * M_PI));
}
// --------------------------------------------------------------------------------------------

__device__ double CDSimulation::update_clusters(size_t n, double delta_time)
{
  if (n >= concentration_boundary) return;
  
  interstitials_temp[n] += i_clusters_delta(n) * delta_time;
  vacancies_temp[n] += v_clusters_delta(n) * delta_time;
}

__global__ void cd_kernel(CDSimulation* simulation, double delta_time)
{
  size_t index = 32 * blockIdx.x + threadIdx.x + 2; // Idx 0 is empty, 1 is handled outside CUDA
  simulation->update_clusters(index, delta_time);
}

__host__ bool CUDA_update_clusters(size_t concentration_boundary, double delta_time, double* is_in, double* vs_in, double* is_out, double* vs_out, 
                                   double dislocation_density, Material material, NuclearReactor reactor)
{
  double* d_is_in;
  double* d_is_out;
  double* d_vs_in;
  double* d_vs_out;
  CDSimulation* d_simulation;

  size_t arraysize = concentration_boundary * sizeof(double);

  cudaMalloc((void**)&d_is_in, arraysize);
  cudaMalloc((void**)&d_is_out, arraysize);
  cudaMalloc((void**)&d_vs_in, arraysize);
  cudaMalloc((void**)&d_vs_out, arraysize);
  cudaMalloc((void**)&d_simulation, sizeof(CDSimulation));

  cudaMemcpy(d_is_in, is_in, arraysize, cudaMemcpyHostToDevice);
  cudaMemcpy(d_vs_in, vs_in, arraysize, cudaMemcpyHostToDevice);
  cudaMemcpy(d_is_out, is_out, arraysize, cudaMemcpyHostToDevice);
  cudaMemcpy(d_vs_out, vs_out, arraysize, cudaMemcpyHostToDevice);

  CDSimulation* simulation = new CDSimulation(concentration_boundary, d_is_in, d_vs_in, d_is_out, d_vs_out, dislocation_density, material, reactor);
  cudaMemcpy(d_simulation, simulation, sizeof(CDSimulation), cudaMemcpyHostToDevice);

  cd_kernel<<<concentration_boundary - 2, 32>>>(d_simulation, delta_time);
  cudaError_t error = cudaGetLastError();
  if (error != cudaSuccess)
  {
    fprintf(stderr, "\nCUDA Error: %s", cudaGetErrorString(error));
  }

  cudaMemcpy(is_out + 2, d_is_out + 2, arraysize - 2 * sizeof(double), cudaMemcpyDeviceToHost);
  cudaMemcpy(vs_out + 2, d_vs_out + 2, arraysize - 2 * sizeof(double), cudaMemcpyDeviceToHost);

  cudaFree(d_is_in);
  cudaFree(d_is_out);
  cudaFree(d_vs_in);
  cudaFree(d_vs_out);
  cudaFree(d_simulation);
  delete simulation;

  return true; // TODO - get validation for each concentration
}