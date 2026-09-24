/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Typing and cardinality rules for the theory of boolean.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY_BOOL_TYPE_RULES_H
#define AVA6__THEORY_BOOL_TYPE_RULES_H

#include "expr/node.h"
#include "expr/type_node.h"

namespace ava6::internal {
namespace theory {
namespace boolean {

class BooleanTypeRule
{
 public:
  static TypeNode preComputeType(NodeManager* nm, TNode n);
  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

class IteTypeRule
{
 public:
  static TypeNode preComputeType(NodeManager* nm, TNode n);
  static TypeNode computeType(NodeManager* nodeManager,
                              TNode n,
                              bool check,
                              std::ostream* errOut);
};

}  // namespace boolean
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY_BOOL_TYPE_RULES_H */
