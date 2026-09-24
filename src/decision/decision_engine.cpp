/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Decision engine.
 */
#include "decision/decision_engine.h"

#include "util/resource_manager.h"

namespace ava6::internal {
namespace decision {

DecisionEngine::DecisionEngine(Env& env,
                               prop::CadicalSolver* ss,
                               prop::CnfStream* cs)
    : EnvObj(env), d_satSolver(ss), d_cnfStream(cs)
{
}

prop::SatLiteral DecisionEngine::getNext(bool& stopSearch)
{
  resourceManager()->spendResource(Resource::DecisionStep);
  return getNextInternal(stopSearch);
}

DecisionEngineEmpty::DecisionEngineEmpty(Env& env)
    : DecisionEngine(env, nullptr, nullptr)
{
}
bool DecisionEngineEmpty::isDone() { return false; }
void DecisionEngineEmpty::addAssertions(
    AVA6_UNUSED const std::vector<TNode>& lems)
{
}
prop::SatLiteral DecisionEngineEmpty::getNextInternal(
    AVA6_UNUSED bool& stopSearch)
{
  return prop::undefSatLiteral;
}

}  // namespace decision
}  // namespace ava6::internal
