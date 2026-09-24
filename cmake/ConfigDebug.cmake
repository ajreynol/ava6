###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
##

add_definitions(-DAVA6_DEBUG)
add_check_c_cxx_flag("-fno-inline")
set(OPTIMIZATION_LEVEL "g")
# enable_debug_symbols=yes
ava6_set_option(ENABLE_DEBUG_SYMBOLS ON)
# enable_statistics=yes
ava6_set_option(ENABLE_STATISTICS ON)
# enable_assertions=yes
ava6_set_option(ENABLE_ASSERTIONS ON)
# enable_proof=yes
ava6_set_option(ENABLE_PROOFS ON)
# enable_tracing=yes
ava6_set_option(ENABLE_TRACING ON)
# enable_muzzle=no
ava6_set_option(ENABLE_MUZZLE OFF)
# enable_valgrind=optional
ava6_set_option(ENABLE_UNIT_TESTING ON)

# Reset visibility for debug builds (https://github.com/cvc5/cvc5/issues/324)
set(CMAKE_CXX_VISIBILITY_PRESET default)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 0)
