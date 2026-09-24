/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Notification class for the master equality engine
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__QUANTIFIERS__MASTER_EQ_NOTIFY__H
#define AVA6__THEORY__QUANTIFIERS__MASTER_EQ_NOTIFY__H

#include <memory>

#include "theory/uf/equality_engine_notify.h"

namespace ava6::internal {
namespace theory {

class QuantifiersEngine;

namespace quantifiers {

/** notify class for master equality engine */
class MasterNotifyClass : public theory::eq::EqualityEngineNotify
{
 public:
  MasterNotifyClass(QuantifiersEngine* qe);
  /**
   * Called when a new equivalence class is created in the master equality
   * engine.
   */
  void eqNotifyNewClass(TNode t) override;

  bool eqNotifyTriggerPredicate(AVA6_UNUSED TNode predicate,
                                AVA6_UNUSED bool value) override
  {
    return true;
  }
  bool eqNotifyTriggerTermEquality(AVA6_UNUSED TheoryId tag,
                                   AVA6_UNUSED TNode t1,
                                   AVA6_UNUSED TNode t2,
                                   AVA6_UNUSED bool value) override
  {
    return true;
  }
  void eqNotifyConstantTermMerge(AVA6_UNUSED TNode t1,
                                 AVA6_UNUSED TNode t2) override
  {
  }
  void eqNotifyMerge(AVA6_UNUSED TNode t1, AVA6_UNUSED TNode t2) override;
  void eqNotifyDisequal(AVA6_UNUSED TNode t1,
                        AVA6_UNUSED TNode t2,
                        AVA6_UNUSED TNode reason) override
  {
  }

 private:
  /** Pointer to quantifiers engine */
  QuantifiersEngine* d_quantEngine;
};

}  // namespace quantifiers
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__QUANTIFIERS__MASTER_EQ_NOTIFY__H */
