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
ava6_add_unit_test_black(regexp_operation_black theory)
ava6_add_unit_test_black(theory_black theory)
ava6_add_unit_test_white(evaluator_white theory)
ava6_add_unit_test_white(logic_info_white theory)
ava6_add_unit_test_white(rewriter_white theory)
ava6_add_unit_test_white(sequences_rewriter_white theory)
ava6_add_unit_test_white(strings_rewriter_white theory)
ava6_add_unit_test_white(theory_arith_white theory)
ava6_add_unit_test_black(theory_arith_rewriter_black theory)
ava6_add_unit_test_black(theory_bv_black theory)
ava6_add_unit_test_white(theory_engine_white theory)
ava6_add_unit_test_white(theory_quantifiers_bv_instantiator_white theory)
ava6_add_unit_test_white(theory_sets_type_enumerator_white theory)
ava6_add_unit_test_white(theory_sets_type_rules_white theory)
ava6_add_unit_test_white(theory_strings_skolem_cache_black theory)
ava6_add_unit_test_white(theory_strings_utils_white theory)
ava6_add_unit_test_white(theory_strings_word_white theory)
ava6_add_unit_test_white(theory_white theory)
ava6_add_unit_test_white(type_enumerator_white theory)
ava6_add_unit_test_white(arith_poly_white theory)
