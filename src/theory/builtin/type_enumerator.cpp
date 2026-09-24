/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Enumerator for uninterpreted sorts and functions.
 */

#include "theory/builtin/type_enumerator.h"

#include "theory/builtin/theory_builtin_rewriter.h"
#include "util/uninterpreted_sort_value.h"

namespace ava6::internal {
namespace theory {
namespace builtin {

UninterpretedSortEnumerator::UninterpretedSortEnumerator(
    TypeNode type, TypeEnumeratorProperties* tep AVA6_UNUSED)
    : TypeEnumeratorBase<UninterpretedSortEnumerator>(type), d_count(0)
{
  Assert(type.isUninterpretedSort());
  Trace("uf-type-enum") << "UF enum " << type << ", tep = " << tep << std::endl;

}

Node UninterpretedSortEnumerator::operator*()
{
  if (isFinished())
  {
    throw NoMoreValuesException(getType());
  }
  return getType().getNodeManager()->mkConst(
      UninterpretedSortValue(getType(), d_count));
}

UninterpretedSortEnumerator& UninterpretedSortEnumerator::operator++()
{
  d_count += 1;
  return *this;
}

bool UninterpretedSortEnumerator::isFinished()
{

  return false;
}

}  // namespace builtin
}  // namespace theory
}  // namespace ava6::internal
