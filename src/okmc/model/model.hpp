#pragma once

#include "vector"
#include "model_object.hpp"
#include "event.hpp"
#include "../vector3.hpp"

class ModelParameters {
public:
    double attempt_frequency;
    double migration_energy;
    double temperature;
    double random_walk_distance;

    double x_from, x_to;
    double y_from, y_to;
    double z_from, z_to;
};

class Model {
public:
    Model();
    void init();

    bool is_within_dimensions(Vector3<double> vector);

    ModelParameters *parameters;
    std::vector<Event *> events;
    std::vector<ModelObject *> objects;
};
