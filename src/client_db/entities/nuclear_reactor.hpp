#pragma once

#include <sqlite3.h>
#include <string>

#include "entity_descriptor.hpp"
#include "model/nuclear_reactor.hpp"

class NuclearReactorEntity : public EntityDescriptor<NuclearReactor, bool> {
 public:
  std::string get_create_query() override;
  void bind(sqlite3_stmt *stmt, NuclearReactor &entity,
            bool &&is_preset) override;

 protected:
  std::string get_create_error_message(const NuclearReactor &) override;
};
