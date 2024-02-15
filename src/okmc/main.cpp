#include <iostream>
#include "okmc_simulation.hpp"
#include "model/events/random_walk.hpp"
#include "model/objects/defect.hpp"
#include "exporters/wolfram_mathematica_exporter.hpp"

void generate_objects(Model *model);

int main()
{
   Model *model = new Model();
   model->events.push_back(new RandomWalk());
   model->parameters->attempt_frequency = 10000000000000;
   model->parameters->migration_energy = 0.34;
   model->parameters->temperature = 130;
   // TODO: 5 * Units.NanoMeters
   // TODO: Should be 5 nm, but 500 nm is used to simplify debugging
   model->parameters->random_walk_distance = 5 * 0.0000001;
   // TODO: 0.1 * Units.MilliMeters
   double size = 0.1 * 0.001;
   model->parameters->x_from = -size / 2;
   model->parameters->x_to = size / 2;
   model->parameters->y_from = -size / 2;
   model->parameters->y_to = size / 2;
   model->parameters->z_from = -size / 2;
   model->parameters->z_to = size / 2;

   generate_objects(model);

   OkmcSimulation simulation = OkmcSimulation();
   simulation.exporter = new WolframMathematicaExporter();
   simulation.run(model, 60 * 60);
   return 0;
}

double random_double_between(double from, double to)
{
   return from + (to - from) * OkmcSimulation::random_double();
}

void generate_objects(Model *model)
{
   const int count = 500;

   for (int i = 0; i < count; i++)
   {
      Vector3<double> position = Vector3<double>();
      position.x = random_double_between(model->parameters->x_from, model->parameters->x_to);
      position.y = random_double_between(model->parameters->y_from, model->parameters->y_to);
      position.z = random_double_between(model->parameters->z_from, model->parameters->z_to);
      Defect *defect = new Defect();
      defect->type = random_double_between(0, 1) < 0.5 ? Interstitial : Vacancy;
      defect->size = 1;
      defect->position = position;
      model->objects.push_back(defect);
   }
}
