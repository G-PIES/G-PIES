#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include "nuclear_reactor.hpp"

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
};

#endif // TEST_UTILS_HPP