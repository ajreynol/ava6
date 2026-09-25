/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Check for monomial bound inference lemmas.
 */

#ifndef AVA6__THEORY__ARITH__NL__EXT__MONOMIAL_BOUNDS_CHECK_H
#define AVA6__THEORY__ARITH__NL__EXT__MONOMIAL_BOUNDS_CHECK_H

#include "expr/node.h"
#include "smt/env_obj.h"
#include "theory/arith/nl/ext/constraint.h"

namespace ava6::internal {
namespace theory {
namespace arith {
namespace nl {

class ExtState;

class MonomialBoundsCheck : protected EnvObj
{
 public:
  MonomialBoundsCheck(Env& env, ExtState* data);

  void init();

  /** check monomial inferred bounds
   *
   * Returns a set of valid theory lemmas, based on a
   * lemma schema that infers new constraints about existing
   * terms based on mulitplying both sides of an existing
   * constraint by a term.
   * For more details, see Section 5 of "Design Theory
   * Solvers with Extensions" by Reynolds et al., FroCoS 2017,
   * Figure 5, this is the schema "Multiply".
   *
   * Examples:
   *
   * x > 0 ^ (y > z + w) => x*y > x*(z+w)
   * x < 0 ^ (y > z + w) => x*y < x*(z+w)
   *   ...where (y > z + w) and x*y are a constraint and term
   *      that occur in the current context.
   */
  void checkBounds(const std::vector<Node>& asserts,
                   const std::vector<Node>& false_asserts);

 private:
  /** Basic data that is shared with other checks */
  ExtState* d_data;

  /** Context-independent database of constraint information */
  ConstraintDb d_cdb;

  // term -> coeff -> rhs -> ( status, exp, b ),
  //   where we have that : exp =>  ( coeff * term <status> rhs )
  //   b is true if degree( term ) >= degree( rhs )
  std::map<Node, std::map<Node, std::map<Node, Kind> > > d_ci;
  std::map<Node, std::map<Node, std::map<Node, Node> > > d_ci_exp;
  std::map<Node, std::map<Node, std::map<Node, bool> > > d_ci_max;
};

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace ava6::internal

#endif
