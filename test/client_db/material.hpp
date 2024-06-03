#pragma once

#include <vector>

#include "client_db/client_db.hpp"
#include "model/material.hpp"
#include "utils/randomizer.hpp"

class MaterialDescriptor {
 public:
  Randomizer randomizer;

  bool create_entity(ClientDb &db, Material &entity, int *sqlite_result_code);
  bool read_entity(ClientDb &db, const int sqlite_id, Material &entity,
                   int *sqlite_result_code);
  bool read_entities(ClientDb &db, std::vector<Material> &entities,
                     int *sqlite_result_code);
  bool update_entity(ClientDb &db, const Material &entity,
                     int *sqlite_result_code);
  bool delete_entity(ClientDb &db, const Material &entity,
                     int *sqlite_result_code);

  void randomize(Material &entity);

  void assert_equal(Material &first, Material &second,
                    bool ignore_sqlite_id = false);
};
