/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Utilities for nodes in the arithmetic rewriter.
 */

#include "theory/arith/rewriter/node_utils.h"

#include "base/check.h"
#include "theory/arith/rewriter/ordering.h"

namespace ava6::internal {
namespace theory {
namespace arith {
namespace rewriter {

Node mkMultTerm(const Rational& multiplicity, TNode monomial)
{
  NodeManager* nm = monomial.getNodeManager();
  if (monomial.isConst())
  {
    return mkConst(nm, multiplicity * monomial.getConst<Rational>());
  }
  if (multiplicity.isOne())
  {
    return monomial;
  }
  return NodeManager::mkNode(Kind::MULT, mkConst(nm, multiplicity), monomial);
}

Node mkMultTerm(NodeManager* nm,
                const Rational& multiplicity,
                std::vector<Node>&& monomial)
{
  if (monomial.empty())
  {
    return mkConst(nm, multiplicity);
  }
  std::sort(monomial.begin(), monomial.end(), rewriter::LeafNodeComparator());
  return mkMultTerm(multiplicity, mkNonlinearMult(nm, monomial));
}

TNode removeToReal(TNode t) { return t.getKind() == Kind::TO_REAL ? t[0] : t; }

Node maybeEnsureReal(TypeNode tn, TNode t)
{
  // if we require being a real
  if (tn.isReal())
  {
    return ensureReal(t);
  }
  return t;
}

Node ensureReal(TNode t)
{
  if (t.getType().isInteger())
  {
    if (t.isConst())
    {
      // short-circuit
      Node ret = t.getNodeManager()->mkConstReal(t.getConst<Rational>());
      Assert(ret.getType().isReal());
      return ret;
    }
    Trace("arith-rewriter-debug") << "maybeEnsureReal: " << t << std::endl;
    return NodeManager::mkNode(Kind::TO_REAL, t);
  }
  return t;
}

}  // namespace rewriter
}  // namespace arith
}  // namespace theory
}  // namespace ava6::internal
