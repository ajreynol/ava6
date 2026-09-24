/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of incompleteness enumeration.
 */

#include "theory/incomplete_id.h"

#include <iostream>

#include "base/check.h"

namespace ava6::internal {
namespace theory {

const char* toString(IncompleteId i)
{
  switch (i)
  {
    case IncompleteId::NONE: return "NONE";
    case IncompleteId::ARITH_NL_DISABLED: return "ARITH_NL_DISABLED";
    case IncompleteId::ARITH_NL: return "ARITH_NL";
    case IncompleteId::QUANTIFIERS: return "QUANTIFIERS";
    case IncompleteId::QUANTIFIERS_FMF: return "QUANTIFIERS_FMF";
    case IncompleteId::QUANTIFIERS_CEGQI: return "QUANTIFIERS_CEGQI";
    case IncompleteId::QUANTIFIERS_RECORDED_INST:
      return "QUANTIFIERS_RECORDED_INST";
    case IncompleteId::QUANTIFIERS_MAX_INST_ROUNDS:
      return "QUANTIFIERS_MAX_INST_ROUNDS";
    case IncompleteId::QUANTIFIERS_SYGUS_NO_WF_GRAMMAR:
      return "QUANTIFIERS_SYGUS_NO_WF_GRAMMAR";
    case IncompleteId::SETS_RELS_CARD: return "SETS_RELS_CARD";
    case IncompleteId::STRINGS_LOOP_SKIP: return "STRINGS_LOOP_SKIP";
    case IncompleteId::STRINGS_REGEXP_NO_SIMPLIFY:
      return "STRINGS_REGEXP_NO_SIMPLIFY";
    case IncompleteId::UNPROCESSED_THEORY_CONFLICT:
      return "UNPROCESSED_THEORY_CONFLICT";
    case IncompleteId::STOP_SEARCH: return "STOP_SEARCH";
    case IncompleteId::PREPROCESSING: return "PREPROCESSING";
    case IncompleteId::UNKNOWN: return "UNKNOWN";
    default:
      DebugUnhandled() << "No print for incomplete id "
                       << static_cast<size_t>(i);
      return "?IncompleteId?";
  }
}

std::ostream& operator<<(std::ostream& out, IncompleteId i)
{
  out << toString(i);
  return out;
}

}  // namespace theory
}  // namespace ava6::internal
