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

#include "theory/ee_manager.h"

#include "options/arith_options.h"
#include "options/theory_options.h"
#include "smt/env.h"
#include "theory/quantifiers_engine.h"
#include "theory/shared_solver.h"
#include "theory/theory_engine.h"
#include "theory/theory_state.h"

namespace ava6::internal {
namespace theory {

EqEngineManager::EqEngineManager(Env& env,
                                               TheoryEngine& te,
                                               SharedSolver& shs)
    : EnvObj(env),
      d_te(te),
      d_sharedSolver(shs),
      d_masterEENotify(nullptr),
      d_centralEENotify(*this),
      d_centralEqualityEngine(
          env, context(), d_centralEENotify, "central::ee", true)
{
  if (env.isTheoryProofProducing())
  {
    d_centralPfee =
        std::make_unique<eq::ProofEqEngine>(env, d_centralEqualityEngine);
    d_centralEqualityEngine.setProofEqualityEngine(d_centralPfee.get());
  }
}

EqEngineManager::~EqEngineManager() {}

void EqEngineManager::initializeTheories()
{
  EeSetupInfo esis;
  AlwaysAssert(d_sharedSolver.needsEqualityEngine(esis));
  d_sharedSolver.setEqualityEngine(&d_centralEqualityEngine);
  const LogicInfo& linfo = logicInfo();
  if (linfo.isQuantified())
  {
    QuantifiersEngine* qe = d_te.getQuantifiersEngine();
    Assert(qe != nullptr);
    d_masterEENotify = std::make_unique<quantifiers::MasterNotifyClass>(qe);
    d_centralEENotify.d_newClassNotify.push_back(d_masterEENotify.get());
  }
  for (TheoryId id = THEORY_FIRST; id != THEORY_LAST; ++id)
  {
    Theory* t = d_te.theoryOf(id);
    EeSetupInfo esi;
    if (t == nullptr)
    {
      continue;
    }
    bool needsEe = t->needsEqualityEngine(esi);
    t->setEqualityEngine(needsEe ? &d_centralEqualityEngine : nullptr);
    if (!needsEe)
    {
      continue;
    }
    // Quantifiers use the same engine as the other theories. Their new-class
    // notifications are dispatched through the master notify object above.
    Assert(esi.d_useMaster || usesCentralEqualityEngine(id));
    if (esi.d_useMaster || !linfo.isTheoryEnabled(id))
    {
      continue;
    }
    if (esi.needsNotifyNewClass())
    {
      d_centralEENotify.d_newClassNotify.push_back(esi.d_notify);
    }
    if (esi.needsNotifyMerge())
    {
      d_centralEENotify.d_mergeNotify.push_back(esi.d_notify);
    }
    if (esi.needsNotifyDisequal())
    {
      d_centralEENotify.d_disequalNotify.push_back(esi.d_notify);
    }
  }
}

bool EqEngineManager::usesCentralEqualityEngine(TheoryId id)
{
  return id == THEORY_BUILTIN || id == THEORY_ARITH || id == THEORY_UF
         || id == THEORY_DATATYPES || id == THEORY_SETS || id == THEORY_STRINGS
         || id == THEORY_ARRAYS || id == THEORY_BV;
}

EqEngineManager::CentralNotifyClass::CentralNotifyClass(
    EqEngineManager& eemc)
    : d_eemc(eemc)
{
}

bool EqEngineManager::CentralNotifyClass::eqNotifyTriggerPredicate(
    TNode predicate, bool value)
{
  Trace("eem-central") << "eqNotifyTriggerPredicate: " << predicate
                       << std::endl;
  return d_eemc.eqNotifyTriggerPredicate(predicate, value);
}

bool EqEngineManager::CentralNotifyClass::eqNotifyTriggerTermEquality(
    TheoryId tag, TNode t1, TNode t2, bool value)
{
  Trace("eem-central") << "eqNotifyTriggerTermEquality: " << t1 << " " << t2
                       << value << ", tag = " << tag << std::endl;
  return d_eemc.eqNotifyTriggerTermEquality(tag, t1, t2, value);
}

void EqEngineManager::CentralNotifyClass::eqNotifyConstantTermMerge(
    TNode t1, TNode t2)
{
  Trace("eem-central") << "eqNotifyConstantTermMerge: " << t1 << " " << t2
                       << std::endl;
  d_eemc.eqNotifyConstantTermMerge(t1, t2);
}

void EqEngineManager::CentralNotifyClass::eqNotifyNewClass(TNode t)
{
  Trace("eem-central") << "...eqNotifyNewClass " << t << std::endl;
  // notify all theories that have new equivalence class notifications
  for (eq::EqualityEngineNotify* notify : d_newClassNotify)
  {
    notify->eqNotifyNewClass(t);
  }
}

void EqEngineManager::CentralNotifyClass::eqNotifyMerge(TNode t1,
                                                               TNode t2)
{
  Trace("eem-central") << "...eqNotifyMerge " << t1 << ", " << t2 << std::endl;
  // notify all theories that have merge notifications
  for (eq::EqualityEngineNotify* notify : d_mergeNotify)
  {
    notify->eqNotifyMerge(t1, t2);
  }
}

void EqEngineManager::CentralNotifyClass::eqNotifyDisequal(TNode t1,
                                                                  TNode t2,
                                                                  TNode reason)
{
  Trace("eem-central") << "...eqNotifyDisequal " << t1 << ", " << t2
                       << std::endl;
  // notify all theories that have disequal notifications
  for (eq::EqualityEngineNotify* notify : d_disequalNotify)
  {
    notify->eqNotifyDisequal(t1, t2, reason);
  }
}

bool EqEngineManager::eqNotifyTriggerPredicate(TNode predicate,
                                                      bool value)
{
  // always propagate with the shared solver
  Trace("eem-central") << "...propagate " << predicate << ", " << value
                       << " with shared solver" << std::endl;
  return d_sharedSolver.propagateLit(predicate, value);
}

bool EqEngineManager::eqNotifyTriggerTermEquality(TheoryId tag,
                                                         TNode a,
                                                         TNode b,
                                                         bool value)
{
  // propagate to theory engine
  bool ok = d_sharedSolver.propagateLit(a.eqNode(b), value);
  if (!ok)
  {
    return false;
  }
  // no need to propagate shared term equalities to the UF theory
  if (tag == THEORY_UF)
  {
    return true;
  }
  // propagate shared equality
  return d_sharedSolver.propagateSharedEquality(tag, a, b, value);
}

void EqEngineManager::eqNotifyConstantTermMerge(TNode t1, TNode t2)
{
  Node lit = t1.eqNode(t2);
  TrustNode conflict;
  if (d_centralPfee != nullptr)
  {
    conflict = d_centralPfee->assertConflict(lit);
  }
  else
  {
    Node conf = d_centralEqualityEngine.mkExplainLit(lit);
    conflict = TrustNode::mkTrustConflict(conf);
  }
  Trace("eem-central") << "...explained conflict of " << lit << " ... "
                       << conflict << std::endl;
  d_sharedSolver.sendConflict(conflict, InferenceId::EQ_CONSTANT_MERGE);
  return;
}

}  // namespace theory
}  // namespace ava6::internal
