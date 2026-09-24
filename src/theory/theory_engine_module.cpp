/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Virtual class for theory engine modules
 */

#include "theory/theory_engine_module.h"

namespace ava6::internal {
namespace theory {

size_t TheoryEngineModule::d_idCounter = 0;
TheoryEngineModule::TheoryEngineModule(Env& env,
                                       TheoryEngine* engine,
                                       const std::string& name)
    : EnvObj(env), d_out(statisticsRegistry(), engine, name, d_idCounter)
{
  // increment the id counter so that the id of this module is unique
  d_idCounter++;
}

void TheoryEngineModule::presolve() {}

void TheoryEngineModule::postsolve(AVA6_UNUSED prop::SatValue result) {}

void TheoryEngineModule::check(AVA6_UNUSED Theory::Effort effort) {}

void TheoryEngineModule::postCheck(AVA6_UNUSED Theory::Effort effort) {}

void TheoryEngineModule::notifyLemma(
    AVA6_UNUSED TNode n,
    AVA6_UNUSED InferenceId id,
    AVA6_UNUSED LemmaProperty p,
    AVA6_UNUSED const std::vector<Node>& skAsserts,
    AVA6_UNUSED const std::vector<Node>& sks)
{
}

bool TheoryEngineModule::needsCandidateModel() { return false; }

void TheoryEngineModule::notifyCandidateModel(AVA6_UNUSED TheoryModel* m) {}

TheoryId TheoryEngineModule::getId() const { return d_out.getId(); }

}  // namespace theory
}  // namespace ava6::internal
