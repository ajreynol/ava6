/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Management of a care graph based approach for theory combination.
 */

#include "theory/combination_engine.h"

#include "prop/prop_engine.h"
#include "proof/eager_proof_generator.h"
#include "theory/care_graph.h"
#include "theory/ee_manager.h"
#include "theory/model_manager.h"
#include "theory/shared_solver.h"
#include "theory/theory_engine.h"

namespace ava6::internal {
namespace theory {

CombinationEngine::CombinationEngine(Env& env,
                                     TheoryEngine& te,
                                     const std::vector<Theory*>& paraTheories)
    : EnvObj(env),
      d_te(te),
      d_valuation(&te),
      d_paraTheories(paraTheories),
      d_eemanager(nullptr),
      d_mmanager(nullptr),
      d_sharedSolver(nullptr),
      d_cmbsPg(env.isTheoryProofProducing()
                   ? new EagerProofGenerator(env, env.getUserContext())
                   : nullptr)
{
  d_sharedSolver.reset(new SharedSolver(env, d_te));
  d_eemanager.reset(new EqEngineManager(env, d_te, *d_sharedSolver));
  d_mmanager.reset(new ModelManager(env, d_te));
}

CombinationEngine::~CombinationEngine() {}

void CombinationEngine::finishInit()
{
  Assert(d_eemanager != nullptr);

  // initialize equality engines in all theories, including quantifiers engine
  // and the (provided) shared solver
  d_eemanager->initializeTheories();

  Assert(d_mmanager != nullptr);
  d_mmanager->finishInit();
}

void CombinationEngine::resetModel() { d_mmanager->resetModel(); }

void CombinationEngine::postProcessModel(bool incomplete)
{
  // postprocess with the model
  d_mmanager->postProcessModel(incomplete);
}

theory::TheoryModel* CombinationEngine::getModel()
{
  return d_mmanager->getModel();
}

SharedSolver* CombinationEngine::getSharedSolver()
{
  return d_sharedSolver.get();
}
bool CombinationEngine::isProofEnabled() const { return d_cmbsPg != nullptr; }

void CombinationEngine::combineTheories()
{
  Trace("combineTheories") << "TheoryEngine::combineTheories()" << std::endl;

  // Care graph we'll be building
  CareGraph careGraph;

  // get the care graph from the parametric theories
  for (Theory* t : d_paraTheories)
  {
    t->getCareGraph(&careGraph);
  }

  Trace("combineTheories")
      << "TheoryEngine::combineTheories(): care graph size = "
      << careGraph.size() << std::endl;

  // Now add splitters for the ones we are interested in
  prop::PropEngine* propEngine = d_te.getPropEngine();
  for (const CarePair& carePair : careGraph)
  {
    Trace("combineTheories")
        << "TheoryEngine::combineTheories(): checking " << carePair.d_a << " = "
        << carePair.d_b << " from " << carePair.d_theory << std::endl;

    // The equality in question (order for no repetition)
    Node equality = carePair.d_a.eqNode(carePair.d_b);

    // We need to split on it
    Trace("combineTheories")
        << "TheoryEngine::combineTheories(): requesting a split " << std::endl;

    TrustNode tsplit;
    if (isProofEnabled())
    {
      // make proof of splitting lemma
      tsplit = d_cmbsPg->mkTrustNodeSplit(equality);
    }
    else
    {
      Node split = equality.orNode(equality.notNode());
      tsplit = TrustNode::mkTrustLemma(split, nullptr);
    }
    d_sharedSolver->sendLemma(
        tsplit, carePair.d_theory, InferenceId::COMBINATION_SPLIT);

    // Could check the equality status here:
    //   EqualityStatus es = getEqualityStatus(carePair.d_a, carePair.d_b);
    // and only require true phase below if:
    //   es == EQUALITY_TRUE || es == EQUALITY_TRUE_IN_MODEL
    // and require false phase below if:
    //   es == EQUALITY_FALSE_IN_MODEL
    // This is supposed to force preference to follow what the theory models
    // already have but it doesn't seem to make a big difference - need to
    // explore more -Clark
    Node e = d_valuation.ensureLiteral(equality);
    propEngine->preferPhase(e, true);
  }
}

bool CombinationEngine::buildModel()
{
  // building the model happens as a separate step
  return d_mmanager->buildModel();
}

}  // namespace theory
}  // namespace ava6::internal
