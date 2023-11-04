#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#ifndef CONCENTRATION_BOUNDARY
#define CONCENTRATION_BOUNDARY 11
#endif

#ifndef SIMULATION_TIME
#define SIMULATION_TIME 16 // seconds (s)
#endif

#ifndef DELTA_TIME
#define DELTA_TIME 1 // seconds (s)
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

// used in formatted debug printing
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

    // recombination in the cascades
    double recombination; 

    // interstitials in the cascades
    double i_bi;
    double i_tri;
    double i_quad;

    // vacancies in the cascades
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
double i_defect_production(int);
double v_defect_production(int);
double i_clusters(uint64_t in);
double v_clusters(uint64_t vn);
double v_clusters(uint64_t vn);
double iemission_vabsorption_np1(uint64_t np1);
double vemission_iabsorption_np1(uint64_t np1);
double iemission_vabsorption_n(uint64_t n);
double vemission_iabsorption_n(uint64_t n);
double iemission_vabsorption_nm1(uint64_t nm1);
double vemission_iabsorption_nm1(uint64_t nm1);
double i_clusters1(uint64_t in);
double v_clusters1(uint64_t vn);
double i_emission_time(uint64_t nmax);
double v_emission_time(uint64_t nmax);
double i_absorption_time(uint64_t nmax);
double v_absorption_time(uint64_t nmax);
double annihilation_rate();
double i_dislocation_annihilation_time();
double v_dislocation_annihilation_time();
double i_grain_boundary_annihilation_time(uint64_t vn);
double v_grain_boundary_annihilation_time(uint64_t vn);
double ii_sum_absorption(uint64_t nmax);
double iv_sum_absorption(uint64_t nmax);
double vv_sum_absorption(uint64_t nmax);
double vi_sum_absorption(uint64_t nmax);
double ii_emission(uint64_t in);
double ii_absorption(uint64_t in);
double iv_absorption(uint64_t in);
double vv_emission(uint64_t vn);
double vv_absorption(uint64_t vn);
double vi_absorption(uint64_t vn);
double i_bias_factor(uint64_t in);
double v_bias_factor(uint64_t vn);
double i_binding_energy(uint64_t in);
double v_binding_energy(uint64_t vn);
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
// GLOBALS (declared in main.cpp)
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

extern NuclearReactor* reactor;
extern Material* material;
// --------------------------------------------------------------------------------------------


#endif