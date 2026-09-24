/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The theory of booleans.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__BOOLEANS__THEORY_BOOL_H
#define AVA6__THEORY__BOOLEANS__THEORY_BOOL_H

#include "context/context.h"
#include "theory/booleans/proof_checker.h"
#include "theory/booleans/theory_bool_rewriter.h"
#include "theory/theory.h"

namespace ava6::internal {
namespace theory {
namespace booleans {

class TheoryBool : public Theory
{
 public:
  TheoryBool(Env& env, OutputChannel& out, Valuation valuation);

  /** get the official theory rewriter of this theory */
  TheoryRewriter* getTheoryRewriter() override;
  /** get the proof checker of this theory */
  ProofRuleChecker* getProofChecker() override;

  bool ppAssert(TrustNode tin, TrustSubstitutionMap& outSubstitutions) override;

  std::string identify() const override;

 private:
  /** The theory rewriter for this theory. */
  TheoryBoolRewriter d_rewriter;
  /** Proof rule checker */
  BoolProofRuleChecker d_checker;
}; /* class TheoryBool */

}  // namespace booleans
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__BOOLEANS__THEORY_BOOL_H */
