/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The solver for SMT queries in an SolverEngine.
 */

#include "smt/smt_driver.h"

#include "options/base_options.h"
#include "options/main_options.h"
#include "options/smt_options.h"
#include "prop/prop_engine.h"
#include "smt/context_manager.h"
#include "smt/env.h"
#include "smt/logic_exception.h"
#include "smt/smt_solver.h"

namespace ava6::internal {
namespace smt {

SmtDriver::SmtDriver(Env& env, SmtSolver& smt, ContextManager* ctx)
    : EnvObj(env), d_smt(smt), d_ctx(ctx), d_ap(env), d_assertionListIndex(userContext(), 0)
{
  // set up proofs, this is done after options are finalized, so the
  // preprocess proof has been setup
  PreprocessProofGenerator* pppg =
      d_smt.getPreprocessor()->getPreprocessProofGenerator();
  if (pppg != nullptr)
  {
    d_ap.enableProofs(pppg);
  }
}

Result SmtDriver::checkSat(const std::vector<Node>& assumptions)
{
  bool hasAssumptions = !assumptions.empty();
  if (d_ctx)
  {
    d_ctx->notifyCheckSat(hasAssumptions);
  }
  Assertions& as = d_smt.getAssertions();
  Result result;
  try
  {
    as.setAssumptions(assumptions);

    Trace("smt") << "SmtSolver::check()" << std::endl;

    ResourceManager* rm = d_env.getResourceManager();
    // if we are already out of (cumulative) resources
    if (rm->out())
    {
      UnknownExplanation why = rm->outOfResources()
                                   ? UnknownExplanation::RESOURCEOUT
                                   : UnknownExplanation::TIMEOUT;
      result = Result(Result::UNKNOWN, why);
    }
    else
    {
      getNextAssertions(d_ap);
      result = checkSatInternal(d_ap);
    }
  }
  catch (const LogicException& e)
  {
    // The exception may have been throw during solving, backtrack to reset the
    // decision level to the level expected after this method finishes
    d_smt.getPropEngine()->resetTrail();
    throw;
  }
  catch (const TypeCheckingExceptionPrivate& e)
  {
    // The exception has been throw during solving, backtrack to reset the
    // decision level to the level expected after this method finishes. Note
    // that we do not expect type checking exceptions to occur during solving.
    // However, if they occur due to a bug, we don't want to additionally cause
    // an assertion failure.
    d_smt.getPropEngine()->resetTrail();
    throw;
  }
  if (d_ctx)
  {
    d_ctx->notifyCheckSatResult(hasAssumptions);
  }
  return result;
}

void SmtDriver::refreshAssertions()
{
  // get the next assertions, store in d_ap
  getNextAssertions(d_ap);
  // preprocess
  d_smt.preprocess(d_ap);
  // assert to internal
  d_smt.assertToInternal(d_ap);
}

void SmtDriver::notifyPushPre()
{
  // must preprocess the assertions and push them to the SAT solver, to make
  // the state accurate prior to pushing
  refreshAssertions();
}

void SmtDriver::notifyPushPost() { d_smt.pushPropContext(); }

void SmtDriver::notifyPopPre() { d_smt.popPropContext(); }

void SmtDriver::notifyPostSolve() { d_smt.resetTrail(); }

Result SmtDriver::checkSatInternal(preprocessing::AssertionPipeline& ap)
{
  // preprocess
  d_smt.preprocess(ap);

  if (options().base.preprocessOnly)
  {
    return Result(Result::UNKNOWN, UnknownExplanation::REQUIRES_FULL_CHECK);
  }

  // assert to internal
  d_smt.assertToInternal(ap);
  return d_smt.checkSatInternal();
}

void SmtDriver::getNextAssertions(
    preprocessing::AssertionPipeline& ap)
{
  ap.clear();
  // Refresh global declarations before collecting pending assertions.
  d_smt.getAssertions().refresh();
  Assertions& as = d_smt.getAssertions();
  const context::CDList<Node>& al = as.getAssertionList();
  size_t alsize = al.size();
  for (size_t i = d_assertionListIndex.get(); i < alsize; ++i)
  {
    ap.push_back(al[i], true);
  }
  d_assertionListIndex = alsize;
}

}  // namespace smt
}  // namespace ava6::internal
