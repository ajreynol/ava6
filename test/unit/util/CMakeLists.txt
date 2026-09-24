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

# Add unit tests.
ava6_add_unit_test_white(array_store_all_white util)
ava6_add_unit_test_white(assert_white util)
ava6_add_unit_test_black(binary_heap_black util)
ava6_add_unit_test_black(bitvector_black util)
ava6_add_unit_test_black(cardinality_black util)
ava6_add_unit_test_white(check_white util)
ava6_add_unit_test_black(configuration_black util)
ava6_add_unit_test_black(datatype_black util)
ava6_add_unit_test_white(didyoumean_black util)
ava6_add_unit_test_black(exception_black util)
ava6_add_unit_test_black(integer_black util)
ava6_add_unit_test_white(integer_white util)
ava6_add_unit_test_black(output_black util)
ava6_add_unit_test_black(rational_black util)
ava6_add_unit_test_white(rational_white util)
if(AVA6_USE_POLY_IMP)
ava6_add_unit_test_black(real_algebraic_number_black util)
endif()
ava6_add_unit_test_white(resource_manager_white util)
ava6_add_unit_test_black(stats_black util)
