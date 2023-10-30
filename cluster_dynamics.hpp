#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP

#ifndef CONCENTRATION_BOUNDARY
#define CONCENTRATION_BOUNDARY 10
#endif

#ifndef SIMULATION_TIME
#define SIMULATION_TIME 3
#endif

#ifndef DELTA_TIME
#define DELTA_TIME 1
#endif

#ifndef BOLTZMANN_EV_KELVIN
#define BOLTZMANN_EV_KELVIN 8.6173e-5
#endif


// C. Pokor et al. / Journal of Nuclear Materials 326 (2004), Table 5
struct NuclearReactor
{
    const char* species;

    // neutron flux inside of the nuclear reactor
    double flux; 

    // (Celcius) 
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

    inline double temperature_kelvin()
    {
        return temperature + 273.15;
    }
};


// C. Pokor et al. / Journal of Nuclear Materials 326 (2004), Table 6
struct Material
{
    const char* species;

    // migration energy (eV)
    double i_migration;
    double v_migration;

    // diffusion coefficients (pre-exponential) (cm^2/s)
    double i_diffusion;
    double v_diffusion;

    // formation energy (eV)
    double i_formation;
    double v_formation;

    // binding energy for bi-interstitials and bi-vacancies (eV)
    double i_binding;
    double v_binding;

    // recombination radius between defects (nm)
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

    // (m^-2)
    double dislocation_density_initial;

    // grain size (cm)
    double grain_size;
};


// --------------------------------------------------------------------------------------------
// PROTOTYPES 
double i_defect_production(int);
double v_defect_production(int);
double i_clusters(int in);
double v_clusters(int vn);
double v_clusters(int vn);
double iemission_vabsorption_np1(int np1);
double vemission_iabsorption_np1(int np1);
double iemission_vabsorption_n(int n);
double vemission_iabsorption_n(int n);
double iemission_vabsorption_nm1(int nm1);
double vemission_iabsorption_nm1(int nm1);
double i_clusters1(int in);
double v_clusters1(int vn);
double i_emission_time(int nmax);
double v_emission_time(int nmax);
double i_absorption_time(int nmax);
double v_absorption_time(int nmax);
double annihilation_rate();
double i_dislocation_annihilation_time();
double v_dislocation_annihilation_time();
double i_grain_boundary_annihilation_time(int vn);
double v_grain_boundary_annihilation_time(int vn);
double ii_sum_absorption(int nmax);
double iv_sum_absorption(int nmax);
double vv_sum_absorption(int nmax);
double vi_sum_absorption(int nmax);
double ii_emission(int in);
double ii_absorption(int in);
double iv_absorption(int in);
double vv_emission(int vn);
double vv_absorption(int vn);
double vi_absorption(int vn);
double i_bias_factor(int in);
double v_bias_factor(int vn);
double i_binding_energy(int in);
double v_binding_energy(int vn);
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
// GLOBALS (declared in main.cpp)
extern NuclearReactor OSIRIS;
extern Material SA304;

// Setting an overall global variable to hold the concentration_boundary
// so that we aren't forces to pass the variable to every single function. - Sean H.
extern int concentration_boundary;
extern int simulation_time;
extern int delta_time;

// result arrays
extern uint64_t* interstitials;
extern uint64_t* vacancies;

extern NuclearReactor* reactor;
extern Material* material;
// --------------------------------------------------------------------------------------------


#endif