#include <metal_stdlib>

#include "utils/constants.hpp"
#include "cluster_dynamics_metal_args.hpp"

gp_float i_concentration_derivative(device ClusterDynamicsMetalArgs& args, uint);
gp_float v_concentration_derivative(device ClusterDynamicsMetalArgs& args, uint);
gp_float i1_concentration_derivative(device ClusterDynamicsMetalArgs& args);
gp_float v1_concentration_derivative(device ClusterDynamicsMetalArgs& args);
gp_float dislocation_density_derivative(device ClusterDynamicsMetalArgs& args);
gp_float i_defect_production(device ClusterDynamicsMetalArgs& args, uint);
gp_float v_defect_production(device ClusterDynamicsMetalArgs& args, uint);
gp_float iemission_vabsorption_np1(device ClusterDynamicsMetalArgs& args, uint);
gp_float vemission_iabsorption_np1(device ClusterDynamicsMetalArgs& args, uint);
gp_float iemission_vabsorption_n(device ClusterDynamicsMetalArgs& args, uint);
gp_float vemission_iabsorption_n(device ClusterDynamicsMetalArgs& args, uint);
gp_float iemission_vabsorption_nm1(device ClusterDynamicsMetalArgs& args, uint);
gp_float vemission_iabsorption_nm1(device ClusterDynamicsMetalArgs& args, uint);
gp_float i_emission_rate(device ClusterDynamicsMetalArgs& args);
gp_float v_emission_rate(device ClusterDynamicsMetalArgs& args);
gp_float i_absorption_rate(device ClusterDynamicsMetalArgs& args);
gp_float v_absorption_rate(device ClusterDynamicsMetalArgs& args);
gp_float annihilation_rate(device ClusterDynamicsMetalArgs& args);
gp_float i_dislocation_annihilation_rate(device ClusterDynamicsMetalArgs& args);
gp_float v_dislocation_annihilation_rate(device ClusterDynamicsMetalArgs& args);
gp_float i_grain_boundary_annihilation_rate(device ClusterDynamicsMetalArgs& args);
gp_float v_grain_boundary_annihilation_rate(device ClusterDynamicsMetalArgs& args);
gp_float ii_emission(device ClusterDynamicsMetalArgs& args, uint);
gp_float vv_emission(device ClusterDynamicsMetalArgs& args, uint);
gp_float ii_absorption(device ClusterDynamicsMetalArgs& args, uint);
gp_float iv_absorption(device ClusterDynamicsMetalArgs& args, uint);
gp_float vi_absorption(device ClusterDynamicsMetalArgs& args, uint);
gp_float vv_absorption(device ClusterDynamicsMetalArgs& args, uint);
gp_float i_bias_factor(device ClusterDynamicsMetalArgs& args, uint);
gp_float v_bias_factor(device ClusterDynamicsMetalArgs& args, uint);
gp_float i_binding_energy(device ClusterDynamicsMetalArgs& args, uint);
gp_float v_binding_energy(device ClusterDynamicsMetalArgs& args, uint);
gp_float dislocation_promotion_probability(device ClusterDynamicsMetalArgs& args, uint);
gp_float cluster_radius(device ClusterDynamicsMetalArgs& args, uint);

gp_float i_diffusion(device ClusterDynamicsMetalArgs& args);
gp_float v_diffusion(device ClusterDynamicsMetalArgs& args);
gp_float ii_sum_absorption(device ClusterDynamicsMetalArgs& args, uint);
gp_float iv_sum_absorption(device ClusterDynamicsMetalArgs& args, uint);
gp_float vv_sum_absorption(device ClusterDynamicsMetalArgs& args, uint);
gp_float vi_sum_absorption(device ClusterDynamicsMetalArgs& args, uint);
gp_float mean_dislocation_cell_radius(device ClusterDynamicsMetalArgs& args);



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

        interstitials_out[index] += i_concentration_derivative(args, index) * args.delta_time;
        vacancies_out[index] += v_concentration_derivative(args, index) * args.delta_time;
    }
}



