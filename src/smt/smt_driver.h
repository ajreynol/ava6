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

#include "ava6_private.h"

#ifndef AVA6__SMT__SMT_DRIVER_H
#define AVA6__SMT__SMT_DRIVER_H

#include <vector>

#include "expr/node.h"
#include "preprocessing/assertion_pipeline.h"
#include "smt/assertions.h"
#include "smt/env_obj.h"
#include "util/result.h"

namespace ava6::internal {
namespace smt {

class SmtSolver;
class ContextManager;

/** Coordinates preprocessing, solving, and incremental contexts for SMT queries. */
class SmtDriver : protected EnvObj
{
 public:
  SmtDriver(Env& env, SmtSolver& smt, ContextManager* ctx);
  ~SmtDriver() = default;
  /**
   * Check satisfiability. This invokes the algorithm given by this driver
   * for checking satisfiability.
   *
   * @param assumptions The assumptions for this check-sat call, which are
   * temporary assertions.
   */
  Result checkSat(const std::vector<Node>& assumptions);

  /**
   * Refresh the assertions that have been asserted to the underlying SMT
   * solver. This gets the set of unprocessed assertions of the underlying
   * SMT solver, preprocesses them, pushes them into the SMT solver.
   *
   * We ensure that assertions are refreshed eagerly during user pushes to
   * ensure that assertions are only preprocessed in one context.
   */
  void refreshAssertions();
  // --------------------------------------- callbacks from the context manager
  /**
   * Notify push pre, which is called just before the user context of the state
   * pushes. This processes all pending assertions.
   */
  void notifyPushPre();
  /**
   * Notify push post, which is called just after the user context of the state
   * pushes. This performs a push on the underlying prop engine.
   */
  void notifyPushPost();
  /**
   * Notify pop pre, which is called just before the user context of the state
   * pops. This performs a pop on the underlying prop engine.
   */
  void notifyPopPre();
  /**
   * Notify post solve, which is called once per check-sat query. It is
   * triggered when the first d_state.doPendingPops() is issued after the
   * check-sat. This calls the postsolve method of the underlying TheoryEngine.
   */
  void notifyPostSolve();
  // ----------------------------------- end callbacks from the context manager
 private:
  /** Refresh and collect the assertions not yet sent to the solver. */
  void getNextAssertions(preprocessing::AssertionPipeline& ap);
  /** Preprocess and perform one satisfiability check. */
  Result checkSatInternal(preprocessing::AssertionPipeline& ap);
  /** The underlying SMT solver */
  SmtSolver& d_smt;
  /** Context callbacks, provided for incremental solving. */
  ContextManager* d_ctx;
  /** assertions pipeline */
  preprocessing::AssertionPipeline d_ap;
  /**
   * The first index in the assertion list of the underlying SMT solver that we
   * have not processed yet. The call to getNextAssertions gets all assertions
   * starting from this index onward.
   */
  context::CDO<size_t> d_assertionListIndex;
};

}  // namespace smt
}  // namespace ava6::internal

#endif
