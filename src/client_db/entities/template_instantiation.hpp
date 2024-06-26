#pragma once

#include <vector>

#include "../client_db_impl.hpp"
#include "history_simulation.hpp"
#include "material.hpp"
#include "nuclear_reactor.hpp"

#define TEMPLATES_0_PARAM(T) \
  template bool ClientDbImpl::create_one<T##Entity>(T &, int *);\
  template bool ClientDbImpl::read_one<T##Entity>(const int, T &, int *);\
  template bool ClientDbImpl::read_all<T##Entity>(std::vector<T> &, int *);\
  template bool ClientDbImpl::update_one<T##Entity>(const T &, int *);\
  template bool ClientDbImpl::delete_one<T##Entity>(const T &, int *);

#define TEMPLATES_1_PARAM(T, Param) \
  template bool ClientDbImpl::create_one<T##Entity>(T &, int *, Param);\
  template bool ClientDbImpl::read_one<T##Entity>(const int, T &, int *);\
  template bool ClientDbImpl::read_all<T##Entity>(std::vector<T> &, int *);\
  template bool ClientDbImpl::update_one<T##Entity>(const T &, int *);\
  template bool ClientDbImpl::delete_one<T##Entity>(const T &, int *);

TEMPLATES_0_PARAM(HistorySimulation) // NOLINT(readability/fn_size)
TEMPLATES_1_PARAM(Material, bool &&) // NOLINT(readability/fn_size)
TEMPLATES_1_PARAM(NuclearReactor, bool &&) // NOLINT(readability/fn_size)
