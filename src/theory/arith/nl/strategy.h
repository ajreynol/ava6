/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Strategies for the nonlinear extension.
 */

#ifndef AVA6__THEORY__ARITH__NL__STRATEGY_H
#define AVA6__THEORY__ARITH__NL__STRATEGY_H

#include <iosfwd>
#include <vector>

#include "options/options.h"

namespace ava6::internal {
namespace theory {
namespace arith {
namespace nl {

/** The possible inference steps for the nonlinear extension */
enum class InferStep
{

  /** Break if any lemma is pending */
  BREAK,
  /** Flush waiting lemmas to be pending */
  FLUSH_WAITING_LEMMAS,

  /** Initialize the NL solver */
  NL_INIT,
  /** Nl factoring lemmas */
  NL_FACTORING,
  /** Nl monomial equality propagation by flattening */
  NL_FLATTEN_MON,
  /** Nl lemmas for monomial bound inference */
  NL_MONOMIAL_INFER_BOUNDS,
  /** Nl lemmas for monomial magnitudes (class 0) */
  NL_MONOMIAL_MAGNITUDE0,
  /** Nl lemmas for monomial magnitudes (class 1) */
  NL_MONOMIAL_MAGNITUDE1,
  /** Nl lemmas for monomial magnitudes (class 2) */
  NL_MONOMIAL_MAGNITUDE2,
  /** Nl lemmas for monomial signs */
  NL_MONOMIAL_SIGN,
  /** Nl tangent plane lemmas */
  NL_TANGENT_PLANES,
  /** Nl tangent plane lemmas as waiting lemmas */
  NL_TANGENT_PLANES_WAITING,
};

/** Streaming operator for InferStep */
std::ostream& operator<<(std::ostream& os, InferStep step);

/** A sequence of steps */
using StepSequence = std::vector<InferStep>;

/**
 * A small wrapper around a StepSequence.
 *
 * This class makes handling a StepSequence slightly more convenient.
 * Also, it may help wrapping a more flexible strategy implementation in the
 * future.
 */
class StepGenerator
{
 public:
  StepGenerator(const StepSequence& ss) : d_steps(ss) {}
  /** Check if there is another step */
  bool hasNext() const;
  /** Get the next step */
  InferStep next();

 private:
  /** The StepSequence to process */
  const StepSequence& d_steps;
  /** The next step */
  std::size_t d_next = 0;
};

/**
 * A strategy for the nonlinear extension
 *
 * Initialization creates a single fixed step sequence. Calling
 * getStrategy() yields a StepGenerator that produces a sequence of InferSteps.
 */
class Strategy
{
 public:
  /** Is this strategy initialized? */
  bool isStrategyInit() const;
  /** Initialize this strategy */
  void initializeStrategy(const Options& options);
  /** Retrieve the strategy for the given effort e */
  StepGenerator getStrategy();

 private:
  /** The fixed sequence, which may be empty. */
  StepSequence d_steps;
  bool d_initialized = false;
};

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__ARITH__NL__STRATEGY_H */
