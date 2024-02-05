#pragma once

#include <vector>

#include "model/model.hpp"
#include "model/event.hpp"
#include "exporter.hpp"

class OkmcSimulationParameters {
public:
    double omega;
};

class OkmcSimulation {

public:
    OkmcSimulation();

    static double random_double();
    void run(Model *model, double simulation_time);

    OkmcSimulationParameters *parameters;
    Exporter *exporter;

private:
    double calculate_increment(Model *model);
    std::vector<double> calculate_event_probabilities(Event *event, double time_increment);
    static double calculate_event_probability(Event *event, int times, double time_increment);
    static int calculate_number_of_times(std::vector<double> probabilities, double random_double);
    void process_out_of_box(Model *model);
    void process_interactions(Model *model);
};