gp_float i_concentration_derivative(device ClusterDynamicsMetalArgs& args, uint n)
{
    return
        
        i_defect_production(args, n)
        
        + iemission_vabsorption_np1(args, n + 1) * args.interstitials[n + 1]
        
        - iemission_vabsorption_n(args, n) * args.interstitials[n];
        
        + iemission_vabsorption_nm1(args, n - 1) * args.interstitials[n - 1];
}



gp_float v_concentration_derivative(device ClusterDynamicsMetalArgs& args, uint vn)
{
    return
        
        v_defect_production(args, vn)
        
        + vemission_iabsorption_np1(args, vn + 1) * args.vacancies[vn + 1]
        
        - vemission_iabsorption_n(args, vn) * args.vacancies[vn]
        
        + vemission_iabsorption_nm1(args, vn - 1) * args.vacancies[vn - 1];
}



gp_float i1_concentration_derivative(device ClusterDynamicsMetalArgs& args)
{
    return 
        
        i_defect_production(args, 1)
        
        - annihilation_rate(args) * args.interstitials[1] * args.vacancies[1]
        
        - args.interstitials[1] * i_dislocation_annihilation_rate(args)
        
        - args.interstitials[1] * i_grain_boundary_annihilation_rate(args)
        
        - args.interstitials[1] * i_absorption_rate(args)
        
        + i_emission_rate(args);
}



gp_float v1_concentration_derivative(device ClusterDynamicsMetalArgs& args)
{
    return 
        
        v_defect_production(args, 1)
        
        - annihilation_rate(args) * args.interstitials[1] * args.vacancies[1]
        
        - args.vacancies[1] * v_dislocation_annihilation_rate(args)
        
        - args.vacancies[1] * v_grain_boundary_annihilation_rate(args)
        
        - args.vacancies[1] * v_absorption_rate(args)
        
        + v_emission_rate(args);
}



gp_float dislocation_density_derivative(device ClusterDynamicsMetalArgs& args)
{
   gp_float gain = 0.0;
   for (uint n = 1; n < args.concentration_boundary; ++n)
   {
      gain += cluster_radius(args, n) * ii_absorption(args, n) * args.interstitials[n] * dislocation_promotion_probability(args, n);
   }

   gain *= 2. * M_PI / args.material->atomic_volume;

   return 
    
      gain
    
      - args.reactor->dislocation_density_evolution * metal::precise::pow(args.material->burgers_vector, 2.) * metal::precise::pow(args.dislocation_density, 3. / 2.);
}



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

    
    return 0.;
}



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

    
    return 0.;
}



gp_float iemission_vabsorption_np1(device ClusterDynamicsMetalArgs& args, uint np1)
{
    return
        
        iv_absorption(args, np1) *
        
        args.vacancies[1] + 
        
        ii_emission(args, np1);
}



gp_float vemission_iabsorption_np1(device ClusterDynamicsMetalArgs& args, uint np1)
{
    return 
        
        vi_absorption(args, np1) * 
        
        args.interstitials[1] + 
        
        vv_emission(args, np1);
}



gp_float iemission_vabsorption_n(device ClusterDynamicsMetalArgs& args, uint n)
{
    return
        
        iv_absorption(args, n) * args.vacancies[1]
        
        + ii_absorption(args, n) * args.interstitials[1] 
        
        + ii_emission(args, n);
}



gp_float vemission_iabsorption_n(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        
        vi_absorption(args, n) * args.interstitials[1] 
        
        + vv_absorption(args, n) * args.vacancies[1]
        
        + vv_emission(args, n);
}



gp_float iemission_vabsorption_nm1(device ClusterDynamicsMetalArgs& args, uint nm1)
{
    return
        
        ii_absorption(args, nm1) 
        
        * args.interstitials[1]
        
        * (1 - dislocation_promotion_probability(args, nm1 + 1));
}



