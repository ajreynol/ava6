###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
#
# The build system configuration.
##

configure_file(rational.h.in rational.h)
configure_file(integer.h.in integer.h)


libava6_add_sources(
  bin_heap.h
  bitvector.cpp
  bitvector.h
  bool.h
  cardinality.cpp
  cardinality.h
  cardinality_class.cpp
  cardinality_class.h
  dense_map.h
  didyoumean.cpp
  didyoumean.h
  divisible.cpp
  divisible.h
  gmp_util.h
  hash.h
  iand.h
  index.cpp
  index.h
  ostream_util.cpp
  ostream_util.h
  real_algebraic_number.cpp
  real_algebraic_number.h
  random.cpp
  random.h
  resource_manager.cpp
  resource_manager.h
  result.cpp
  result.h
  regexp.cpp
  regexp.h
  safe_print.cpp
  safe_print.h
  sexpr.cpp
  sexpr.h
  smt2_quote_string.cpp
  smt2_quote_string.h
  statistics_public.cpp
  statistics_public.h
  statistics_registry.cpp
  statistics_registry.h
  statistics_stats.cpp
  statistics_stats.h
  statistics_value.cpp
  statistics_value.h
  string.cpp
  string.h
  uninterpreted_sort_value.cpp
  uninterpreted_sort_value.h
  utility.cpp
  utility.h
)



  libava6_add_sources(rational_gmp_imp.cpp integer_gmp_imp.cpp)

