/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The theory equality notify utility.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__THEORY_EQ_NOTIFY_H
#define AVA6__THEORY__THEORY_EQ_NOTIFY_H

#include "expr/node.h"
#include "theory/theory_inference_manager.h"
#include "theory/uf/equality_engine_notify.h"

namespace ava6::internal {
namespace theory {

/**
 * The default class for equality engine callbacks for a theory. This forwards
 * calls for trigger predicates, trigger term equalities and conflicts due to
 * constant merges to the provided theory inference manager.
 */
class TheoryEqNotifyClass : public eq::EqualityEngineNotify
{
 public:
  TheoryEqNotifyClass(TheoryInferenceManager& im) : d_im(im) {}
  ~TheoryEqNotifyClass() {}

  bool eqNotifyTriggerPredicate(TNode predicate, bool value) override
  {
    if (value)
    {
      return d_im.propagateLit(predicate);
    }
    return d_im.propagateLit(predicate.notNode());
  }
  bool eqNotifyTriggerTermEquality(AVA6_UNUSED TheoryId tag,
                                   TNode t1,
                                   TNode t2,
                                   bool value) override
  {
    if (value)
    {
      return d_im.propagateLit(t1.eqNode(t2));
    }
    return d_im.propagateLit(t1.eqNode(t2).notNode());
  }
  void eqNotifyConstantTermMerge(TNode t1, TNode t2) override
  {
    d_im.conflictEqConstantMerge(t1, t2);
  }
  void eqNotifyNewClass(AVA6_UNUSED TNode t) override
  {
    // do nothing
  }
  void eqNotifyMerge(AVA6_UNUSED TNode t1, AVA6_UNUSED TNode t2) override
  {
    // do nothing
  }
  void eqNotifyDisequal(AVA6_UNUSED TNode t1,
                        AVA6_UNUSED TNode t2,
                        AVA6_UNUSED TNode reason) override
  {
    // do nothing
  }

 protected:
  /** Reference to the theory inference manager */
  TheoryInferenceManager& d_im;
};

}  // namespace theory
}  // namespace ava6::internal

#endif
