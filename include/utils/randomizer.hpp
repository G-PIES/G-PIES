#ifndef RANDOMIZER_HPP
#define RANDOMIZER_HPP

#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "model/simulation_model.hpp"


class Randomizer {
 public:
    Randomizer() { srand(std::time(nullptr)); }

    gp_float randd() const {
        return static_cast<gp_float>(rand()) /
               (static_cast<gp_float>(RAND_MAX / 100));
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

   void simulation_randomize(SimulationModel& simulation)
   {
      simulation.id_reactor = rand() % 10;
      simulation.id_material = rand() % 10;
      simulation.cd_state.time = randd();

      int vec_size = rand() % 100000 + 4;
      for (int i = 0; i < vec_size; ++i) 
      {
         simulation.cd_state.interstitials.push_back(randd());
         simulation.cd_state.vacancies.push_back(randd());
      }

      simulation.cd_state.dislocation_density = randd();
   }
};

#endif  // RANDOMIZER_HPP
