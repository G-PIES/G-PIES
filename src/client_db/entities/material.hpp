#pragma once

#include <sqlite3.h>
#include <string>

#include "entity_descriptor.hpp"
#include "model/material.hpp"

class MaterialEntity : public EntityDescriptor<Material, bool> {
 public:
  std::string get_create_one_query() override;
  std::string get_read_one_query() override;
  std::string get_read_all_query() override;
  std::string get_update_one_query() override;
  std::string get_delete_one_query() override;

  void bind_create_one(sqlite3_stmt *, const Material &, bool &&)
    override;
  void bind_update_one(sqlite3_stmt *, const Material &) override;

  void read_row(sqlite3_stmt *, Material &) override;
  void read_row(sqlite3_stmt *, Material &, const int);

  std::string get_entity_name() override;
  std::string get_entities_name() override;
  std::string get_entity_description(const Material &object) override;

 private:
  void bind_base(sqlite3_stmt *, const Material &);
};
