#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#include "nuclear_reactor.hpp"
#include "material.hpp"

#ifndef CONCENTRATION_BOUNDARY
#define CONCENTRATION_BOUNDARY 10
#endif

#ifndef SIMULATION_TIME
#define SIMULATION_TIME 60 * 60 * 24 * 365 // seconds (s)
#endif

#ifndef DELTA_TIME
#define DELTA_TIME 60 * 60 * 24 // seconds (s)
#endif

#ifndef BOLTZMANN_EV_KELVIN
#define BOLTZMANN_EV_KELVIN 8.6173e-5 // (eV / Kelvin)
#endif

// verbose printing
#ifndef VPRINT
#define VPRINT false 
#endif

// verbose breakpoints
#ifndef VBREAK
#define VBREAK false 
#endif

// csv output
#ifndef CSV
#define CSV false
#endif

// used formatted debug printing
#define TABS "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"


// --------------------------------------------------------------------------------------------
// PROTOTYPES 
bool validate(uint64_t, double);
double i_defect_production(uint64_t);
double v_defect_production(uint64_t);
double i_clusters_delta(uint64_t);
double v_clusters_delta(uint64_t);
double iemission_vabsorption_np1(uint64_t);
double vemission_iabsorption_np1(uint64_t);
double iemission_vabsorption_n(uint64_t);
double vemission_iabsorption_n(uint64_t);
double iemission_vabsorption_nm1(uint64_t);
double vemission_iabsorption_nm1(uint64_t);
double i1_cluster_delta(uint64_t);
double v1_cluster_delta(uint64_t);
double i_emission_time(uint64_t);
double v_emission_time(uint64_t);
double i_absorption_time(uint64_t);
double v_absorption_time(uint64_t);
double annihilation_rate();
double i_dislocation_annihilation_time();
double v_dislocation_annihilation_time();
double i_grain_boundary_annihilation_time(uint64_t);
double v_grain_boundary_annihilation_time(uint64_t);
double ii_sum_absorption(uint64_t);
double iv_sum_absorption(uint64_t);
double vv_sum_absorption(uint64_t);
double vi_sum_absorption(uint64_t);
double ii_emission(uint64_t);
double ii_absorption(uint64_t);
double iv_absorption(uint64_t);
double vv_emission(uint64_t);
double vv_absorption(uint64_t);
double vi_absorption(uint64_t);
double i_bias_factor(uint64_t);
double v_bias_factor(uint64_t);
double i_binding_energy(uint64_t);
double v_binding_energy(uint64_t);
double i_diffusion();
double v_diffusion();
double dislocation_density_delta();
double cluster_radius(uint64_t);
// --------------------------------------------------------------------------------------------

inline double print_return(double result)
{
    fprintf(stdout, "%.*s= %8.15g", 15, TABS, result);
    fprintf(stdout, "\n\n");
    #if VBREAK
    fgetc(stdin);
    #endif
    return result;
}


// --------------------------------------------------------------------------------------------
// GLOBALS (declared in main.cpp)

// Setting an overall global variable to hold the concentration_boundary
// so that we aren't forces to pass the variable to every single function. - Sean H.
extern uint64_t concentration_boundary;
extern double simulation_time;
extern double delta_time;

// result arrays
extern std::array<double, CONCENTRATION_BOUNDARY> interstitials;
extern std::array<double, CONCENTRATION_BOUNDARY> vacancies;

extern double dislocation_density;

extern NuclearReactor reactor;
extern Material material;
// --------------------------------------------------------------------------------------------




#endif
