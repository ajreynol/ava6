/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * [[ Add one-line brief description here ]]
 *
 * [[ Add lengthier description here ]]
 * \todo document this file
 */
#include "theory/arith/linear/approx_simplex.h"

#include <cfloat>
#include <cmath>
#include <unordered_set>

#include "base/ava6config.h"
#include "base/output.h"
#include "proof/eager_proof_generator.h"
#include "theory/arith/linear/constraint.h"
#include "theory/arith/linear/cut_log.h"
#include "theory/arith/linear/matrix.h"
#include "theory/arith/linear/normal_form.h"
#include "util/statistics_registry.h"


/* Begin GPLK/NOGLPK Glue code. */
namespace ava6::internal {
namespace theory {
namespace arith::linear {

ApproximateSimplex* ApproximateSimplex::mkApproximateSimplexSolver(
    AVA6_UNUSED const ArithVariables& vars,
    AVA6_UNUSED TreeLog& l,
    AVA6_UNUSED ApproximateStatistics& s)
{
  Unimplemented() << "Approximate simplex solver requires GLPK";
}

bool ApproximateSimplex::enabled()
{
  return false;
}

ApproximateStatistics::ApproximateStatistics(StatisticsRegistry& sr)
    : d_branchMaxDepth(sr.registerInt("z::approx::branchMaxDepth")),
      d_branchesMaxOnAVar(sr.registerInt("z::approx::branchesMaxOnAVar")),
      d_gaussianElimConstructTime(
          sr.registerTimer("z::approx::gaussianElimConstruct::time")),
      d_gaussianElimConstruct(
          sr.registerInt("z::approx::gaussianElimConstruct::calls")),
      d_averageGuesses(sr.registerAverage("z::approx::averageGuesses"))
{
}

std::ostream& operator<<(std::ostream& out, MipResult res)
{
  switch (res)
  {
    case MipUnknown: out << "MipUnknown"; break;
    case MipBingo: out << "MipBingo"; break;
    case MipClosed: out << "MipClosed"; break;
    case BranchesExhausted: out << "BranchesExhausted"; break;
    case PivotsExhauasted: out << "PivotsExhauasted"; break;
    case ExecExhausted: out << "ExecExhausted"; break;
    default: out << "Unexpected Mip Value!"; break;
  }
  return out;
}

}  // namespace arith::linear
}  // namespace theory
}  // namespace ava6::internal
/* End GPLK/NOGLPK Glue code. */
