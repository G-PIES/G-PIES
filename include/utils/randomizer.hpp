#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include "nuclear_reactor.hpp"
#include "material.hpp"

class Randomizer
{
public:
   Randomizer()
   {
      srand(std::time(nullptr));
   }

   double randd() const
   {
      return static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / 100));
   }

   void reactor_randomize(NuclearReactor& reactor) const
   {
      reactor.flux = randd();
      reactor.temperature = randd();
      reactor.recombination = randd(); 
      reactor.i_bi = randd();
      reactor.i_tri = randd();
      reactor.i_quad = randd();
      reactor.v_bi = randd();
      reactor.v_tri = randd();
      reactor.v_quad = randd();
      reactor.dislocation_density_evolution = randd();
   }

   void material_randomize(Material& material)
   {
      material.i_migration = randd();
      material.v_migration = randd();
      material.i_diffusion_0 = randd();
      material.v_diffusion_0 = randd();
      material.i_formation = randd();
      material.v_formation = randd();
      material.i_binding = randd();
      material.v_binding = randd();
      material.recombination_radius = randd();
      material.i_loop_bias = randd();
      material.i_dislocation_bias = randd();
      material.i_dislocation_bias_param = randd();
      material.v_loop_bias = randd();
      material.v_dislocation_bias = randd();
      material.v_dislocation_bias_param = randd();
      material.dislocation_density_0 = randd();
      material.grain_size = randd();
      material.lattice_param = randd();
      material.burgers_vector = randd();
      material.atomic_volume = randd();
   }
};

#endif // TEST_UTILS_HPP