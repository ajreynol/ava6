/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * An enumerator for Booleans.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__BOOLEANS__TYPE_ENUMERATOR_H
#define AVA6__THEORY__BOOLEANS__TYPE_ENUMERATOR_H

#include "expr/kind.h"
#include "expr/type_node.h"
#include "theory/type_enumerator.h"

namespace ava6::internal {
namespace theory {
namespace booleans {

class BooleanEnumerator : public TypeEnumeratorBase<BooleanEnumerator>
{
  enum
  {
    FALSE,
    TRUE,
    DONE
  } d_value;

 public:
  BooleanEnumerator(TypeNode type,
                    AVA6_UNUSED TypeEnumeratorProperties* tep = nullptr)
      : TypeEnumeratorBase<BooleanEnumerator>(type), d_value(FALSE)
  {
    Assert(type.getKind() == Kind::TYPE_CONSTANT
           && type.getConst<TypeConstant>() == BOOLEAN_TYPE);
  }

  Node operator*() override
  {
    switch (d_value)
    {
      case FALSE: return getType().getNodeManager()->mkConst(false);
      case TRUE: return getType().getNodeManager()->mkConst(true);
      default: throw NoMoreValuesException(getType());
    }
  }

  BooleanEnumerator& operator++() override
  {
    // sequence is FALSE, TRUE
    if (d_value == FALSE)
    {
      d_value = TRUE;
    }
    else
    {
      d_value = DONE;
    }
    return *this;
  }

  bool isFinished() override { return d_value == DONE; }
}; /* class BooleanEnumerator */

}  // namespace booleans
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__BOOLEANS__TYPE_ENUMERATOR_H */
