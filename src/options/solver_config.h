#ifndef AVA6__OPTIONS__SOLVER_CONFIG_H
#define AVA6__OPTIONS__SOLVER_CONFIG_H

#include <cstdint>
#include "options/managed_streams.h"

namespace ava6::internal::options {

// These are implementation states, not command-line or SMT-LIB options.
enum class PrenexQuantMode { NONE, SIMPLE };
enum class FmfMbqiMode { NONE, FMC };
enum class ProofMode { OFF, PP_ONLY, FULL_STRICT };
enum class UnsatCoresMode { OFF, SAT_PROOF, ASSUMPTIONS };
enum class ParsingMode { DEFAULT, STRICT, LENIENT };
enum class TheoryOfMode { THEORY_OF_TYPE_BASED, THEORY_OF_TERM_BASED };

/** Derived by SetDefaults from the logic and supported public options.
 * Copies retain the configuration when constructing an internal subsolver.
 * Fixed algorithm choices belong in their implementation, not in this struct.
 */
struct SolverConfig
{
  /** Pivot budgets selected for difference logic versus general arithmetic. */
  int64_t arithHeuristicPivots = 0;
  /** Variable-order pivot budget, derived from arithmetic logic. */
  int64_t arithStandardCheckVarOrderPivots = -1;
  /** Pivot threshold, increased for difference logic. */
  uint64_t arithPivotThreshold = 2;
  /** Diophantine solving is disabled for quantifier-free nonlinear arithmetic. */
  bool arithDioSolver = true;
  /** Bound inference is enabled for quantifier-free arithmetic. */
  bool nlRlvAssertBounds = false;
  /** Prenexing is disabled for bounded integer quantification. */
  PrenexQuantMode prenexQuant = PrenexQuantMode::SIMPLE;
  /** Nested pre-skolemization requires UF in the logic. */
  bool preSkolemQuantNested = true;
  /** Pure quantified bit-vectors use CEGQI at full effort. */
  bool cegqiFullEffort = false;
  /** Model checking policy used by bounded quantifier enumeration. */
  FmfMbqiMode fmfMbqiMode = FmfMbqiMode::FMC;
  /** Eager bit-blasting may require Ackermannization. */
  bool ackermann = false;
  /** Disabled in subsolvers used to evaluate model expressions. */
  bool modelVarElimUneval = true;
  /** Proof bookkeeping required by produce-proofs or unsat cores. */
  ProofMode proofMode = ProofMode::OFF;
  /** Cores come from full proofs when available, otherwise SAT assumptions. */
  UnsatCoresMode unsatCoresMode = UnsatCoresMode::OFF;
  /** Repeat nonclausal simplification for QF_AUFBV when cores permit it. */
  bool repeatSimp = false;
  /** Linear array optimization is disabled when producing models. */
  bool arraysOptimizeLinear = true;
  /** Complete proof checking depends on check-proofs and its granularity. */
  bool checkProofsComplete = false;
  /** Term ownership is selected from the combination of theories. */
  TheoryOfMode theoryOfMode = TheoryOfMode::THEORY_OF_TYPE_BASED;
};

/** Streams and parser/driver state set by the public option handlers. */
struct InputOutputConfig
{
  ManagedErr err;
  ManagedIn in;
  ManagedOut out;
  bool showTraceTags = false;
  ParsingMode parsingMode = ParsingMode::DEFAULT;
};

}  // namespace ava6::internal::options
#endif
