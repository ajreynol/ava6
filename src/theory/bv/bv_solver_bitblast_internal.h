/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Bit-blast solver that sends bit-blast lemmas directly to the internal
 * CaDiCaL.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__BV__BV_SOLVER_BITBLAST_INTERNAL_H
#define AVA6__THEORY__BV__BV_SOLVER_BITBLAST_INTERNAL_H

#include "proof/eager_proof_generator.h"
#include "smt/env_obj.h"
#include "theory/bv/bitblast/proof_bitblaster.h"
#include "theory/theory_state.h"
#include "theory/theory_inference_manager.h"

namespace ava6::internal {
namespace theory {
namespace bv {

/**
 * Bit-blasting solver that sends bit-blasting lemmas directly to the
 * internal CaDiCaL. It is also able to handle atoms of kind
 * BITVECTOR_EAGER_ATOM.
 *
 * Sends lemmas atom <=> bb(atom) to CaDiCaL on preNotifyFact().
 */
class BVSolverBitblastInternal : protected EnvObj
{
 public:
  BVSolverBitblastInternal(Env& env,
                           TheoryState* state,
                           TheoryInferenceManager& inferMgr);
  ~BVSolverBitblastInternal() = default;

  bool needsEqualityEngine(EeSetupInfo& esi);

  bool preNotifyFact(TNode atom,
                     bool pol,
                     TNode fact,
                     bool isPrereg,
                     bool isInternal);

  TrustNode explain(TNode n);

  bool collectModelValues(TheoryModel* m,
                          const std::set<Node>& termSet);

  Node getValue(TNode node, bool initialize);

 private:
  TheoryState& d_state;
  TheoryInferenceManager& d_im;

  /**
   * Sends a bit-blasting lemma fact <=> d_bitblaster.bbAtom(fact) to the
   * inference manager.
   */
  void addBBLemma(TNode fact);

  /** Bit-blaster used to bit-blast atoms/terms. */
  std::unique_ptr<BBProof> d_bitblaster;
  /** Proof generator for unpacking BITVECTOR_EAGER_ATOM. */
  std::unique_ptr<EagerProofGenerator> d_epg;
};

}  // namespace bv
}  // namespace theory
}  // namespace ava6::internal

#endif
