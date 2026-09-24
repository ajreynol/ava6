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
ava6_add_unit_test_black(api_command_black api/cpp)
ava6_add_unit_test_black(api_datatype_black api/cpp)
ava6_add_unit_test_black(api_input_parser_black api/cpp)
ava6_add_unit_test_black(api_kind_black api/cpp)
ava6_add_unit_test_black(api_lifetime_black api/cpp)
ava6_add_unit_test_black(api_op_black api/cpp)
ava6_add_unit_test_white(api_op_white api/cpp)
ava6_add_unit_test_black(api_parametric_datatype_black api/cpp)
ava6_add_unit_test_black(api_parser_lifetime_black api/cpp)
ava6_add_unit_test_black(api_proof_black api/cpp)
ava6_add_unit_test_black(api_proof_rule_black api/cpp)
ava6_add_unit_test_black(api_result_black api/cpp)
ava6_add_unit_test_black(api_skolem_id_black api/cpp)
ava6_add_unit_test_black(api_solver_black api/cpp)
ava6_add_unit_test_white(api_solver_white api/cpp)
ava6_add_unit_test_black(api_sort_black api/cpp)
ava6_add_unit_test_black(api_sort_kind_black api/cpp)
ava6_add_unit_test_black(api_symbol_manager_black api/cpp)
ava6_add_unit_test_black(api_term_black api/cpp)
ava6_add_unit_test_white(api_term_white api/cpp)
ava6_add_unit_test_black(api_term_manager_black api/cpp)

# Tests that deliberately exercise deprecated enum conversion functions.
set_source_files_properties(api_deprecated_black.cpp
  PROPERTIES COMPILE_OPTIONS
  "-Wno-deprecated-declarations;-Wno-error=deprecated-declarations")
ava6_add_unit_test_black(api_deprecated_black api/cpp)
