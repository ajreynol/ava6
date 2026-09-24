/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * An enumerator for bitvectors.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__BV__TYPE_ENUMERATOR_H
#define AVA6__THEORY__BV__TYPE_ENUMERATOR_H

#include "expr/kind.h"
#include "expr/type_node.h"
#include "theory/bv/theory_bv_utils.h"
#include "theory/type_enumerator.h"
#include "util/bitvector.h"
#include "util/integer.h"

namespace ava6::internal {
namespace theory {
namespace bv {

class BitVectorEnumerator : public TypeEnumeratorBase<BitVectorEnumerator>
{
  size_t d_size;
  Integer d_bits;

 public:
  BitVectorEnumerator(TypeNode type,
                      AVA6_UNUSED TypeEnumeratorProperties* tep = nullptr)
      : TypeEnumeratorBase<BitVectorEnumerator>(type),
        d_size(type.getBitVectorSize()),
        d_bits(0)
  {
  }

  Node operator*() override
  {
    if (d_bits != d_bits.modByPow2(d_size))
    {
      throw NoMoreValuesException(getType());
    }
    return utils::mkConst(getType().getNodeManager(), d_size, d_bits);
  }

  BitVectorEnumerator& operator++() override
  {
    d_bits += 1;
    return *this;
  }

  bool isFinished() override { return d_bits != d_bits.modByPow2(d_size); }
}; /* BitVectorEnumerator */

}  // namespace bv
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__BV__TYPE_ENUMERATOR_H */
