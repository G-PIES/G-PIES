#pragma once

#include "../../vector3.hpp"
#include "../event.hpp"

class RandomWalk : public Event {
 public:
  virtual void init(Model *model) override;
  virtual double get_rate() override { return _rate; };
  virtual void execute(Model *model, ModelObject *object) override;

 private:
  static Vector3<double> generate_random_vector(double length);
  double _rate;
};
