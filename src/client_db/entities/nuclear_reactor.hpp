#pragma once

#include <sqlite3.h>
#include <string>

#include "entity_descriptor.hpp"
#include "model/nuclear_reactor.hpp"

class NuclearReactorEntity : public EntityDescriptor<NuclearReactor, bool> {
 public:
  std::string get_create_one_query() override;
  std::string get_read_one_query() override;
  std::string get_read_all_query() override;

  void bind_create_one(sqlite3_stmt *, NuclearReactor &, bool &&) override;

  void read_row(sqlite3_stmt *, NuclearReactor &) override;

 protected:
  std::string get_create_one_error_message(const NuclearReactor &) override;
  std::string get_read_one_error_message(const int) override;
  std::string get_read_all_error_message() override;
};
