/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The care pair argument callback.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__CARE_PAIR_ARGUMENT_CALLBACK_H
#define AVA6__THEORY__CARE_PAIR_ARGUMENT_CALLBACK_H

#include "expr/node_trie_algorithm.h"
#include "theory/theory.h"

namespace ava6::internal {
namespace theory {

/**
 * The standard callback for computing the care pairs from a node trie.
 */
class CarePairArgumentCallback : public NodeTriePathPairProcessCallback
{
 public:
  CarePairArgumentCallback(Theory& t);
  ~CarePairArgumentCallback() {}
  /**
   * Call on the arguments a and b of two function applications we are
   * computing care pairs for. Returns true if a and b are not already
   * disequal according to theory combination (Theory::areCareDisequal).
   */
  bool considerPath(TNode a, TNode b) override;
  /**
   * Called when we have two function applications that do not have pairs
   * of disequal arguments at any position. We call Theory::processCarePairArgs
   * to add all relevant care pairs.
   */
  void processData(TNode fa, TNode fb) override;

 private:
  /** Reference to theory */
  Theory& d_theory;
};

}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__CARE_ARGUMENT_CALLBACK_H */
