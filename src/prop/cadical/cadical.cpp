/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Wrapper for CaDiCaL SAT Solver.
 *
 * Implementation of the CaDiCaL SAT solver for ava6 (bit-vectors).
 */

#include "prop/cadical/cadical.h"

#include <cadical/cadical.hpp>
#include <cstdint>
#include <memory>

#include "base/check.h"
#include "options/base_options.h"
#include "options/main_options.h"
#include "options/proof_options.h"
#include "prop/cadical/cdclt_propagator.h"
#include "prop/cadical/proof_tracer.h"
#include "prop/cadical/util.h"
#include "prop/sat_solver_types.h"
#include "prop/theory_proxy.h"
#include "theory/shared_terms_database.h"
#include "util/resource_manager.h"
#include "util/statistics_registry.h"
#include "util/string.h"

namespace ava6::internal::prop {
using namespace cadical;

/* -------------------------------------------------------------------------- */


CadicalSolver::CadicalSolver(Env& env,
                             TheoryProxy& theoryProxy,
                             const std::string& name)
    : EnvObj(env),
      d_solver(new CaDiCaL::Solver()),
      d_context(context()),
      d_proxy(&theoryProxy),
      // Note: CaDiCaL variables start with index 1 rather than 0 since negated
      //       literals are represented as the negation of the index.
      d_nextVarIdx(1),
      d_inSatMode(false),
      d_true(undefSatVariable),
      d_false(undefSatVariable),
      d_statistics(statisticsRegistry(), name)
{
  setResourceLimit(resourceManager());
  initialize();
}

void CadicalSolver::initialize()
{
  d_propagator.reset(new CadicalPropagator(
      d_proxy,
      d_context,
      *d_solver,
      statisticsRegistry(),
      d_env.isTheoryProofProducing()));
  if (d_env.isSatProofProducing())
  {
    d_proof_tracer.reset(new ProofTracer(*d_propagator));
    d_solver->connect_proof_tracer(d_proof_tracer.get(), true);
  }

  d_solver->set("quiet", 1);  // CaDiCaL is verbose by default

  // walk and lucky phase do not use the external propagator, disable for now
  d_solver->set("walk", 0);
  d_solver->set("lucky", 0);
  // ilb currently does not play well with user propagators
  d_solver->set("ilb", 0);
  d_solver->set("ilbassumptions", 0);
  d_solver->connect_fixed_listener(d_propagator.get());
  d_solver->connect_external_propagator(d_propagator.get());


  d_true = newVar(false);
  d_false = newVar(false);
  d_solver->clause(toCadicalVar(d_true));
  d_solver->clause(-toCadicalVar(d_false));
}

CadicalSolver::~CadicalSolver()
{
  if (d_proof_tracer != nullptr)
  {
    d_solver->disconnect_proof_tracer(d_proof_tracer.get());
  }
}

/**
 * Terminator class that notifies CaDiCaL to terminate when the resource limit
 * is reached (used for resource limits specified via --rlimit or --tlimit).
 */
class ResourceLimitTerminator : public CaDiCaL::Terminator
{
 public:
  ResourceLimitTerminator(ResourceManager& resmgr) : d_resmgr(resmgr) {}

  bool terminate() override
  {
    d_resmgr.spendResource(Resource::BvSatStep);
    return d_resmgr.out();
  }

