/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Type rules for the builtin theory.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__BUILTIN__THEORY_BUILTIN_TYPE_RULES_H
#define AVA6__THEORY__BUILTIN__THEORY_BUILTIN_TYPE_RULES_H

#include <sstream>

#include "expr/node.h"
#include "expr/type_node.h"

namespace ava6::internal {
namespace theory {
namespace builtin {

class EqualityTypeRule
{
 public:
  static TypeNode preComputeType(NodeManager* nm, TNode n);
  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

class SExprTypeRule
{
 public:
  static TypeNode preComputeType(NodeManager* nm, TNode n);
  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

class UninterpretedSortValueTypeRule
{
 public:
  static TypeNode preComputeType(NodeManager* nm, TNode n);
  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

class WitnessTypeRule
{
 public:
  static TypeNode preComputeType(NodeManager* nm, TNode n);
  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

class ApplyIndexedSymbolicTypeRule
{
 public:
  static TypeNode preComputeType(NodeManager* nm, TNode n);
  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

/**
 * Type rule for the internally used typeof operator used by RARE proof
 * reconstruction.
 */
class TypeOfTypeRule
{
 public:
  static TypeNode preComputeType(NodeManager* nm, TNode n);
  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

class SortProperties
{
 public:
  static bool isWellFounded(AVA6_UNUSED TypeNode type) { return true; }
  static Node mkGroundTerm(TypeNode type);
};

}  // namespace builtin
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__BUILTIN__THEORY_BUILTIN_TYPE_RULES_H */
