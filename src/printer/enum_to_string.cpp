/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of utilities for printing API enum values.
 */

#include "printer/enum_to_string.h"

namespace ava6::internal {

const char* toString(ava6::SkolemId id)
{
  switch (id)
  {
    case ava6::SkolemId::INTERNAL: return "internal";
    case ava6::SkolemId::PURIFY: return "purify";
    case ava6::SkolemId::GROUND_TERM: return "ground_term";
    case ava6::SkolemId::ARRAY_DEQ_DIFF: return "array_deq_diff";
    case ava6::SkolemId::BV_EMPTY: return "bv_empty";
    case ava6::SkolemId::DIV_BY_ZERO: return "div_by_zero";
    case ava6::SkolemId::INT_DIV_BY_ZERO: return "int_div_by_zero";
    case ava6::SkolemId::MOD_BY_ZERO: return "mod_by_zero";
    case ava6::SkolemId::ARITH_VTS_DELTA: return "arith_vts_delta";
    case ava6::SkolemId::ARITH_VTS_DELTA_FREE: return "arith_vts_delta_free";
    case ava6::SkolemId::ARITH_VTS_INFINITY: return "arith_vts_infinity";
    case ava6::SkolemId::ARITH_VTS_INFINITY_FREE:
      return "arith_vts_infinity_free";
    case ava6::SkolemId::QUANTIFIERS_SKOLEMIZE: return "quantifiers_skolemize";
    case ava6::SkolemId::WITNESS_STRING_LENGTH: return "witness_string_length";
    case ava6::SkolemId::WITNESS_INV_CONDITION: return "witness_inv_condition";
    case ava6::SkolemId::STRINGS_NUM_OCCUR: return "strings_num_occur";
    case ava6::SkolemId::STRINGS_NUM_OCCUR_RE: return "strings_num_occur_re";
    case ava6::SkolemId::STRINGS_OCCUR_INDEX: return "strings_occur_index";
    case ava6::SkolemId::STRINGS_OCCUR_INDEX_RE:
      return "strings_occur_index_re";
    case ava6::SkolemId::STRINGS_DEQ_DIFF: return "strings_deq_diff";
    case ava6::SkolemId::STRINGS_REPLACE_ALL_RESULT:
      return "strings_replace_all_result";
    case ava6::SkolemId::STRINGS_REPLACE_RE_ALL_RESULT:
      return "strings_replace_re_all_result";
    case ava6::SkolemId::STRINGS_ITOS_RESULT: return "strings_itos_result";
    case ava6::SkolemId::STRINGS_STOI_RESULT: return "strings_stoi_result";
    case ava6::SkolemId::STRINGS_STOI_NON_DIGIT:
      return "strings_stoi_non_digit";
    case ava6::SkolemId::RE_UNFOLD_POS_COMPONENT:
      return "re_unfold_pos_component";
    case ava6::SkolemId::SETS_CHOOSE: return "sets_choose";
    case ava6::SkolemId::SETS_DEQ_DIFF: return "sets_deq_diff";
    case ava6::SkolemId::NONE: return "none";
    default: return "?";
  }
}

}  // namespace ava6::internal
