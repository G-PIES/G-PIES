#include <iostream>
#include <cstring>
#include <cmath>
#include <array>

#include "cluster_dynamics.hpp"
#include "material.hpp"
#include "nuclear_reactor.hpp"

#include "TGraph.h"
#include "TCanvas.h"
#include "TGButton.h"
#include "TGClient.h"
#include "TMultiGraph.h"
#include "TApplication.h"
#include "TRootCanvas.h"
#include "TF1.h"
#include "TPaveText.h"
#include "TSystem.h"
#include "TGaxis.h"

#ifndef VPRINT
#define VPRINT false
#endif

#ifndef VBREAK
#define VBREAK false
#endif

#ifndef CSV
#define CSV false
#endif

double concentration_boundary;
double simulation_time;
double delta_time;
double sample_interval; // How often (in seconds) to record the state

void print_start_message()
{
    fprintf(stderr, "\nSimulation Started: ");
    fprintf(stderr, "delta_time: %g, ", delta_time);
    fprintf(stderr, "simulation_time: %g, ", simulation_time);
    fprintf(stderr, "concentration_boundary: %llu\n", (unsigned long long)concentration_boundary);
}

void print_state(ClusterDynamicsState& state)
{
    if (!state.valid) fprintf(stdout, "\nINVALID SIM @ Time=%g", state.time);
    else fprintf(stdout, "\nTime=%g", state.time);

    if (state.interstitials.size() != concentration_boundary || state.vacancies.size() != concentration_boundary)
    {
        fprintf(stderr, "\nError: Output data is incorrect size.\n");
        return;
    }

    fprintf(stdout, "\nCluster Size\t\t-\t\tInterstitials\t\t-\t\tVacancies\n\n");
    for (uint64_t n = 1; n < concentration_boundary; ++n)
    {
        fprintf(stdout, "%llu\t\t\t\t\t%13g\t\t\t  %15g\n\n", (unsigned long long)n, state.interstitials[n], state.vacancies[n]);
    }
}

void print_csv(ClusterDynamicsState& state)
{
    for (uint64_t n = 1; n < concentration_boundary; ++n)
    {
        fprintf(stdout, "%g,%llu,%g,%g\n", state.time, (unsigned long long) n, state.interstitials[n], state.vacancies[n]);
    }
}

int main(int argc, char* argv[])
{
    NuclearReactor reactor = nuclear_reactors::OSIRIS();
    Material material = materials::SA304();

    // Defaults
    concentration_boundary = 10;
    simulation_time = 1.0;
    delta_time = 1e-5;
    sample_interval = delta_time;

    switch (argc)
    {
        case 4:
            concentration_boundary = strtod(argv[3], NULL);
        case 3:
            simulation_time = strtod(argv[2], NULL);
        case 2:
            delta_time = strtod(argv[1], NULL);
        default:
            break;
    }

    double graph_index = concentration_boundary / simulation_time;

    ClusterDynamics cd(concentration_boundary, reactor, material);

    print_start_message();

    #if CSV
    fprintf(stdout, "Time (s),Cluster Size,Interstitials / cm^3,Vacancies / cm^3\n");
    #endif

    ClusterDynamicsState state;

    // Initialize the canvas aka the window
    TApplication app("ClD", &argc, argv); 
    TCanvas* c = new TCanvas("c", "Cluster Dynamics Simulation Test", 0, 0, 800, 600);

    // Canvas configuration
    
    TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();

    rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");

    TGraph* i_graph = new TGraph(concentration_boundary);
    TGraph* v_graph = new TGraph(concentration_boundary);
    
    std::vector<TGraph*> i_graphs;
    std::vector<TGraph*> v_graphs;
    
    for (int i = 0; i < concentration_boundary; ++i){
        i_graphs.push_back(new TGraph(concentration_boundary));
        v_graphs.push_back(new TGraph(concentration_boundary));
        i_graphs.back()->SetLineColor(kRed);
        v_graphs.back()->SetLineColor(kBlue);
    }

    TMultiGraph* mg = new TMultiGraph();

    for (int i = 0; i < concentration_boundary; ++i){
        mg->Add(i_graphs[i], "alp");
        mg->Add(v_graphs[i], "lp"); 
    }
    
    mg->Draw();
    
    // --------------------------------------------------------------------------------------------
    // main simulation loop
    double loop_index = 0.;
    for (double t = 0; t < simulation_time; t += delta_time)
    {
        // run simulation for this time slice
        state = cd.run(delta_time, sample_interval);

        print_state(state);

        if (!state.valid) 
        {
            break;
        }
        
        loop_index = t*graph_index;
        for (int n = 0; n < concentration_boundary; ++n)
        {
            i_graphs.at(n)->AddPoint((loop_index), state.interstitials[n]);
            v_graphs.at(n)->AddPoint((loop_index), state.vacancies[n]);
        }
        
        fprintf(stdout, "\n = %f\n", (loop_index));

        c->Modified(); c->Update();
        gSystem->ProcessEvents();
    }
    // -------------------------------------------------------------------------------------------
    // App run

    app.Run(true);

    delete(i_graph);
    delete(v_graph);
    delete(mg);
    delete(c);

    fprintf(stderr, "Exiting..\n");

    return 0;
}