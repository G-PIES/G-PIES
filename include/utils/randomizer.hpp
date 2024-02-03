#ifndef RANDOMIZER_HPP
#define RANDOMIZER_HPP

#include "nuclear_reactor.hpp"
#include "material.hpp"

class Randomizer
{
public:
   Randomizer()
   {
      srand(std::time(nullptr));
   }

   gp_float randd() const
   {
      return static_cast<gp_float>(rand()) / (static_cast<gp_float>(RAND_MAX / 100));
   }

   void reactor_randomize(NuclearReactor& reactor) const
   {
      reactor.set_flux(randd());
      reactor.set_temperature(randd());
      reactor.set_recombination(randd()) ;
      reactor.set_i_bi(randd());
      reactor.set_i_tri(randd());
      reactor.set_i_quad(randd());
      reactor.set_v_bi(randd());
      reactor.set_v_tri(randd());
      reactor.set_v_quad(randd());
      reactor.set_dislocation_density_evolution(randd());
   }

   void material_randomize(Material& material)
   {
      material.set_i_migration(randd());
      material.set_v_migration(randd());
      material.set_i_diffusion_0(randd());
      material.set_v_diffusion_0(randd());
      material.set_i_formation(randd());
      material.set_v_formation(randd());
      material.set_i_binding(randd());
      material.set_v_binding(randd());
      material.set_recombination_radius(randd());
      material.set_i_loop_bias(randd());
      material.set_i_dislocation_bias(randd());
      material.set_i_dislocation_bias_param(randd());
      material.set_v_loop_bias(randd());
      material.set_v_dislocation_bias(randd());
      material.set_v_dislocation_bias_param(randd());
      material.set_dislocation_density_0(randd());
      material.set_grain_size(randd());
      material.set_lattice_param(randd());
      material.set_burgers_vector(randd());
      material.set_atomic_volume(randd());
   }
};

#endif // RANDOMIZER_HPP