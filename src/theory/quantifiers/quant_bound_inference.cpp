/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of quantifiers bound inference.
 */

#include "theory/quantifiers/quant_bound_inference.h"
#include "util/cardinality.h"

#include "theory/quantifiers/fmf/bounded_integers.h"
#include "util/rational.h"

using namespace ava6::internal::kind;

namespace ava6::internal {
namespace theory {
namespace quantifiers {

QuantifiersBoundInference::QuantifiersBoundInference(unsigned cardMax)
    : d_cardMax(cardMax), d_bint(nullptr)
{
}

void QuantifiersBoundInference::finishInit(BoundedIntegers* b) { d_bint = b; }

bool QuantifiersBoundInference::mayComplete(TypeNode tn)
{
  std::unordered_map<TypeNode, bool>::iterator it = d_may_complete.find(tn);
  if (it == d_may_complete.end())
  {
    // cache
    bool mc = mayComplete(tn, d_cardMax);
    d_may_complete[tn] = mc;
    return mc;
  }
  return it->second;
}

bool QuantifiersBoundInference::mayComplete(TypeNode tn, unsigned maxCard)
{
  if (!tn.isClosedEnumerable())
  {
    return false;
  }
  bool mc = false;
  if (isCardinalityClassFinite(tn.getCardinalityClass()))
  {
    Cardinality c = tn.getCardinality();
    if (!c.isLargeFinite())
    {
      // check if less than fixed upper bound
      mc = (c.getFiniteCardinality() < Integer(maxCard));
    }
  }
  return mc;
}

bool QuantifiersBoundInference::isFiniteBound(Node q, Node v)
{
  if (d_bint && d_bint->isBound(q, v))
  {
    return true;
  }
  TypeNode tn = v.getType();
  if (mayComplete(tn))
  {
    return true;
  }
  return false;
}

BoundVarType QuantifiersBoundInference::getBoundVarType(Node q, Node v)
{
  if (d_bint)
  {
    BoundVarType bvt = d_bint->getBoundVarType(q, v);
    // If the bounded integer module has a bound, use it. Otherwise, we fall
    // through.
    if (bvt != BOUND_NONE)
    {
      return bvt;
    }
  }
  return isFiniteBound(q, v) ? BOUND_FINITE : BOUND_NONE;
}

void QuantifiersBoundInference::getBoundVarIndices(
    Node q, std::vector<size_t>& indices) const
{
  Assert(indices.empty());
  // we take the bounded variables first
  if (d_bint)
  {
    d_bint->getBoundVarIndices(q, indices);
  }
  // then get the remaining ones
  for (size_t i = 0, nvars = q[0].getNumChildren(); i < nvars; i++)
  {
    if (std::find(indices.begin(), indices.end(), i) == indices.end())
    {
      indices.push_back(i);
    }
  }
}

bool QuantifiersBoundInference::getBoundElements(
    RepSetIterator* rsi,
    bool initial,
    Node q,
    Node v,
    std::vector<Node>& elements) const
{
  if (d_bint)
  {
    return d_bint->getBoundElements(rsi, initial, q, v, elements);
  }
  return false;
}

}  // namespace quantifiers
}  // namespace theory
}  // namespace ava6::internal
