#include <array>
#include <cmath>
#include <cstring>
#include <iostream>

#include "cluster_dynamics/cluster_dynamics.hpp"
#include "model/material.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/timer.hpp"

#ifndef VPRINT
#define VPRINT false
#endif

#ifndef VBREAK
#define VBREAK false
#endif

#ifndef CSV
#define CSV false
#endif

size_t concentration_boundary;
bool sensitivity_analysis_mode = false;
std::string sensitivity_analysis_variable = "";
size_t num_of_simulation_loops = 0;
gp_float delta_sensitivity_analysis = 0;
gp_float simulation_time;
gp_float delta_time;
gp_float sample_interval;  // How often (in seconds) to record the state

void print_start_message() {
    fprintf(stderr, "\nSimulation Started: ");
    fprintf(stderr, "delta_time: %g, ", delta_time);
    fprintf(stderr, "simulation_time: %g, ", simulation_time);
    fprintf(stderr, "concentration_boundary: %4d\n",
            static_cast<int>(concentration_boundary));
}

void print_state(const ClusterDynamicsState &state) {
    if (!state.valid)
        fprintf(stdout, "\nINVALID SIM @ Time=%g", state.time);
    else
        fprintf(stdout, "\nTime=%g", state.time);

    if (state.interstitials.size() != concentration_boundary ||
        state.vacancies.size() != concentration_boundary) {
        fprintf(stderr, "\nError: Output data is incorrect size.\n");
        return;
    }

    fprintf(stdout,
            "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n");
    for (uint64_t n = 1; n < concentration_boundary; ++n) {
        fprintf(stdout, "%llu\t\t\t\t\t%13g\t\t\t  %15g\n\n",
                (long long unsigned int)n, state.interstitials[n],
                state.vacancies[n]);
    }

    fprintf(stderr, "\nDislocation Network Density: %g\n\n",
            state.dislocation_density);
}

void print_csv(const ClusterDynamicsState &state) {
    for (uint64_t n = 1; n < concentration_boundary; ++n) {
        fprintf(stdout, "%g,%llu,%g,%g\n", state.time,
                (long long unsigned int)n, state.interstitials[n],
                state.vacancies[n]);
    }
}

void profile() {
    std::vector<gp_float> times;
    Timer timer;

    ClusterDynamicsState state;

    NuclearReactor reactor;
    nuclear_reactors::OSIRIS(reactor);

    Material material;
    materials::SA304(material);

    ClusterDynamics cd(10, reactor, material);
    cd.run(1e-5, 1e-5);

    for (int n = 100; n < 400000; n += 10000) {
        fprintf(stderr, "N=%d\n", n);
        ClusterDynamics cd(n, reactor, material);

        timer.Start();
        state = cd.run(1e-5, 1e-5);
        gp_float time = timer.Stop();

        fprintf(stdout, "\n%g", time);
    }
}

enum var_code {
    e_noMatch,
    e_iMigration,
    e_vMigration,
    e_iFormation,
    e_vFormation,
    e_iBinding,
    e_vBinding,
    e_dislocationDensity0,
    e_flux,
    e_temperature,
    e_dislocationDensityEvolution
};

var_code hashit(std::string const& varString) {
    if (varString == "i_migration") return e_iMigration;
    if (varString == "v_migration") return e_vMigration;
    if (varString == "i_formation") return e_iFormation;
    if (varString == "v_formation") return e_vFormation;
    if (varString == "i_binding") return e_iBinding;
    if (varString == "v_binding") return e_vBinding;
    if (varString == "dislocation_density_0") return e_dislocationDensity0;
    if (varString == "flux") return e_flux;
    if (varString == "temperature") return e_temperature;
    if (varString == "dislocation_density_evolution")
        return e_dislocationDensityEvolution;
    return e_noMatch;
}

