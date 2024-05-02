#pragma once

#include "../client_db_impl.hpp"
#include "nuclear_reactor.hpp"

#define TEMPLATES_0_PARAM(T) \
  template bool ClientDbImpl::create_one<T##Entity>(T &, int *);\
  template bool ClientDbImpl::read_one<T##Entity>(const int, T &, int *);

#define TEMPLATES_1_PARAM(T,Param) \
  template bool ClientDbImpl::create_one<T##Entity>(T &, int *, Param);\
  template bool ClientDbImpl::read_one<T##Entity>(const int, T &, int *);

TEMPLATES_1_PARAM(NuclearReactor, bool &&)
