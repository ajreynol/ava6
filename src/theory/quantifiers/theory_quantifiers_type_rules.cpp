/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Theory of quantifiers.
 */

#include "theory/quantifiers/theory_quantifiers_type_rules.h"


namespace ava6::internal {
namespace theory {
namespace quantifiers {

TypeNode QuantifierTypeRule::preComputeType(NodeManager* nm,
                                            AVA6_UNUSED TNode n)
{
  return nm->booleanType();
}
TypeNode QuantifierTypeRule::computeType(NodeManager* nodeManager,
                                         TNode n,
                                         bool check,
                                         std::ostream* errOut)
{
  Trace("typecheck-q") << "type check for fa " << n << std::endl;
  Assert((n.getKind() == Kind::FORALL || n.getKind() == Kind::EXISTS)
         && n.getNumChildren() > 0);
  if (check)
  {
    // bound variable lists, etc. cannot be abstracted
    if (!AVA6_EQUAL(n[0].getTypeOrNull(), nodeManager->boundVarListType()))
    {
      if (errOut)
      {
        (*errOut) << "first argument of quantifier is not bound var list";
      }
      return TypeNode::null();
    }
    TypeNode bodyType = n[1].getTypeOrNull();
    if (!bodyType.isBoolean() && !bodyType.isFullyAbstract())
    {
      if (errOut)
      {
        (*errOut) << "body of quantifier is not boolean";
      }
      return TypeNode::null();
    }
    if (n.getNumChildren() == 3)
    {
      if (!AVA6_EQUAL(n[2].getTypeOrNull(), nodeManager->instPatternListType()))
      {
        if (errOut)
        {
          (*errOut) << "third argument of quantifier is not instantiation "
                       "pattern list";
        }
        return TypeNode::null();
      }

    }
  }
  return nodeManager->booleanType();
}

TypeNode QuantifierBoundVarListTypeRule::preComputeType(NodeManager* nm,
                                                        AVA6_UNUSED TNode n)
{
  return nm->boundVarListType();
}
TypeNode QuantifierBoundVarListTypeRule::computeType(NodeManager* nodeManager,
                                                     TNode n,
                                                     bool check,
                                                     std::ostream* errOut)
{
  Assert(n.getKind() == Kind::BOUND_VAR_LIST);
  if (check)
  {
    for (const Node& nc : n)
    {
      if (nc.getKind() != Kind::BOUND_VARIABLE)
      {
        if (errOut)
        {
          (*errOut) << "argument of bound var list is not bound variable";
        }
        return TypeNode::null();
      }
    }
  }
  return nodeManager->boundVarListType();
}

TypeNode QuantifierInstPatternTypeRule::preComputeType(NodeManager* nm,
                                                       AVA6_UNUSED TNode n)
{
  return nm->instPatternType();
}
TypeNode QuantifierInstPatternTypeRule::computeType(NodeManager* nodeManager,
                                                    TNode n,
                                                    bool check,
                                                    std::ostream* errOut)
{
  Assert(n.getKind() == Kind::INST_PATTERN);
  if (check)
  {
    TypeNode tn = n[0].getTypeOrNull();
    // this check catches the common mistake writing :pattern (f x) instead of
    // :pattern ((f x))
    if (n[0].isVar() && n[0].getKind() != Kind::BOUND_VARIABLE
        && tn.isFunction())
    {
      if (errOut)
      {
        (*errOut) << "Pattern must be a list of fully-applied terms.";
      }
      return TypeNode::null();
    }
  }
  return nodeManager->instPatternType();
}

TypeNode QuantifierAnnotationTypeRule::preComputeType(NodeManager* nm,
                                                      AVA6_UNUSED TNode n)
{
  return nm->instPatternType();
}
TypeNode QuantifierAnnotationTypeRule::computeType(NodeManager* nodeManager,
                                                   TNode n,
                                                   bool check,
                                                   std::ostream* errOut AVA6_UNUSED)
{
  if (check)
  {
    Kind k = n.getKind();
    if (k == Kind::INST_ATTRIBUTE)
    {
      if (n.getNumChildren() > 1)
      {
        // first must be a keyword
        if (n[0].getKind() != Kind::CONST_STRING)
        {
          throw TypeCheckingExceptionPrivate(
              n[0], "Expecting a keyword at the head of INST_ATTRIBUTE.");
        }
      }
    }


  }
  return nodeManager->instPatternType();
}

TypeNode QuantifierInstPatternListTypeRule::preComputeType(NodeManager* nm,
                                                           AVA6_UNUSED TNode n)
{
  return nm->instPatternListType();
}
TypeNode QuantifierInstPatternListTypeRule::computeType(
    NodeManager* nodeManager, TNode n, bool check, std::ostream* errOut)
{
  Assert(n.getKind() == Kind::INST_PATTERN_LIST);
  if (check)
  {
    for (const Node& nc : n)
    {
      Kind k = nc.getKind();
      if (k != Kind::INST_PATTERN && k != Kind::INST_NO_PATTERN
          && k != Kind::INST_ATTRIBUTE)
      {
        if (errOut)
        {
          (*errOut) << "argument of inst pattern list is not a legal "
                       "quantifiers annotation";
        }
        return TypeNode::null();
      }
    }
  }
  return nodeManager->instPatternListType();
}

}  // namespace quantifiers
}  // namespace theory
}  // namespace ava6::internal
