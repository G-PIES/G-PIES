#include "model/material.hpp"

#include <gtest/gtest.h>

#include "client_db/client_db.hpp"
#include "entity_test.hpp"
#include "utils/randomizer.hpp"

class MaterialDescriptor {
 public:
  Randomizer randomizer;

  bool create_entity(ClientDb &db, Material &entity, int *sqlite_result_code) {
    return db.create_material(entity, sqlite_result_code);
  }

  bool read_entity(ClientDb &db, const int sqlite_id, Material &entity,
                   int *sqlite_result_code) {
    return db.read_material(sqlite_id, entity, sqlite_result_code);
  }

  bool read_entities(ClientDb &db, std::vector<Material> &entities,
                     int *sqlite_result_code) {
    return db.read_materials(entities, sqlite_result_code);
  }

  bool update_entity(ClientDb &db, const Material &entity,
                     int *sqlite_result_code) {
    return db.update_material(entity, sqlite_result_code);
  }

  bool delete_entity(ClientDb &db, const Material &entity,
                     int *sqlite_result_code) {
    return db.delete_material(entity, sqlite_result_code);
  }

  void randomize(Material &entity) { randomizer.material_randomize(entity); }

  void assert_equal(Material &first, Material &second, bool ignore_sqlite_id) {
    if (!ignore_sqlite_id) {
      ASSERT_EQ(first.sqlite_id, second.sqlite_id);
    }
    ASSERT_EQ(first.creation_datetime, second.creation_datetime);
    ASSERT_EQ(first.species, second.species);
    ASSERT_EQ(first.get_i_migration(), second.get_i_migration());
    ASSERT_EQ(first.get_v_migration(), second.get_v_migration());
    ASSERT_EQ(first.get_i_diffusion_0(), second.get_i_diffusion_0());
    ASSERT_EQ(first.get_v_diffusion_0(), second.get_v_diffusion_0());
    ASSERT_EQ(first.get_i_formation(), second.get_i_formation());
    ASSERT_EQ(first.get_v_formation(), second.get_v_formation());
    ASSERT_EQ(first.get_i_binding(), second.get_i_binding());
    ASSERT_EQ(first.get_v_binding(), second.get_v_binding());
    ASSERT_EQ(first.get_recombination_radius(),
              second.get_recombination_radius());
    ASSERT_EQ(first.get_i_loop_bias(), second.get_i_loop_bias());
    ASSERT_EQ(first.get_i_dislocation_bias(), second.get_i_dislocation_bias());
    ASSERT_EQ(first.get_i_dislocation_bias_param(),
              second.get_i_dislocation_bias_param());
    ASSERT_EQ(first.get_v_loop_bias(), second.get_v_loop_bias());
    ASSERT_EQ(first.get_v_dislocation_bias(), second.get_v_dislocation_bias());
    ASSERT_EQ(first.get_v_dislocation_bias_param(),
              second.get_v_dislocation_bias_param());
    ASSERT_EQ(first.get_dislocation_density_0(),
              second.get_dislocation_density_0());
    ASSERT_EQ(first.get_grain_size(), second.get_grain_size());
    ASSERT_EQ(first.get_lattice_param(), second.get_lattice_param());
    ASSERT_EQ(first.get_burgers_vector(), second.get_burgers_vector());
    ASSERT_EQ(first.get_atomic_volume(), second.get_atomic_volume());
  }
};

ALL_ENTITY_TESTS(Material)
