#pragma once

#include <vector>

#include "client_db/client_db.hpp"
#include "model/nuclear_reactor.hpp"
#include "utils/randomizer.hpp"

class NuclearReactorDescriptor {
 public:
  Randomizer randomizer;

  bool create_entity(ClientDb &db, NuclearReactor &entity,
                     int *sqlite_result_code);
  bool read_entity(ClientDb &db, const int sqlite_id, NuclearReactor &entity,
                   int *sqlite_result_code);
  bool read_entities(ClientDb &db, std::vector<NuclearReactor> &entities,
                     int *sqlite_result_code);
  bool update_entity(ClientDb &db, const NuclearReactor &entity,
                     int *sqlite_result_code);
  bool delete_entity(ClientDb &db, const NuclearReactor &entity,
                     int *sqlite_result_code);

  void randomize(NuclearReactor &entity);

  void assert_equal(NuclearReactor &first, NuclearReactor &second,
                    bool ignore_sqlite_id = false);
};