gp_float vemission_iabsorption_nm1(device ClusterDynamicsMetalArgs& args, uint nm1)
{
    return
        
        vv_absorption(args, nm1) * 
        
        args.vacancies[1];
}



gp_float i_emission_rate(device ClusterDynamicsMetalArgs& args)
{
   gp_float time = 0.;
   for (uint in = 3; in < args.concentration_boundary - 1; ++in)
   {
         time +=
         
         ii_emission(args, in) * args.interstitials[in];
   }

   time +=
      
      4. * ii_emission(args, 2) * args.interstitials[2]
      
      + iv_absorption(args, 2) * args.vacancies[1] * args.interstitials[2];

  return time;
}



gp_float v_emission_rate(device ClusterDynamicsMetalArgs& args)
{
   gp_float time = 0.;
   for (uint vn = 3; vn < args.concentration_boundary - 1; ++vn)
   {
      time += 
         
         vv_emission(args, vn) * args.vacancies[vn];
   }

   time +=
      
      4. * vv_emission(args, 2) * args.vacancies[2]
      
      + vi_absorption(args, 2) * args.interstitials[1] * args.vacancies[2];

   return time;
}



gp_float i_absorption_rate(device ClusterDynamicsMetalArgs& args)
{
   gp_float time = ii_absorption(args, 1) * args.interstitials[1];
   for (uint in = 2; in < args.concentration_boundary - 1; ++in)
   {
      time +=
         
         ii_absorption(args, in) * args.interstitials[in]
         
         + vi_absorption(args, in) * args.vacancies[in];
   }

   return time;
}



gp_float v_absorption_rate(device ClusterDynamicsMetalArgs& args)
{
   gp_float time = vv_absorption(args, 1) * args.vacancies[1];
   for (uint vn = 2; vn < args.concentration_boundary - 1; ++vn)
   {
      time +=
         
         vv_absorption(args, vn) * args.vacancies[vn]
         
         + iv_absorption(args, vn) * args.interstitials[vn];
   }

   return time;
}



gp_float annihilation_rate(device ClusterDynamicsMetalArgs& args)
{
    return 
        
        4. * M_PI * 
        
        (args.i_diffusion_val + args.v_diffusion_val) *
        
        args.material->recombination_radius;
}



gp_float i_dislocation_annihilation_rate(device ClusterDynamicsMetalArgs& args)
{
    return
        
        args.dislocation_density *
        
        args.i_diffusion_val *
        
        args.material->i_dislocation_bias;
}





gp_float v_dislocation_annihilation_rate(device ClusterDynamicsMetalArgs& args)
{
    return
        
        args.dislocation_density *
        
        args.v_diffusion_val *
        
        args.material->v_dislocation_bias;
}


gp_float i_grain_boundary_annihilation_rate(device ClusterDynamicsMetalArgs& args)
{
    return
        
        6. * args.i_diffusion_val *
        metal::precise::sqrt
        (
            
            args.dislocation_density * 
            args.material->i_dislocation_bias
            
            + args.ii_sum_absorption_val
            
            + args.vi_sum_absorption_val
        ) /
        
        args.material->grain_size;
}


gp_float v_grain_boundary_annihilation_rate(device ClusterDynamicsMetalArgs& args)
{
    return
        
        6. * args.v_diffusion_val *
        metal::precise::sqrt
        (
            
            args.dislocation_density *
            args.material->v_dislocation_bias
            
            + args.vv_sum_absorption_val
            
            + args.iv_sum_absorption_val
        ) /
        
        args.material->grain_size;
}



gp_float ii_emission(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        
        2. * M_PI * cluster_radius(args, n) *
        
        i_bias_factor(args, n) *
        
        args.i_diffusion_val / args.material->atomic_volume *
        
        metal::precise::exp
        (
            -i_binding_energy(args, n) /
            (BOLTZMANN_EV_KELVIN * args.reactor->temperature)
        );
}




