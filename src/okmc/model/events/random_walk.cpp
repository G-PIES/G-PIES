#include "random_walk.hpp"

#include <cmath>

#include "../../okmc_simulation.hpp"
#include "../model.hpp"
#include "../objects/defect.hpp"

void RandomWalk::init(Model *model) {
  double v0 = model->parameters->attempt_frequency;
  double Em = model->parameters->migration_energy;
  // TODO: Move Boltzman constant to a shared file
  double kB = 8.617333262 * 0.00001;
  double T = model->parameters->temperature;

  _rate = v0 * std::exp(-Em / (kB * T));
}

void RandomWalk::execute(Model *model, ModelObject *object) {
  Defect *sia = (Defect *)object;

  if (sia->size >= 5) {
    return;
  }

  double lambda = model->parameters->random_walk_distance;

  Vector3<double> random_vector = RandomWalk::generate_random_vector(lambda);
  sia->position.x += random_vector.x;
  sia->position.y += random_vector.y;
  sia->position.z += random_vector.z;
}

Vector3<double> RandomWalk::generate_random_vector(double length) {
  double const two_pi = 2 * M_PI;

  double alpha = two_pi * OkmcSimulation::random_double();
  double beta = two_pi * OkmcSimulation::random_double();
  double cos_alpha = std::cos(alpha);
  double cos_beta = std::cos(beta);
  double sin_alpha = std::sin(alpha);
  double sin_beta = std::sin(beta);

  Vector3<double> vector = Vector3<double>();
  vector.x = length * cos_alpha * cos_beta;
  vector.y = length * sin_alpha * cos_beta;
  vector.z = length * sin_beta;

  return vector;
}
