#include "nuclear_reactor.hpp"

#include <gtest/gtest.h>
#include <vector>

#include "client_db/client_db.hpp"
#include "entity_test.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/randomizer.hpp"

bool NuclearReactorDescriptor::create_entity(ClientDb &db,
                                             NuclearReactor &entity,
                                             int *sqlite_result_code) {
  return db.create_reactor(entity, sqlite_result_code);
}

bool NuclearReactorDescriptor::read_entity(ClientDb &db, const int sqlite_id,
                                           NuclearReactor &entity,
                                           int *sqlite_result_code) {
  return db.read_reactor(sqlite_id, entity, sqlite_result_code);
}

bool NuclearReactorDescriptor::read_entities(
    ClientDb &db, std::vector<NuclearReactor> &entities,
    int *sqlite_result_code) {
  return db.read_reactors(entities, sqlite_result_code);
}

bool NuclearReactorDescriptor::update_entity(ClientDb &db,
                                             const NuclearReactor &entity,
                                             int *sqlite_result_code) {
  return db.update_reactor(entity, sqlite_result_code);
}

bool NuclearReactorDescriptor::delete_entity(ClientDb &db,
                                             const NuclearReactor &entity,
                                             int *sqlite_result_code) {
  return db.delete_reactor(entity, sqlite_result_code);
}

void NuclearReactorDescriptor::randomize(NuclearReactor &entity) {
  randomizer.reactor_randomize(entity);
}

void NuclearReactorDescriptor::assert_equal(NuclearReactor &first,
                                            NuclearReactor &second,
                                            bool ignore_sqlite_id) {
  if (!ignore_sqlite_id) {
    ASSERT_EQ(first.sqlite_id, second.sqlite_id);
  }
  ASSERT_EQ(first.creation_datetime, second.creation_datetime);
  ASSERT_EQ(first.species, second.species);
  ASSERT_EQ(first.get_flux(), second.get_flux());
  ASSERT_EQ(first.get_temperature(), second.get_temperature());
  ASSERT_EQ(first.get_recombination(), second.get_recombination());
  ASSERT_EQ(first.get_i_bi(), second.get_i_bi());
  ASSERT_EQ(first.get_i_tri(), second.get_i_tri());
  ASSERT_EQ(first.get_i_quad(), second.get_i_quad());
  ASSERT_EQ(first.get_v_bi(), second.get_v_bi());
  ASSERT_EQ(first.get_v_tri(), second.get_v_tri());
  ASSERT_EQ(first.get_v_quad(), second.get_v_quad());
  ASSERT_EQ(first.get_dislocation_density_evolution(),
            second.get_dislocation_density_evolution());
}

ALL_ENTITY_TESTS(NuclearReactor)
