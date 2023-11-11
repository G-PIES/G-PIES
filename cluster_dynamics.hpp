#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#ifndef CONCENTRATION_BOUNDARY
#define CONCENTRATION_BOUNDARY 5
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

#ifndef DISLOCATION_DENSITY_EVOLUTION
#define DISLOCATION_DENSITY_EVOLUTION 300
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

// Celcius to Kevlin Conversion
const double CELCIUS_KELVIN_CONV = 273.15;

// Second to femtosecond conversion
const double SEC_FS_CONV = 1e15;

// Meter to centimeter conversion
const double M_CM_CONV = 1e4;

// Picometer to centimeter conversion
const double PM_CM_CONV = 1e10;


// C. Pokor et al. / Journal of Nuclear Materials 326 (2004), Table 5
struct NuclearReactor
{
    const char* species;

    // neutron flux inside of the nuclear reactor (cm^2 / s)
    double flux; 

    // (Kelvin) 
    double temperature;

    // recombination the cascades
    double recombination; 

    // interstitials the cascades
    double i_bi;
    double i_tri;
    double i_quad;

    // vacancies the cascades
    double v_bi;
    double v_tri;
    double v_quad;
};


// C. Pokor et al. / Journal of Nuclear Materials 326 (2004), Table 6
struct Material
{
    const char* species;

    // migration energy (eV)
    double i_migration;
    double v_migration;

    // diffusion coefficients (pre-exponential) (cm^2 / s)
    double i_diffusion;
    double v_diffusion;

    // formation energy (eV)
    double i_formation;
    double v_formation;

    // binding energy for bi-interstitials and bi-vacancies (eV)
    double i_binding;
    double v_binding;

    // recombination radius between defects (cm)
    double recombination_radius;

    // bias factor of the loops for interstitials
    double i_loop_bias;

    // bias factor of dislocations for interstitial
    double i_dislocation_bias;
    double i_dislocation_bias_param;

    // bias factor of the loops for vacancies
    double v_loop_bias;

    // bias factor of dislocations for vacancies 
    double v_dislocation_bias;
    double v_dislocation_bias_param;

    // (cm^2)
    double dislocation_density_initial;

    // grain size (cm)
    double grain_size;
};


// --------------------------------------------------------------------------------------------
// PROTOTYPES 
bool validate(uint64_t);
double i_defect_production(uint64_t);
double v_defect_production(uint64_t);
double i_clusters(uint64_t);
double v_clusters(uint64_t);
double iemission_vabsorption_np1(uint64_t);
double vemission_iabsorption_np1(uint64_t);
double iemission_vabsorption_n(uint64_t);
double vemission_iabsorption_n(uint64_t);
double iemission_vabsorption_nm1(uint64_t);
double vemission_iabsorption_nm1(uint64_t);
double i_clusters1(uint64_t);
double v_clusters1(uint64_t);
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
double dislocation_density_delta();
double burgers_vector(double);
// --------------------------------------------------------------------------------------------

inline double print_return(double result)
{
    fprintf(stdout, "%.*s= %8.15lf", 15, TABS, result);
    fprintf(stdout, "\n\n");
    #if VBREAK
    fgetc(stdin);
    #endif
    return result;
}


// --------------------------------------------------------------------------------------------
// GLOBALS (declared main.cpp)
extern NuclearReactor OSIRIS;
extern Material SA304;

// Setting an overall global variable to hold the concentration_boundary
// so that we aren't forces to pass the variable to every single function. - Sean H.
extern uint64_t concentration_boundary;
extern uint64_t simulation_time;
extern uint64_t delta_time;

// result arrays
extern std::array<double, CONCENTRATION_BOUNDARY> interstitials;
extern std::array<double, CONCENTRATION_BOUNDARY> vacancies;
extern double dislocation_density;

extern NuclearReactor* reactor;
extern Material* material;
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
namespace lattice_parameters
{
    // Chromium: 291 pm
    const double chromium = 291.0 * PM_CM_CONV;
    // Nickel: 352.4 pm
    const double nickel = 352.4 * PM_CM_CONV;
    // Carbon: 246.4 pm
    const double carbon = 246.4 * PM_CM_CONV;

    const double sa304 = 
                    chromium * .18 +
                    nickel * .08 +
                    carbon * .74;
}
// --------------------------------------------------------------------------------------------


#endif