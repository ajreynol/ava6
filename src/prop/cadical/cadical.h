/******************************************************************************
 * Top contributors (to current version):
 *   Mathias Preiner, Aina Niemetz, Andrew Reynolds
 *
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

#include "ava6_private.h"

#ifndef AVA6__PROP__CADICAL_H
#define AVA6__PROP__CADICAL_H

#include "context/cdhashset.h"
#include "prop/sat_clause_sink.h"
#include "proof/proof_node.h"
#include "util/statistics_registry.h"
#include "smt/env_obj.h"

namespace CaDiCaL {
class Solver;
class Terminator;
}  // namespace CaDiCaL

namespace ava6::internal::prop {

namespace cadical {
class CadicalPropagator;
class ProofTracer;
}  // namespace cadical
class TheoryProxy;

class CadicalSolver : public SatClauseSink, protected EnvObj
{
 public:
  CadicalSolver(Env& env, TheoryProxy& theoryProxy, const std::string& name = "");
  ~CadicalSolver() override;

  bool addClause(const SatClause& clause, bool removable) override;
  SatVariable newVar(bool isTheoryAtom) override;
  SatVariable trueVar() override;
  SatVariable falseVar() override;

  SatValue solve(const std::vector<SatLiteral>& assumptions = {});
  void getUnsatAssumptions(std::vector<SatLiteral>& assumptions);
  void interrupt();
  SatValue value(SatLiteral l);
  SatValue modelValue(SatLiteral l);

  uint32_t getAssertionLevel() const;
  void push();
  void pop();
  void resetTrail();
  /** Prefer this literal's polarity for decisions made by CaDiCaL. */
  void preferPhase(SatLiteral lit);
  bool isDecision(SatVariable var) const;
  bool isFixed(SatVariable var) const;
  std::vector<SatLiteral> getDecisions() const;
  /** Get the refutation reconstructed from CaDiCaL proof tracing. */
  std::shared_ptr<ProofNode> getProof();

 private:
  void initialize();
  void setResourceLimit(ResourceManager* resmgr);

  /** The wrapped CaDiCaL instance. */
  std::unique_ptr<CaDiCaL::Solver> d_solver;
  /** The CaDiCaL terminator (for termination via resource manager). */
  std::unique_ptr<CaDiCaL::Terminator> d_terminator;

  /** Context for synchronizing the SAT solver. */
  context::Context* d_context = nullptr;
  /** The associated theory proxy . */
  TheoryProxy* const d_proxy;
  /** The CaDiCaL propagator . */
  std::unique_ptr<cadical::CadicalPropagator> d_propagator;
  /** Proof tracer instance for extracting unsat cores. */
  std::unique_ptr<cadical::ProofTracer> d_proof_tracer;

  /**
   * Stores the current set of assumptions provided via solve() and is used to
   * query the solver if a given assumption is false.
   */
  std::vector<SatLiteral> d_assumptions;
  /** Next fresh SAT variable index. */
  unsigned d_nextVarIdx;
  /**
   * Whether we are in SAT mode. If true, the SAT solver returned satisfiable
   * and we are allowed to query model values from the solver.
   */
  bool d_inSatMode;
  /** The variable representing true. */
  SatVariable d_true;
  /** The variable representing false. */
  SatVariable d_false;

  struct Statistics
  {
    IntStat d_numSatCalls;
    IntStat d_numVariables;
    IntStat d_numClauses;
    TimerStat d_solveTime;
    Statistics(StatisticsRegistry& registry, const std::string& prefix);
  };

  Statistics d_statistics;
};

}  // namespace ava6::internal::prop

#endif  // AVA6__PROP__CADICAL_H