 private:
  ResourceManager& d_resmgr;
};

void CadicalSolver::setResourceLimit(ResourceManager* resmgr)
{
  d_terminator.reset(new ResourceLimitTerminator(*resmgr));
  d_solver->connect_terminator(d_terminator.get());
}

SatValue CadicalSolver::solve(const std::vector<SatLiteral>& assumptions)
{
  Trace("cadical::propagator") << "solve start" << std::endl;
  d_propagator->renotify_fixed();

  TimerStat::CodeTimer codeTimer(d_statistics.d_solveTime);
  d_assumptions.clear();
  // Assume activation literals for all active user levels.
  for (const auto& lit : d_propagator->activation_literals())
  {
    Trace("cadical::propagator")
        << "assume activation lit: " << ~lit << std::endl;
    d_solver->assume(toCadicalLit(~lit));
  }

  for (const SatLiteral& lit : assumptions)
  {
    Trace("cadical::propagator") << "assume: " << lit << std::endl;

    d_solver->assume(toCadicalLit(lit));
    d_assumptions.push_back(lit);
  }
  d_propagator->in_search(true);

  const SatValue res =
      toSatValue(d_solver->solve());
  Assert(res != SAT_VALUE_TRUE || d_propagator->done());
  Trace("cadical::propagator") << "solve done: " << res << std::endl;
  d_propagator->in_search(false);

  ++d_statistics.d_numSatCalls;
  d_inSatMode = (res == SAT_VALUE_TRUE);
  return res;
}


bool CadicalSolver::addClause(const SatClause& clause, bool removable)
{
  if (TraceIsOn("cadical::propagator"))
  {
    Trace("cadical::propagator") << "addClause (" << removable << "):";
    SatLiteral alit = d_propagator->current_activation_lit();
    if (alit != undefSatLiteral)
    {
      Trace("cadical::propagator") << " " << alit;
    }
    for (const SatLiteral& lit : clause)
    {
      Trace("cadical::propagator") << " " << lit;
    }
    Trace("cadical::propagator") << " 0" << std::endl;
  }
  d_propagator->add_clause(clause, removable);
  ++d_statistics.d_numClauses;
  return true;
}

SatVariable CadicalSolver::newVar(bool isTheoryAtom)
{
  ++d_statistics.d_numVariables;
  d_propagator->add_new_var(d_nextVarIdx, isTheoryAtom);

  return d_nextVarIdx++;
}

SatVariable CadicalSolver::trueVar() { return d_true; }

SatVariable CadicalSolver::falseVar() { return d_false; }


void CadicalSolver::getUnsatAssumptions(std::vector<SatLiteral>& assumptions)
{
  for (const SatLiteral& lit : d_assumptions)
  {
    if (d_solver->failed(toCadicalLit(lit)))
    {
      assumptions.push_back(lit);
    }
  }
}

void CadicalSolver::interrupt() { d_solver->terminate(); }

SatValue CadicalSolver::value(SatLiteral l) { return d_propagator->value(l); }

SatValue CadicalSolver::modelValue(SatLiteral l)
{
  Assert(d_inSatMode);
  auto val = d_solver->val(toCadicalVar(l.getSatVariable()));
  return toSatValueLit(l.isNegated() ? -val : val);
}

CadicalSolver::Statistics::Statistics(StatisticsRegistry& registry,
                                      const std::string& prefix)
    : d_numSatCalls(registry.registerInt(prefix + "cadical::calls_to_solve")),
      d_numVariables(registry.registerInt(prefix + "cadical::variables")),
      d_numClauses(registry.registerInt(prefix + "cadical::clauses")),
      d_solveTime(registry.registerTimer(prefix + "cadical::solve_time"))
{
}


void CadicalSolver::push()
{
  d_context->push();  // SAT context for ava6
  // Push new user level
  d_propagator->user_push();
  // Set new activation literal for pushed user level
  // Note: This happens after the push to ensure that the activation literal's
  // introduction level is the current user level.
  SatVariable alit = newVar(false);
  d_propagator->set_activation_lit(alit);
}

uint32_t CadicalSolver::getAssertionLevel() const
{
  Assert(d_propagator);
  return d_propagator->current_user_level();
}

void CadicalSolver::pop()
{
  d_context->pop();  // SAT context for ava6
  d_propagator->user_pop();
  // CaDiCaL issues notify_backtrack(0) when done, we don't have to call this
  // explicitly here
}

void CadicalSolver::resetTrail()
{
  // Reset SAT context to decision level 0
  d_propagator->notify_backtrack(0);
}

void CadicalSolver::preferPhase(SatLiteral lit)
{
  Trace("cadical::propagator") << "phase: " << lit << std::endl;
  d_propagator->phase(lit);
}

bool CadicalSolver::isDecision(SatVariable var) const
{
  return d_solver->is_decision(toCadicalVar(var));
}

bool CadicalSolver::isFixed(SatVariable var) const
{
  return d_propagator->is_fixed(var);
}

std::vector<SatLiteral> CadicalSolver::getDecisions() const
{
  std::vector<SatLiteral> decisions;
  for (SatLiteral lit : d_propagator->get_decisions())
  {
    if (lit != undefSatLiteral)
    {
      decisions.push_back(lit);
    }
  }
  return decisions;
}

std::shared_ptr<ProofNode> CadicalSolver::getProof()
{
  if (d_proof_tracer)
  {
    ProofNodeManager* pnm = d_env.getProofNodeManager();
    NodeManager* nm = d_env.getNodeManager();
    return d_proof_tracer->get_chain_resolution_proof(pnm, nm, d_proxy);
  }
  return nullptr;
}

/* -------------------------------------------------------------------------- */
}  // namespace ava6::internal::prop
