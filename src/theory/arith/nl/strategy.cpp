/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of non-linear solver.
 */

#include "theory/arith/nl/strategy.h"

#include <iostream>

#include "base/check.h"
#include "options/arith_options.h"

namespace ava6::internal {
namespace theory {
namespace arith {
namespace nl {

std::ostream& operator<<(std::ostream& os, InferStep step)
{
  switch (step)
  {
    case InferStep::BREAK: return os << "BREAK";
    case InferStep::FLUSH_WAITING_LEMMAS: return os << "FLUSH_WAITING_LEMMAS";
    case InferStep::NL_FACTORING: return os << "NL_FACTORING";
    case InferStep::NL_FLATTEN_MON: return os << "NL_FLATTEN_MON";
    case InferStep::NL_INIT: return os << "NL_INIT";
    case InferStep::NL_MONOMIAL_INFER_BOUNDS:
      return os << "NL_MONOMIAL_INFER_BOUNDS";
    case InferStep::NL_MONOMIAL_MAGNITUDE0:
      return os << "NL_MONOMIAL_MAGNITUDE0";
    case InferStep::NL_MONOMIAL_MAGNITUDE1:
      return os << "NL_MONOMIAL_MAGNITUDE1";
    case InferStep::NL_MONOMIAL_MAGNITUDE2:
      return os << "NL_MONOMIAL_MAGNITUDE2";
    case InferStep::NL_MONOMIAL_SIGN: return os << "NL_MONOMIAL_SIGN";
    case InferStep::NL_TANGENT_PLANES: return os << "NL_TANGENT_PLANES";
    case InferStep::NL_TANGENT_PLANES_WAITING:
      return os << "NL_TANGENT_PLANES_WAITING";
    default: Unreachable(); return os << "UNKNOWN_STEP";
  }
}

namespace {
/** Puts a new InferStep into a StepSequence */
inline StepSequence& operator<<(StepSequence& steps, InferStep s)
{
  steps.emplace_back(s);
  return steps;
}
}  // namespace

bool StepGenerator::hasNext() const { return d_next < d_steps.size(); }
InferStep StepGenerator::next() { return d_steps[d_next++]; }

bool Strategy::isStrategyInit() const { return d_initialized; }
void Strategy::initializeStrategy(const Options& options)
{
  Assert(!d_initialized);
  StepSequence& one = d_steps;
  
  if (options.arith.nlExt == options::NlExtMode::FULL
      || options.arith.nlExt == options::NlExtMode::LIGHT)
  {
    one << InferStep::NL_INIT << InferStep::BREAK;
  }
  if (options.arith.nlExt == options::NlExtMode::FULL
      || options.arith.nlExt == options::NlExtMode::LIGHT)
  {
    one << InferStep::NL_MONOMIAL_SIGN << InferStep::BREAK;
    one << InferStep::NL_MONOMIAL_MAGNITUDE0 << InferStep::BREAK;
  }
  if (options.arith.nlExtFlattenMon)
  {
    one << InferStep::NL_FLATTEN_MON << InferStep::BREAK;
  }
  if (options.arith.nlExt == options::NlExtMode::FULL)
  {
    one << InferStep::NL_MONOMIAL_MAGNITUDE1 << InferStep::BREAK;
    one << InferStep::NL_MONOMIAL_MAGNITUDE2 << InferStep::BREAK;
    one << InferStep::NL_MONOMIAL_INFER_BOUNDS;
    if (options.arith.nlExtTangentPlanes
        && options.arith.nlExtTangentPlanesInterleave)
    {
      one << InferStep::NL_TANGENT_PLANES;
    }
    one << InferStep::BREAK;
  }
  
  if (options.arith.nlExt == options::NlExtMode::FULL)
  {
    // Use heuristic non-terminating techniques as a last resort.
    one << InferStep::FLUSH_WAITING_LEMMAS << InferStep::BREAK;
    if (options.arith.nlExtFactor)
    {
      one << InferStep::NL_FACTORING << InferStep::BREAK;
    }
    
    if (options.arith.nlExtTangentPlanes
        && !options.arith.nlExtTangentPlanesInterleave)
    {
      one << InferStep::NL_TANGENT_PLANES_WAITING;
    }
    one << InferStep::BREAK;
  }

  d_initialized = true;
}
StepGenerator Strategy::getStrategy()
{
  return StepGenerator(d_steps);
}

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace ava6::internal
