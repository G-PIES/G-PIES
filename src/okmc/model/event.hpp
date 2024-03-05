#pragma once

class Model;
class ModelObject;

class Event {
 public:
  virtual void init(Model *model) = 0;
  virtual double get_rate() = 0;
  virtual void execute(Model *model, ModelObject *object) = 0;
};
