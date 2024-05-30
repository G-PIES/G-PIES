#pragma once

#include <vector>

#include "client_db/client_db.hpp"
#include "material.hpp"
#include "model/history_simulation.hpp"
#include "nuclear_reactor.hpp"
#include "utils/randomizer.hpp"

class HistorySimulationDescriptor {
 public:
  Randomizer randomizer;
  MaterialDescriptor material_descriptor;
  NuclearReactorDescriptor nuclear_reactor_descriptor;

  bool create_entity(ClientDb &db, HistorySimulation &entity,
                     int *sqlite_result_code);
  bool read_entity(ClientDb &db, const int sqlite_id, HistorySimulation &entity,
                   int *sqlite_result_code);
  bool read_entities(ClientDb &db, std::vector<HistorySimulation> &entities,
                     int *sqlite_result_code);
  bool delete_entity(ClientDb &db, const HistorySimulation &entity,
                     int *sqlite_result_code);

  void randomize(HistorySimulation &entity);

  void assert_equal(HistorySimulation &first, HistorySimulation &second,
                    bool ignore_sqlite_id);
};
