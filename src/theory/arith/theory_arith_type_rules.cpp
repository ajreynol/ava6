/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Typing and cardinality rules for theory arithmetic.
 */

#include "theory/arith/theory_arith_type_rules.h"

#include "util/rational.h"

namespace ava6::internal {
namespace theory {
namespace arith {

bool isMaybeRealOrInt(const TypeNode& tn)
{
  return tn.isRealOrInt() || tn.isFullyAbstract();
}

bool isMaybeInteger(const TypeNode& tn)
{
  return tn.isInteger() || tn.isFullyAbstract();
}

TypeNode ArithConstantTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                               AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode ArithConstantTypeRule::computeType(NodeManager* nodeManager,
                                            TNode n,
                                            bool check,
                                            AVA6_UNUSED std::ostream* errOut)
{
  // we use different kinds for constant integers and reals
  if (n.getKind() == Kind::CONST_RATIONAL)
  {
    // constant rationals are always real type, even if their value is integral
    return nodeManager->realType();
  }
  Assert(n.getKind() == Kind::CONST_INTEGER);
  // constant integers should always have integral value
  if (check)
  {
    if (!n.getConst<Rational>().isIntegral())
    {
      throw TypeCheckingExceptionPrivate(
          n, "making an integer constant from a non-integral rational");
      return TypeNode::null();
    }
  }
  return nodeManager->integerType();
}

TypeNode ArithOperatorTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                               AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}
TypeNode ArithOperatorTypeRule::computeType(NodeManager* nodeManager,
                                            TNode n,
                                            bool check,
                                            std::ostream* errOut)
{
  TypeNode integerType = nodeManager->integerType();
  TypeNode realType = nodeManager->realType();
  TNode::iterator child_it = n.begin();
  TNode::iterator child_it_end = n.end();
  bool isAbstract = false;
  bool isInteger = true;
  Kind k = n.getKind();
  for (; child_it != child_it_end; ++child_it)
  {
    TypeNode childType = (*child_it).getTypeOrNull();
    if (childType.isAbstract())
    {
      isAbstract = true;
    }
    else if (!childType.isInteger())
    {
      isInteger = false;
      if (!check)
      {  // if we're not checking, nothing left to do
        break;
      }
    }
    if (check)
    {
      if (!isMaybeRealOrInt(childType))
      {
        if (errOut)
        {
          (*errOut) << "expecting an arithmetic subterm";
        }
        return TypeNode::null();
      }
      if (k == Kind::TO_REAL && !childType.isInteger())
      {
        if (errOut)
        {
          (*errOut) << "expecting an integer subterm";
        }
        return TypeNode::null();
      }
    }
  }
  switch (k)
  {
    case Kind::TO_REAL:
    case Kind::DIVISION:
    case Kind::DIVISION_TOTAL: return realType;
    case Kind::TO_INTEGER: return integerType;
    default:
    {
      if (isAbstract)
      {
        // fully abstract since Int and Real are incomparable
        // NOTE: could use an abstract real???
        return nodeManager->mkAbstractType(Kind::ABSTRACT_TYPE);
      }
      return isInteger ? integerType : realType;
    }
  }
}

TypeNode ArithRelationTypeRule::preComputeType(NodeManager* nm,
                                               AVA6_UNUSED TNode n)
{
  return nm->booleanType();
}
TypeNode ArithRelationTypeRule::computeType(NodeManager* nodeManager,
                                            TNode n,
                                            bool check,
                                            std::ostream* errOut)
{
  if (check)
  {
    Assert(n.getNumChildren() == 2);
    if (!isMaybeRealOrInt(n[0].getTypeOrNull())
        || !isMaybeRealOrInt(n[1].getTypeOrNull()))
    {
      if (errOut)
      {
        (*errOut) << "expecting an arithmetic subterm for arithmetic relation";
      }
      return TypeNode::null();
    }
  }
  return nodeManager->booleanType();
}

TypeNode PowTypeRule::preComputeType(AVA6_UNUSED NodeManager* nm,
                                     AVA6_UNUSED TNode n)
{
  return TypeNode::null();
}

TypeNode PowTypeRule::computeType(AVA6_UNUSED NodeManager* nodeManager,
                                  TNode n,
                                  AVA6_UNUSED bool check,
                                  std::ostream* errOut)
{
  Assert(n.getKind() == Kind::POW);
  TypeNode arg1 = n[0].getTypeOrNull();
  TypeNode arg2 = n[1].getTypeOrNull();
  TypeNode t = arg1.leastUpperBound(arg2);
  if (t.isNull())
  {
    if (errOut)
    {
      (*errOut) << "expecting same arithmetic types to POW";
    }
    return TypeNode::null();
  }
  return t;
}

}  // namespace arith
}  // namespace theory
}  // namespace ava6::internal
