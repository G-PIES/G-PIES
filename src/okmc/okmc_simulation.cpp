#include "okmc_simulation.hpp"

#include <cstdlib>
#include <ctime>

#include "exporters/null_exporter.hpp"
#include "model/objects/defect.hpp"

int factorial(int n) {
    if (n == 0) return 1;
    return n * factorial(n - 1);
}

OkmcSimulation::OkmcSimulation() {
    exporter = new NullExporter();
    parameters = new OkmcSimulationParameters();
    parameters->omega = 1;
}

void OkmcSimulation::run(Model *model, double simulation_time) {
    // TODO: Init random number generation
    model->init();

    double currentTime = 0.0;

    while (currentTime < simulation_time) {
        double time_increment = calculate_increment(model);

        std::vector<Event *> events = model->events;
        std::vector<ModelObject *> objects = model->objects;

        for (Event *event : events) {
            std::vector<double> probabilities =
                calculate_event_probabilities(event, time_increment);

            for (ModelObject *object : objects) {
                if (object == nullptr) {
                    continue;
                }

                double rnd = random_double();
                int times = calculate_number_of_times(probabilities, rnd);
                for (int i = 0; i < times; i++) {
                    event->execute(model, object);
                }
            }
        }

        process_out_of_box(model);
        process_interactions(model);

        currentTime += time_increment;

        exporter->export_model(model);
    }
}

double OkmcSimulation::random_double() {
    int random_int = rand();
    double random_double = 1.0 * random_int / RAND_MAX;
    return random_double;
}

double OkmcSimulation::calculate_increment(Model *model) {
    std::vector<Event *> events = model->events;
    double max_rate = events[0]->get_rate();
    for (Event *event : events) {
        double rate = event->get_rate();
        max_rate = std::max(max_rate, rate);
    }
    return parameters->omega / max_rate;
}

std::vector<double> OkmcSimulation::calculate_event_probabilities(
    Event *event, double time_increment) {
    std::vector<double> probabilities;
    double sum = 0.0;
    int times = 0;

    while (sum < 0.999999) {
        sum += calculate_event_probability(event, times, time_increment);
        probabilities.push_back(sum);
        times++;
    }

    return probabilities;
}

double OkmcSimulation::calculate_event_probability(Event *event, int times,
                                                   double time_increment) {
    double result = std::pow(event->get_rate() * time_increment, times);
    result /= factorial(times);
    result *= std::exp(-event->get_rate() * time_increment);
    return result;
}

int OkmcSimulation::calculate_number_of_times(std::vector<double> probabilities,
                                              double random_double) {
    unsigned int index = 0;
    for (; index < probabilities.size(); index++) {
        if (probabilities[index] > random_double) {
            return index;
        }
    }

    return index;
}

void OkmcSimulation::process_out_of_box(Model *model) {
    for (ModelObject *&object : model->objects) {
        Defect *defect = static_cast<Defect *>(object);
        if (defect == nullptr) {
            continue;
        }
        if (!model->is_within_dimensions(defect->position)) {
            object = nullptr;
        }
    }
}

void OkmcSimulation::process_interactions(Model *model) {
    double random_walk_distance = model->parameters->random_walk_distance;

    unsigned int object_count = model->objects.size();

    for (unsigned int i = 0; i < object_count; i++) {
        Defect *first = static_cast<Defect *>(model->objects[i]);
        if (first == nullptr) {
            continue;
        }
        for (unsigned int j = i + 1; j < object_count; j++) {
            Defect *second = static_cast<Defect *>(model->objects[j]);
            if (second == nullptr) {
                continue;
            }

            if (first->position.distance(second->position) <
                random_walk_distance) {
                model->objects[j] = nullptr;
                if (first->type == second->type) {
                    first->size++;
                } else {
                    first->size--;
                    if (first->size == 0) {
                        model->objects[i] = nullptr;
                    }
                }
            }
        }
    }
}
