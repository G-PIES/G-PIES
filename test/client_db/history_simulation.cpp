#include "model/history_simulation.hpp"

#include <gtest/gtest.h>

#include "client_db/client_db.hpp"
#include "entity_test.hpp"
#include "utils/randomizer.hpp"

class MaterialDescriptor {
 public:
  void assert_equal(Material &first, Material &second, bool ignore_sqlite_id);
};

class NuclearReactorDescriptor {
 public:
  void assert_equal(NuclearReactor &first, NuclearReactor &second,
                    bool ignore_sqlite_id);
};

class HistorySimulationDescriptor {
 public:
  Randomizer randomizer;
  MaterialDescriptor material_descriptor;
  NuclearReactorDescriptor nuclear_reactor_descriptor;

  bool create_entity(ClientDb &db, HistorySimulation &entity,
                     int *sqlite_result_code) {
    return db.create_simulation(entity, sqlite_result_code);
  }

  bool read_entity(ClientDb &db, const int sqlite_id, HistorySimulation &entity,
                   int *sqlite_result_code) {
    return db.read_simulation(sqlite_id, entity, sqlite_result_code);
  }

  bool read_entities(ClientDb &db, std::vector<HistorySimulation> &entities,
                     int *sqlite_result_code) {
    return db.read_simulations(entities, sqlite_result_code);
  }

  bool delete_entity(ClientDb &db, const HistorySimulation &entity,
                     int *sqlite_result_code) {
    return db.delete_simulation(entity, sqlite_result_code);
  }

  void randomize(HistorySimulation &entity) {
    randomizer.simulation_randomize(entity);
  }

  void assert_equal(HistorySimulation &first, HistorySimulation &second,
                    bool ignore_sqlite_id) {
    if (!ignore_sqlite_id) {
      ASSERT_EQ(first.sqlite_id, second.sqlite_id);
    }
    material_descriptor.assert_equal(first.material, second.material,
                                     ignore_sqlite_id);
    nuclear_reactor_descriptor.assert_equal(first.reactor, second.reactor,
                                            ignore_sqlite_id);
  }
};

ENTITY_TEST(HistorySimulation, CreateAndRead_Success)
ENTITY_TEST(HistorySimulation, InvalidRead_DoesNotChangeInputObject)
ENTITY_TEST(HistorySimulation, DeleteUnknownId_Success)
ENTITY_TEST(HistorySimulation, Delete_Success)
ENTITY_TEST(HistorySimulation, DeleteInvalidId_Exception)
ENTITY_TEST(HistorySimulation, CreateAndReadMany_Success)
ENTITY_TEST(HistorySimulation, DeleteMany_Success)
