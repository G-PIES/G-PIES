#ifndef CLUSTER_DYNAMICS_HPP
#define CLUSTER_DYNAMICS_HPP


// (Pokor et al. 2004, Table 5)
struct NuclearReactor
{
    const char* species;

    // neutron flux inside of the nuclear reactor
    double flux; 

    // (C) 
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

    // defect production of interstitials
    inline double i_defect_production(int cluster_size)
    {
        switch (cluster_size)
        {
            case 1: return recombination * flux * (1 - i_bi - i_tri - i_quad);
            case 2: return recombination * flux * i_bi;
            case 3: return recombination * flux * i_tri;
            case 4: return recombination * flux * i_quad;

            default: break;
        }

        // cluster sizes > greater than 4 always zero
        return 0.f;
    };

    // defect production of vacancies
    inline double v_defect_production(int cluster_size)
    {
        switch (cluster_size)
        {
            case 1: return recombination * flux * (1 - v_bi - v_tri - v_quad);
            case 2: return recombination * flux * v_bi;
            case 3: return recombination * flux * v_tri;
            case 4: return recombination * flux * v_quad;

            default: break;
        }

        // cluster sizes > greater than 4 always zero
        return 0.f;
    };
};

NuclearReactor OSIRIS = { "OSIRIS", 2.9e-7, 330.f, .3f, .5f, .2f, .06f, .06f, .03f, .02f };

// (Pokor et al. 2004, Table 6)
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

Material SA304 = { "SA304", .45f, 1.35f, 1e-3, .6f, 4.1f, 1.7f, .6f, .5f, .7f, 63.f, .8f, 1.1f, 33, .65f, 1.f, 1e10, 4e10-3 };

// TODO: rates of interstitial emission and vacancy absorption
/* Pokor et al. 2004, 2b
*/
double iemission_vabsorption_np1(int np1)
{
    return 1.f;
}

/* Pokor et al. 2004, 2c
*/
double iemission_vabsorption_n(int n)
{
    return 1.f;
}

/* Pokor et al. 2004, 2d
*/
double iemission_vabsorption_nm1(int nm1)
{
    return 1.f;
}

#endif