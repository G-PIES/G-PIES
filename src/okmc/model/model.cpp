#include "model.hpp"
#include "event.hpp"

Model::Model() {
    parameters = new ModelParameters();
}


void Model::init() {
    for (Event *event: events) {
        event->init(this);
    }
}

bool Model::is_within_dimensions(Vector3<double> vector) {
    return vector.x >= parameters->x_from && vector.x <= parameters->x_to &&
           vector.y >= parameters->y_from && vector.y <= parameters->y_to &&
           vector.z >= parameters->z_from && vector.z <= parameters->z_to;
}
