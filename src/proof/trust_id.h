/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Trust identifier enumeration
 */

#include "ava6_private.h"

#ifndef AVA6__PROOF__TRUST_ID_H
#define AVA6__PROOF__TRUST_ID_H

#include "expr/node.h"

namespace ava6::internal {

/**
 * Identifiers for trusted steps in proofs.
 */
enum class TrustId : uint32_t
{
  NONE,
  /** A lemma sent by a theory without a proof */
  THEORY_LEMMA,
  /**
   * A step proving false, used as a trust step when the prop engine is not SAT
   * proof producing (--proof-mode=pp-only).
   */
  SMT_REFUTATION,
  /**
   * An internal inference made by a theory without a proof. These are split
   * per theory, and introduced as needed.
   */
  THEORY_INFERENCE_ARITH,
  THEORY_INFERENCE_ARRAYS,
  THEORY_INFERENCE_DATATYPES,
  THEORY_INFERENCE_SETS,
  THEORY_INFERENCE_STRINGS,
  /** A ppStaticRewrite step */
  PP_STATIC_REWRITE,
  /** A rewrite of the input formula made by a theory during preprocessing
     without a proof */
  THEORY_PREPROCESS,
  /** A lemma added during theory-preprocessing without a proof */
  THEORY_PREPROCESS_LEMMA,
  /** BvToBool preprocessing pass */
  PREPROCESS_BV_TO_BOOL,
  /** BoolToBv preprocessing pass */
  PREPROCESS_BOOL_TO_BV,
  /** Ackermann preprocessing pass */
  PREPROCESS_ACKERMANN,
  PREPROCESS_ACKERMANN_LEMMA,
  /** StaticLearning preprocessing pass */
  PREPROCESS_STATIC_LEARNING_LEMMA,
  /** QuantifiersPreprocess preprocessing pass */
  PREPROCESS_QUANTIFIERS_PP,
  /** RealToInt preprocessing pass */
  PREPROCESS_REAL_TO_INT,
  /** A step from the distinct extension */
  UF_DISTINCT,
  /**
   * A conversion between a literal used in the inference id lemma
   * InferenceId::ARITH_NL_COMPARISON and a relation between absolute
   * values as used by ProofRule::ARITH_MULT_ABS_COMPARISON.
   */
  ARITH_NL_COMPARE_LIT_TRANSFORM,
  /** A lemma from the DIO solver */
  ARITH_DIO_LEMMA,
  /** A lemma from the ArithStaticLearner utility */
  ARITH_STATIC_LEARN,
  /** A nonlinear comparison lemma that failed proof reconstruction */
  ARITH_NL_COMPARE_LEMMA,
  /** A nonlinear flatten monomial lemma that failed proof reconstruction */
  ARITH_NL_FLATTEN_MON_LEMMA,
  /** A conflict coming from the bitblast solver */
  /** A step from BvPpAssert utility */
  BV_PP_ASSERT,
  /** Diamonds preprocessing in TheoryUf::ppStaticLearn */
  DIAMONDS,
  /** An extended theory rewrite */
  EXT_THEORY_REWRITE,
  /** A rewrite whose proof could not be elaborated */
  REWRITE_NO_ELABORATE,
  /** A flattening rewrite in an equality engine proof */
  FLATTENING_REWRITE,
  /** A proof of an applied substitution that could not be no elaborate */
  SUBS_NO_ELABORATE,
  /** A proof of an applied substitution that could not be reconstructed during
     solving */
  SUBS_MAP,
  /** A proof of a substitution x=t that could not be shown by rewrite */
  SUBS_EQ,
  /** A step of the form (~ s t) = (~ (to_real s) (to_real t)) */
  ARITH_PRED_CAST_TYPE,
  /** A quantifiers preprocessing step that was given without a proof */
  QUANTIFIERS_PREPROCESS,
  /** A quantifiers rewriting step for instantiations, e.g. virtual term
     substitution */
  QUANTIFIERS_INST_REWRITE,
  /** A rewrite performed at TheoryStrings::ppStaticRewrite */
  STRINGS_PP_STATIC_REWRITE,
  /**
   * An existential corresponding to a witness term introduced e.g. in
   * quantifier instantiation
   */
  VALID_WITNESS,
  /** A subtype elimination step that could not be processed */
  SUBTYPE_ELIMINATION,
  /** A rewrite required for showing a macro theory rewrite */
  MACRO_THEORY_REWRITE_RCONS,
  /**
   * A rewrite required for showing a macro theory rewrite that should not
   * require the use of theory rewrites to prove.
   */
  MACRO_THEORY_REWRITE_RCONS_SIMPLE,
  /** Untracked sources of trust, which are discouraged */
  /** A rewrite of the input formula by a preprocessing pass without a proof */
  UNKNOWN_PREPROCESS,
  /** A lemma added during preprocessing without a proof */
  UNKNOWN_PREPROCESS_LEMMA,
};
/** Converts a trust id to a string. */
const char* toString(TrustId id);
/** Write a trust id to out */
std::ostream& operator<<(std::ostream& out, TrustId id);
/** Make a trust id node */
Node mkTrustId(NodeManager* nm, TrustId id);
/** get a trust identifier from a node, return false if we fail */
bool getTrustId(TNode n, TrustId& i);

}  // namespace ava6::internal

#endif /* AVA6__PROOF__METHOD_ID_H */
