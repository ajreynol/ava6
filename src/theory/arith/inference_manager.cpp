/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of the inference manager for the theory of strings.
 */

#include "theory/arith/inference_manager.h"

#include "options/arith_options.h"
#include "theory/arith/theory_arith.h"
#include "theory/rewriter.h"

namespace ava6::internal {
namespace theory {
namespace arith {

InferenceManager::InferenceManager(Env& env, TheoryArith& ta, TheoryState& s)
    : InferenceManagerBuffered(env, ta, s, "theory::arith::"),
      // currently must track propagated literals if using the equality solver
      d_trackPropLits(options().arith.arithEqSolver),
      d_propLits(context())
{
}

void InferenceManager::addPendingLemma(std::unique_ptr<SimpleTheoryLemma> lemma,
                                       bool isWaiting)
{
  Trace("arith::infman") << "Add " << lemma->getId() << " " << lemma->d_node
                         << (isWaiting ? " as waiting" : "") << std::endl;
  if (hasCachedLemma(lemma->d_node, lemma->d_property))
  {
    return;
  }
  if (isWaiting)
  {
    d_waitingLem.emplace_back(std::move(lemma));
  }
  else
  {
    d_pendingLem.emplace_back(std::move(lemma));
  }
}
void InferenceManager::addPendingLemma(const SimpleTheoryLemma& lemma,
                                       bool isWaiting)
{
  addPendingLemma(
      std::unique_ptr<SimpleTheoryLemma>(new SimpleTheoryLemma(lemma)),
      isWaiting);
}
void InferenceManager::addPendingLemma(const Node& lemma,
                                       InferenceId inftype,
                                       ProofGenerator* pg,
                                       bool isWaiting,
                                       LemmaProperty p)
{
  addPendingLemma(std::unique_ptr<SimpleTheoryLemma>(
                      new SimpleTheoryLemma(inftype, lemma, p, pg)),
                  isWaiting);
}

void InferenceManager::flushWaitingLemmas()
{
  for (auto& lem : d_waitingLem)
  {
    Trace("arith::infman") << "Flush waiting lemma to pending: " << lem->getId()
                           << " " << lem->d_node << std::endl;
    d_pendingLem.emplace_back(std::move(lem));
  }
  d_waitingLem.clear();
}
void InferenceManager::clearWaitingLemmas() { d_waitingLem.clear(); }

bool InferenceManager::hasUsed() const { return hasSent() || hasPending(); }

bool InferenceManager::hasWaitingLemma() const { return !d_waitingLem.empty(); }

std::size_t InferenceManager::numWaitingLemmas() const
{
  return d_waitingLem.size();
}

bool InferenceManager::hasCachedLemma(TNode lem, LemmaProperty p)
{
  Node rewritten = rewrite(lem);
  return TheoryInferenceManager::hasCachedLemma(rewritten, p);
}

bool InferenceManager::cacheLemma(TNode lem, LemmaProperty p)
{
  Node rewritten = rewrite(lem);
  return TheoryInferenceManager::cacheLemma(rewritten, p);
}

bool InferenceManager::propagateLit(TNode lit)
{
  if (d_trackPropLits)
  {
    d_propLits.insert(lit);
  }
  return TheoryInferenceManager::propagateLit(lit);
}

bool InferenceManager::hasPropagated(TNode lit) const
{
  Assert(d_trackPropLits);
  return d_propLits.find(lit) != d_propLits.end();
}

}  // namespace arith
}  // namespace theory
}  // namespace ava6::internal
