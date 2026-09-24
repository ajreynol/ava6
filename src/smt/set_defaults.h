/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Method for setting the default options of an SMT engine.
 */

#ifndef AVA6__SMT__SET_DEFAULTS_H
#define AVA6__SMT__SET_DEFAULTS_H

#include "options/options.h"
#include "smt/env_obj.h"
#include "theory/logic_info.h"

namespace ava6::internal {
namespace smt {

/**
 * Class responsible for setting default options, which includes managing
 * implied options and dependencies between the options and the logic.
 */
class SetDefaults : protected EnvObj
{
 public:
  /**
   * @param isInternalSubsolver Whether we are setting the options for an
   * internal subsolver (see SolverEngine::isInternalSubsolver).
   */
  SetDefaults(Env& env, bool isInternalSubsolver);
  /**
   * The purpose of this method is to set the default options and update the
   * logic info for an SMT engine.
   *
   * @param logic A reference to the logic of SolverEngine, which can be
   * updated by this method based on the current options and the logic itself.
   * @param opts The options to modify, typically the main options of the
   * SolverEngine in scope.
   */
  void setDefaults(LogicInfo& logic, Options& opts);

  /**
   * Disable checking for the given options. This is typically done for
   * the options that are passed to subsolvers.
   *
   * This disables basic checking options that is run for check-sat calls,
   * including checkProofs, checkUnsatCores, checkModels. It also disables
   * produceProofs.
   *
   * It does not disable more advanced checking (e.g. checkSynthSol,
   * checkAbduct, etc.), since these features are not exercised on subsolvers.
   */
  static void disableChecking(Options& opts);

 private:
  //------------------------- utility methods
  /**
   * Determine whether we will be solving a SyGuS problem.
   */
  /**
   * Determine whether we will be using SyGuS.
   */
  /**
   * Check if incompatible with incremental mode. Notice this method may modify
   * the options to ensure that we are compatible with incremental mode.
   *
   * If this method returns true, then the reason why we were incompatible with
   * incremental mode is written on the reason output stream. Suggestions for
   * how to resolve the incompatibility exception are written on the suggest
   * stream.
   */
  bool incompatibleWithIncremental(const LogicInfo& logic,
                                   Options& opts,
                                   std::ostream& reason,
                                   std::ostream& suggest) const;
  /**
   * Check if incompatible with quantified formulas. Notice this method may
   * modify the options to ensure that we are compatible with quantified logics.
   * The output stream reason is similar to above.
   */
  bool incompatibleWithQuantifiers(const Options& opts,
                                   std::ostream& reason) const;
  //------------------------- options setting, prior finalization of logic
  /**
   * Set defaults pre, which sets all options prior to finalizing the logic.
   * It is required that any options that impact the finalization of logic
   * (finalizeLogic).
   */
  void setDefaultsPre(Options& opts);
  //------------------------- finalization of the logic
  /**
   * Finalize the logic based on the options.
   */
  void finalizeLogic(LogicInfo& logic, Options& opts) const;
  /**
   * Widen logic to theories that are required, since some theories imply the
   * use of other theories to handle certain operators, e.g. UF to handle
   * partial functions.
   */
  void widenLogic(LogicInfo& logic, const Options& opts) const;
  //------------------------- options setting, post finalization of logic
  /**
   * Set all default options, after we have finalized the logic.
   */
  void setDefaultsPost(const LogicInfo& logic, Options& opts) const;
  /**
   * Set defaults related to quantifiers, called when quantifiers are enabled.
   * This method modifies opt.quantifiers only.
   */
  void setDefaultsQuantifiers(const LogicInfo& logic, Options& opts) const;
  /**
   * Set defaults related to SyGuS, called when SyGuS is enabled.
   */
  /**
   * Set default decision mode
   */
  void setDefaultDecisionMode(const LogicInfo& logic, Options& opts) const;
  /** Notify that we are modifying option x to val due to reason. */
  void notifyModifyOption(const std::string& x,
                          const std::string& val,
                          const std::string& reason) const;
  /** Are we an internal subsolver? */
  bool d_isInternalSubsolver;
};

}  // namespace smt
}  // namespace ava6::internal

#endif /* AVA6__SMT__SET_DEFAULTS_H */
