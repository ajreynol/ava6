/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * SAT Solver creation facility.
 */

#include "prop/sat_solver_factory.h"

#include "prop/cadical/cadical.h"
#include "prop/minisat/minisat.h"

namespace ava6::internal {
namespace prop {



SatSolverFactory::CDCLTFactory SatSolverFactory::getFactory(
    const options::SatSolverMode mode)
{
  using options::SatSolverMode;
  switch (mode)
  {
    case SatSolverMode::CADICAL:
      return createCDCLTSatSolver<SatSolverMode::CADICAL>;
    case SatSolverMode::MINISAT:
      return createCDCLTSatSolver<SatSolverMode::MINISAT>;
    default: Unreachable(); return nullptr;
  }
}

template <>
CDCLTSatSolver*
SatSolverFactory::createCDCLTSatSolver<options::SatSolverMode::MINISAT>(
    Env& env,
    StatisticsRegistry& registry,
    AVA6_UNUSED ResourceManager* resmgr,
    TheoryProxy* theory_proxy,
    AVA6_UNUSED const std::string& name)
{
  MinisatSatSolver* res = new MinisatSatSolver(env, registry);
  res->initialize(theory_proxy);
  return res;
}

template <>
CDCLTSatSolver*
SatSolverFactory::createCDCLTSatSolver<options::SatSolverMode::CADICAL>(
    Env& env,
    StatisticsRegistry& registry,
    ResourceManager* resmgr,
    TheoryProxy* theory_proxy,
    const std::string& name)
{
  CadicalSolver* res = new CadicalSolver(env, registry, name);
  res->setResourceLimit(resmgr);
  res->initialize(theory_proxy);
  return res;
}

}  // namespace prop
}  // namespace ava6::internal
