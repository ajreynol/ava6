/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Arithmetic evaluator.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__ARITH__ARITH_EVALUATOR_H
#define AVA6__THEORY__ARITH__ARITH_EVALUATOR_H

#include <optional>

#include "expr/node.h"
#include "smt/env.h"
#include "theory/arith/arith_subs.h"

namespace ava6::internal {
namespace theory {
namespace arith {

/**
 * Check if the expression `expr` is zero over the given model.
 * The environment is used for rewriting under the model substitutions.
 *
 * The result is true or false, if the expression could be evaluated. If it
 * could not, the result is std::nullopt.
 */
std::optional<bool> isExpressionZero(Env& env,
                                     Node expr,
                                     const ArithSubs& subs,
                                     bool traverseNlMult);
}  // namespace arith
}  // namespace theory
}  // namespace ava6::internal

#endif
