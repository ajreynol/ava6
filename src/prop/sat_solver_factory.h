/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * SAT Solver creation facility
 */

#include "ava6_private.h"

#ifndef AVA6__PROP__SAT_SOLVER_FACTORY_H
#define AVA6__PROP__SAT_SOLVER_FACTORY_H

#include <string>

#include "options/prop_options.h"
#include "prop/sat_solver.h"
#include "smt/env.h"
#include "util/resource_manager.h"

namespace ava6::internal {
namespace prop {

class TheoryProxy;

class SatSolverFactory
{
 public:

  using CDCLTFactory = CDCLTSatSolver* (*)(Env&,
                                           StatisticsRegistry&,
                                           ResourceManager*,
                                           TheoryProxy*,
                                           const std::string&);


  template <options::SatSolverMode T>
  static CDCLTSatSolver* createCDCLTSatSolver(Env& env,
                                              StatisticsRegistry& registry,
                                              ResourceManager* resmgr,
                                              TheoryProxy* theory_proxy,
                                              const std::string& name = "");

  static CDCLTFactory getFactory(options::SatSolverMode);
};

template <>
CDCLTSatSolver*
SatSolverFactory::createCDCLTSatSolver<options::SatSolverMode::MINISAT>(
    Env&,
    StatisticsRegistry&,
    ResourceManager*,
    TheoryProxy*,
    const std::string&);

template <>
CDCLTSatSolver*
SatSolverFactory::createCDCLTSatSolver<options::SatSolverMode::CADICAL>(
    Env&,
    StatisticsRegistry&,
    ResourceManager*,
    TheoryProxy*,
    const std::string&);

}  // namespace prop
}  // namespace ava6::internal

#endif  // AVA6__PROP__SAT_SOLVER_FACTORY_H
