/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Bit-vector solver interface.
 *
 * Describes the interface for the internal bit-vector solver of TheoryBV.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__BV__BV_SOLVER_H
#define AVA6__THEORY__BV__BV_SOLVER_H

#include "smt/env_obj.h"
#include "theory/theory.h"

namespace ava6::internal {
namespace theory {
namespace bv {

class BVSolver : protected EnvObj
{
 public:
  BVSolver(Env& env, TheoryState& state, TheoryInferenceManager& inferMgr)
      : EnvObj(env), d_state(state), d_im(inferMgr) {};

  virtual ~BVSolver() {}

  /**
   * Returns true if we need an equality engine. If so, we initialize the
   * information regarding how it should be setup. For details, see the
   * documentation in Theory::needsEqualityEngine.
   */
  virtual bool needsEqualityEngine(AVA6_UNUSED EeSetupInfo& esi)
  {
    return false;
  }

  virtual void finishInit() {};

  virtual void preRegisterTerm(TNode n) = 0;

  /**
   * Forwarded from TheoryBV::preCheck().
   */
  virtual bool preCheck(
      AVA6_UNUSED Theory::Effort level = Theory::Effort::EFFORT_FULL)
  {
    return false;
  }
  /**
   * Forwarded from TheoryBV::postCheck().
   */
  virtual void postCheck(
      AVA6_UNUSED Theory::Effort level = Theory::Effort::EFFORT_FULL) {};
  /**
   * Forwarded from TheoryBV:preNotifyFact().
   */
  virtual bool preNotifyFact(AVA6_UNUSED TNode atom,
                             AVA6_UNUSED bool pol,
                             AVA6_UNUSED TNode fact,
                             AVA6_UNUSED bool isPrereg,
                             AVA6_UNUSED bool isInternal)
  {
    return false;
  }
  /**
   * Forwarded from TheoryBV::notifyFact().
   */
  virtual void notifyFact(AVA6_UNUSED TNode atom,
                          AVA6_UNUSED bool pol,
                          AVA6_UNUSED TNode fact,
                          AVA6_UNUSED bool isInternal)
  {
  }

  virtual bool needsCheckLastEffort() { return false; }

  virtual void propagate(AVA6_UNUSED Theory::Effort e) {}

  virtual TrustNode explain(AVA6_UNUSED TNode n)
  {
    Unimplemented() << "BVSolver propagated a node but doesn't implement the "
                       "BVSolver::explain() interface!";
    return TrustNode::null();
  }

  /** Additionally collect terms relevant for collecting model values. */
  virtual void computeRelevantTerms(AVA6_UNUSED std::set<Node>& termSet) {}

  /** Collect model values in m based on the relevant terms given by termSet */
  virtual bool collectModelValues(TheoryModel* m,
                                  const std::set<Node>& termSet) = 0;

  virtual std::string identify() const = 0;

  virtual TrustNode ppRewrite(AVA6_UNUSED TNode t) { return TrustNode::null(); }

  virtual void ppStaticLearn(AVA6_UNUSED TNode in,
                             AVA6_UNUSED std::vector<TrustNode>& learned)
  {
  }

  virtual void presolve() {}

  virtual void notifySharedTerm(AVA6_UNUSED TNode t) {}

  virtual EqualityStatus getEqualityStatus(AVA6_UNUSED TNode a,
                                           AVA6_UNUSED TNode b)
  {
    return EqualityStatus::EQUALITY_UNKNOWN;
  }

  /**
   * Get the current value of `node`.
   *
   * The `initialize` flag indicates whether bits should be zero-initialized
   * if they don't have a value yet.
   */
  virtual Node getValue(AVA6_UNUSED TNode node, AVA6_UNUSED bool initialize)
  {
    return Node::null();
  }

  /**
   * @return True if current model is consistent.
   * @note Can only ever be inconsistent in the case of abstraction.
   */
  virtual bool isModelConsistent() const { return true; }

 protected:
  TheoryState& d_state;
  TheoryInferenceManager& d_im;
};

}  // namespace bv
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__BV__BV_SOLVER_H */
