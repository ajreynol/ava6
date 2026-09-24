/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Equality engine manager for central equality engine architecture
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__EE_MANAGER__H
#define AVA6__THEORY__EE_MANAGER__H

#include <map>
#include <memory>

#include "smt/env_obj.h"
#include "theory/ee_setup_info.h"
#include "theory/theory.h"
#include "theory/quantifiers/master_eq_notify.h"
#include "theory/uf/equality_engine.h"

namespace ava6::internal {
class TheoryEngine;

namespace theory {

class SharedSolver;

struct EeTheoryInfo
{
  EeTheoryInfo() : d_usedEe(nullptr) {}
  /** Equality engine that is used (if it exists) */
  eq::EqualityEngine* d_usedEe;
};



/**
 * The (central) equality engine manager. This encapsulates an architecture
 * in which all applicable theories use a single central equality engine.
 *
 * This class is not responsible for actually initializing equality engines in
 * theories (since this class does not have access to the internals of Theory).
 * Instead, it is only responsible for the construction of the equality
 * engine objects themselves. TheoryEngine is responsible for querying this
 * class during finishInit() to determine the equality engines to pass to each
 * theories based on getEeTheoryInfo.
 *
 * It also may allocate a "master" equality engine, which is intuitively the
 * equality engine of the theory of quantifiers. If all theories use the
 * central equality engine, then the master equality engine is the same as the
 * central equality engine.
 *
 * The theories that use central equality engine are determined by
 * Theory::usesCentralEqualityEngine.
 *
 * The main idea behind this class is to use a notification class on the
 * central equality engine which dispatches *multiple* notifications to the
 * theories that use the central equality engine.
 */
class EqEngineManager : protected EnvObj
{
 public:
  EqEngineManager(Env& env, TheoryEngine& te, SharedSolver& shs);
  ~EqEngineManager();
  /**
   * Initialize theories. This method allocates unique equality engines
   * per theories and connects them to a master equality engine.
   */
  void initializeTheories();

  /**
   * Return true if the theory with the given id uses central equality engine
   * with the given options.
   */
  static bool usesCentralEqualityEngine(TheoryId id);
  const EeTheoryInfo* getEeTheoryInfo(TheoryId tid) const;
  eq::EqualityEngine* allocateEqualityEngine(EeSetupInfo& esi,
                                           context::Context* c);

 private:
  TheoryEngine& d_te;
  SharedSolver& d_sharedSolver;
  std::map<TheoryId, EeTheoryInfo> d_einfo;
  /**
   * Notify class for central equality engine. This class dispatches
   * notifications from the central equality engine to the appropriate
   * theory(s).
   */
  class CentralNotifyClass : public theory::eq::EqualityEngineNotify
  {
   public:
    CentralNotifyClass(EqEngineManager& eemc);
    bool eqNotifyTriggerPredicate(TNode predicate, bool value) override;
    bool eqNotifyTriggerTermEquality(TheoryId tag,
                                     TNode t1,
                                     TNode t2,
                                     bool value) override;
    void eqNotifyConstantTermMerge(TNode t1, TNode t2) override;
    void eqNotifyNewClass(TNode t) override;
    void eqNotifyMerge(TNode t1, TNode t2) override;
    void eqNotifyDisequal(TNode t1, TNode t2, TNode reason) override;
    /** Parent */
    EqEngineManager& d_eemc;
    /** List of notify classes that need new class notification */
    std::vector<eq::EqualityEngineNotify*> d_newClassNotify;
    /** List of notify classes that need merge notification */
    std::vector<eq::EqualityEngineNotify*> d_mergeNotify;
    /** List of notify classes that need disequality notification */
    std::vector<eq::EqualityEngineNotify*> d_disequalNotify;
  };
  /** Notification when predicate gets value in central equality engine */
  bool eqNotifyTriggerPredicate(TNode predicate, bool value);
  bool eqNotifyTriggerTermEquality(TheoryId tag,
                                   TNode t1,
                                   TNode t2,
                                   bool value);
  /** Notification when constants are merged in central equality engine */
  void eqNotifyConstantTermMerge(TNode t1, TNode t2);
  /** The master equality engine notify class */
  std::unique_ptr<quantifiers::MasterNotifyClass> d_masterEENotify;
  /** The central equality engine notify class */
  CentralNotifyClass d_centralEENotify;
  /** The central equality engine. */
  eq::EqualityEngine d_centralEqualityEngine;
  /** The proof equality engine for the central equality engine */
  std::unique_ptr<eq::ProofEqEngine> d_centralPfee;
};

}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__EE_MANAGER__H */