gp_float ii_absorption(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        
        2. * M_PI * cluster_radius(args, n) *
        
        i_bias_factor(args, n) *
        
        args.i_diffusion_val;
}



gp_float iv_absorption(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        
        2. * M_PI * cluster_radius(args, n) *
        
        v_bias_factor(args, n) *
        
        args.v_diffusion_val;
}



gp_float vv_emission(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        
        2. * M_PI * cluster_radius(args, n) *
        
        v_bias_factor(args, n) *
        
        args.v_diffusion_val *
        
        metal::precise::exp
        (
            -v_binding_energy(args, n) /
            (BOLTZMANN_EV_KELVIN * args.reactor->temperature)
        );
}



gp_float vv_absorption(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        
        2. * M_PI * cluster_radius(args, n) *
        
        v_bias_factor(args, n) *
        
        args.v_diffusion_val;
}



gp_float vi_absorption(device ClusterDynamicsMetalArgs& args, uint n)
{
    return 
        2. * M_PI * cluster_radius(args, n) *
        i_bias_factor(args, n) *
        args.i_diffusion_val;
}



gp_float i_bias_factor(device ClusterDynamicsMetalArgs& args, uint n)
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
            (gp_float)n,
            args.material->i_dislocation_bias_param / 2.
        );
}



gp_float v_bias_factor(device ClusterDynamicsMetalArgs& args, uint n)
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
            (gp_float)n,
            args.material->v_dislocation_bias_param / 2.
        );
}



gp_float i_binding_energy(device ClusterDynamicsMetalArgs& args, uint n)
{
    return
        
        args.material->i_formation +
        
        + (args.material->i_binding - args.material->i_formation) / (metal::precise::pow(2., .8) - 1.) *
        
        (metal::precise::pow((gp_float)n, .8) - metal::precise::pow((gp_float)n - 1., .8));
}



gp_float v_binding_energy(device ClusterDynamicsMetalArgs& args, uint n)
{
    return
        
        args.material->v_formation
        
        + (args.material->v_binding - args.material->v_formation) / (metal::precise::pow(2., .8) - 1) *
        
        (metal::precise::pow((gp_float)n, .8) - metal::precise::pow((gp_float)n - 1., .8));
}



gp_float i_diffusion(device ClusterDynamicsMetalArgs& args)
{
    
    return args.material->i_diffusion_0 * metal::precise::exp(-args.material->i_migration / (BOLTZMANN_EV_KELVIN * args.reactor->temperature));
}



gp_float mean_dislocation_cell_radius(device ClusterDynamicsMetalArgs& args)
{
   gp_float r_0_factor = 0.;
   for (uint i = 1; i < args.concentration_boundary; ++i)
   {
      r_0_factor += cluster_radius(args, i) * args.interstitials[i];
   }

                     
   return 1 / metal::precise::sqrt((2. * M_PI * M_PI / args.material->atomic_volume) * r_0_factor + M_PI * args.dislocation_density);
}



gp_float dislocation_promotion_probability(device ClusterDynamicsMetalArgs& args, uint n)
{
   gp_float dr = cluster_radius(args, n + 1) - cluster_radius(args, n);

   
   return (2. * cluster_radius(args, n) * dr + metal::precise::pow(dr, 2.)) 
   
      / (metal::precise::pow(M_PI * args.mean_dislocation_radius_val / 2., 2) - metal::precise::pow(cluster_radius(args, n), 2.)); 
}



gp_float cluster_radius(device ClusterDynamicsMetalArgs& args, uint n)
{
    return metal::precise::sqrt(metal::precise::sqrt(3.) * metal::precise::pow(args.material->lattice_param, 2.) * (gp_float)n / (4. * M_PI));
}



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