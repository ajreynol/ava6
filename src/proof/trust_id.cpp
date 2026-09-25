/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of trust identifier
 */

#include "proof/trust_id.h"

#include "proof/proof_checker.h"
#include "util/rational.h"

using namespace ava6::internal::kind;

namespace ava6::internal {

const char* toString(TrustId id)
{
  switch (id)
  {
    case TrustId::NONE: return "NONE";
    case TrustId::THEORY_LEMMA: return "THEORY_LEMMA";
    case TrustId::SMT_REFUTATION: return "SMT_REFUTATION";
    // core
    case TrustId::THEORY_INFERENCE_ARITH: return "THEORY_INFERENCE_ARITH";
    case TrustId::THEORY_INFERENCE_ARRAYS: return "THEORY_INFERENCE_ARRAYS";
    case TrustId::THEORY_INFERENCE_DATATYPES:
      return "THEORY_INFERENCE_DATATYPES";
    case TrustId::THEORY_INFERENCE_SETS: return "THEORY_INFERENCE_SETS";
    case TrustId::THEORY_INFERENCE_STRINGS: return "THEORY_INFERENCE_STRINGS";
    case TrustId::PP_STATIC_REWRITE: return "PP_STATIC_REWRITE";
    case TrustId::THEORY_PREPROCESS: return "THEORY_PREPROCESS";
    case TrustId::THEORY_PREPROCESS_LEMMA: return "THEORY_PREPROCESS_LEMMA";
    // preprocess passes
    case TrustId::PREPROCESS_BV_TO_BOOL: return "PREPROCESS_BV_TO_BOOL";
    case TrustId::PREPROCESS_BOOL_TO_BV: return "PREPROCESS_BOOL_TO_BV";
    case TrustId::PREPROCESS_ACKERMANN: return "PREPROCESS_ACKERMANN";
    case TrustId::PREPROCESS_ACKERMANN_LEMMA:
      return "PREPROCESS_ACKERMANN_LEMMA";
    case TrustId::PREPROCESS_STATIC_LEARNING_LEMMA:
      return "PREPROCESS_STATIC_LEARNING_LEMMA";
    case TrustId::PREPROCESS_QUANTIFIERS_PP: return "PREPROCESS_QUANTIFIERS_PP";
    case TrustId::PREPROCESS_REAL_TO_INT: return "PREPROCESS_REAL_TO_INT";
    case TrustId::UF_DISTINCT: return "UF_DISTINCT";
    case TrustId::ARITH_NL_COMPARE_LIT_TRANSFORM:
      return "ARITH_NL_COMPARE_LIT_TRANSFORM";
    case TrustId::ARITH_DIO_LEMMA: return "ARITH_DIO_LEMMA";
    case TrustId::ARITH_STATIC_LEARN: return "ARITH_STATIC_LEARN";
    case TrustId::ARITH_NL_COMPARE_LEMMA: return "ARITH_NL_COMPARE_LEMMA";
    case TrustId::ARITH_NL_FLATTEN_MON_LEMMA:
      return "ARITH_NL_FLATTEN_MON_LEMMA";
    case TrustId::BV_PP_ASSERT: return "BV_PP_ASSERT";
    case TrustId::DIAMONDS: return "DIAMONDS";
    case TrustId::EXT_THEORY_REWRITE: return "EXT_THEORY_REWRITE";
    case TrustId::REWRITE_NO_ELABORATE: return "REWRITE_NO_ELABORATE";
    case TrustId::FLATTENING_REWRITE: return "FLATTENING_REWRITE";
    case TrustId::SUBS_NO_ELABORATE: return "SUBS_NO_ELABORATE";
    case TrustId::SUBS_MAP: return "SUBS_MAP";
    case TrustId::SUBS_EQ: return "SUBS_EQ";
    case TrustId::ARITH_PRED_CAST_TYPE: return "ARITH_PRED_CAST_TYPE";
    case TrustId::QUANTIFIERS_PREPROCESS: return "QUANTIFIERS_PREPROCESS";
    case TrustId::QUANTIFIERS_INST_REWRITE: return "QUANTIFIERS_INST_REWRITE";
    case TrustId::STRINGS_PP_STATIC_REWRITE: return "STRINGS_PP_STATIC_REWRITE";
    case TrustId::VALID_WITNESS: return "VALID_WITNESS";
    case TrustId::SUBTYPE_ELIMINATION: return "SUBTYPE_ELIMINATION";
    case TrustId::MACRO_THEORY_REWRITE_RCONS:
      return "MACRO_THEORY_REWRITE_RCONS";
    case TrustId::MACRO_THEORY_REWRITE_RCONS_SIMPLE:
      return "MACRO_THEORY_REWRITE_RCONS_SIMPLE";
    case TrustId::UNKNOWN_PREPROCESS: return "UNKNOWN_PREPROCESS";
    case TrustId::UNKNOWN_PREPROCESS_LEMMA: return "UNKNOWN_PREPROCESS_LEMMA";
    default: return "TrustId::Unknown";
  };
}

std::ostream& operator<<(std::ostream& out, TrustId id)
{
  out << toString(id);
  return out;
}

Node mkTrustId(NodeManager* nm, TrustId id)
{
  return nm->mkConstInt(Rational(static_cast<uint32_t>(id)));
}

bool getTrustId(TNode n, TrustId& i)
{
  uint32_t index;
  if (!ProofRuleChecker::getUInt32(n, index))
  {
    return false;
  }
  i = static_cast<TrustId>(index);
  return true;
}

}  // namespace ava6::internal
