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
  std::string get_update_one_query() override;
  std::string get_delete_one_query() override;

  void bind_create_one(sqlite3_stmt *, const NuclearReactor &,
                       bool &&) override;
  void bind_update_one(sqlite3_stmt *, const NuclearReactor &) override;

  void read_row(sqlite3_stmt *, NuclearReactor &) override;
  void read_row(sqlite3_stmt *, NuclearReactor &, const int);

  std::string get_entity_name() override;
  std::string get_entities_name() override;
  std::string get_entity_description(const NuclearReactor &object) override;

 private:
  void bind_base(sqlite3_stmt *, const NuclearReactor &);
};