void update_for_sensitivity_analysis(ClusterDynamics &cd,
                                     NuclearReactor &reactor,
                                     Material &material,
                                     const gp_float delta) {
    switch (hashit(sensitivity_analysis_variable)) {
        case e_iMigration:
            material.set_i_migration(material.get_i_migration() + delta);
            cd.set_material(material);
            break;
        case e_vMigration:
            material.set_v_migration(material.get_v_migration() + delta);
            cd.set_material(material);
            break;
        case e_iFormation:
            material.set_i_formation(material.get_i_formation() + delta);
            cd.set_material(material);
            break;
        case e_vFormation:
            material.set_v_formation(material.get_v_formation() + delta);
            cd.set_material(material);
            break;
        case e_iBinding:
            material.set_i_binding(material.get_i_binding() + delta);
            cd.set_material(material);
            break;
        case e_vBinding:
            material.set_v_binding(material.get_v_binding() + delta);
            cd.set_material(material);
            break;
        case e_dislocationDensity0:
            material.set_dislocation_density_0(
                material.get_dislocation_density_0() + delta);
            cd.set_material(material);
            break;
        case e_flux:
            reactor.set_flux(reactor.get_flux() + delta);
            cd.set_reactor(reactor);
            break;
        case e_temperature:
            reactor.set_temperature(reactor.get_temperature() + delta);
            cd.set_reactor(reactor);
            break;
        case e_dislocationDensityEvolution:
            reactor.set_dislocation_density_evolution(
                reactor.get_dislocation_density_evolution() + delta);
            cd.set_reactor(reactor);
            break;
        default:
            break;
    }
}

int main(int argc, char* argv[]) {
    NuclearReactor reactor;
    nuclear_reactors::OSIRIS(reactor);

    Material material;
    materials::SA304(material);

    // Default values
    concentration_boundary = 10;
    simulation_time = 1.;
    delta_time = 1e-5;
    sample_interval = delta_time;

    // Override default values with CLI arguments
    switch (argc) {
        case 8:
            delta_sensitivity_analysis = strtod(argv[7], NULL);
            num_of_simulation_loops = strtod(argv[6], NULL);
            sensitivity_analysis_variable = argv[5];
            sensitivity_analysis_mode = true; // argv[4] should be -s
            // fall through
        case 4:
            concentration_boundary = strtod(argv[3], NULL);
            // fall through
        case 3:
            simulation_time = strtod(argv[2], NULL);
            // fall through
        case 2:
            delta_time = strtod(argv[1], NULL);
        default:
            break;
    }

    if (sensitivity_analysis_mode) {
        // --------------------------------------------------------------------------------------------
        // sensitivity analysis simulation loop
        for (size_t n = 0; n < num_of_simulation_loops; n++) {
            ClusterDynamics cd(concentration_boundary, reactor, material);

            print_start_message();

            #if CSV
            fprintf(stdout,
                "Time (s),Cluster Size," +
                "Interstitials / cm^3,Vacancies / cm^3\n");
            #endif

            ClusterDynamicsState state;
            update_for_sensitivity_analysis(cd,
                                            reactor,
                                            material,
                                            n * delta_sensitivity_analysis);


            for (gp_float t = 0; t < simulation_time; t = state.time) {
                // run simulation for this time slice
                state = cd.run(delta_time, sample_interval);

                #if VPRINT
                    print_state(state);
                #elif CSV
                    print_csv(state);
                #endif

                if (!state.valid) {
                    break;
                }

                #if VBREAK
                fgetc(stdin);
                #endif
            }

            // ----------------------------------------------------------------
            // print results
            #if !VPRINT && !CSV
            print_state(state);
            #endif
            // ----------------------------------------------------------------
        }
        // --------------------------------------------------------------------
    } else {
        ClusterDynamics cd(concentration_boundary, reactor, material);

        print_start_message();

        #if CSV
        fprintf(stdout,
            "Time (s),Cluster Size,Interstitials / cm^3,Vacancies / cm^3\n");
        #endif

        ClusterDynamicsState state;
        // --------------------------------------------------------------------------------------------
        // main simulation loop
        for (gp_float t = 0; t < simulation_time; t = state.time) {
            // run simulation for this time slice
            state = cd.run(delta_time, sample_interval);

            #if VPRINT
                print_state(state);
            #elif CSV
                print_csv(state);
            #endif

            if (!state.valid) {
                break;
            }

            #if VBREAK
            fgetc(stdin);
            #endif
        }
        // --------------------------------------------------------------------------------------------

        // --------------------------------------------------------------------------------------------
        // print results
        #if !VPRINT && !CSV
        print_state(state);
        #endif
        // --------------------------------------------------------------------------------------------
    }

    return 0;
}
