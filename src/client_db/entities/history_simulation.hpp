#pragma once

#include <sqlite3.h>

#include <string>

#include "entity_descriptor.hpp"
#include "material.hpp"
#include "model/history_simulation.hpp"
#include "nuclear_reactor.hpp"

class HistorySimulationEntity : public EntityDescriptor<HistorySimulation> {
 public:
  HistorySimulationEntity() : _material_entity(), _nuclear_reactor_entity() {}

  std::string get_create_one_query() override;
  std::string get_read_one_query() override;
  std::string get_read_all_query() override;
  std::string get_update_one_query() override;
  std::string get_delete_one_query() override;

  void bind_create_one(sqlite3_stmt *, const HistorySimulation &) override;
  void bind_update_one(sqlite3_stmt *, const HistorySimulation &) override;

  void read_row(sqlite3_stmt *, HistorySimulation &) override;

  std::string get_entity_name() override;
  std::string get_entities_name() override;
  std::string get_entity_description(const HistorySimulation &object) override;

 private:
  void bind_base(sqlite3_stmt *, const HistorySimulation &);

  MaterialEntity _material_entity;
  NuclearReactorEntity _nuclear_reactor_entity;
};
