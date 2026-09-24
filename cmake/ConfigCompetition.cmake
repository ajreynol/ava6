###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
##

add_definitions(-DAVA6_COMPETITION_MODE)
add_check_c_cxx_flag("-funroll-all-loops")
add_check_c_cxx_flag("-fexpensive-optimizations")
add_check_c_cxx_flag("-fno-enforce-eh-specs")
# OPTLEVEL=9
set(OPTIMIZATION_LEVEL 9)
# enable_debug_symbols=no
ava6_set_option(ENABLE_DEBUG_SYMBOLS OFF)
# enable_statistics=no
ava6_set_option(ENABLE_STATISTICS OFF)
# enable_assertions=no
ava6_set_option(ENABLE_ASSERTIONS OFF)
# enable_proof=no
ava6_set_option(ENABLE_PROOFS OFF)
# enable_tracing=no
ava6_set_option(ENABLE_TRACING OFF)
# enable_muzzle=yes
ava6_set_option(ENABLE_MUZZLE ON)
# enable_valgrind=no
ava6_set_option(BUILD_SHARED_LIBS OFF)
ava6_set_option(ENABLE_UNIT_TESTING OFF)

# By default, we include all dependencies in our competition build that are
# required to achieve the best performance
set(ENABLE_GPL ON)
ava6_set_option(USE_CADICAL ON)
ava6_set_option(USE_CLN ON)
ava6_set_option(USE_CRYPTOMINISAT ON)
ava6_set_option(USE_SYMFPU ON)
