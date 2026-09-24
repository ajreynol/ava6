/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Template for the metakind header.
 */

#include "ava6_private.h"

#ifndef AVA6__KIND__METAKIND_H
#define AVA6__KIND__METAKIND_H

#include <iosfwd>

#include "base/check.h"
#include "expr/kind.h"

namespace ava6::internal {

// clang-format off
${metakind_fwd_decls}
// clang-format on

namespace expr {
class NodeValue;
}  // namespace expr

namespace kind {
namespace metakind {

struct NodeValueCompare
{
  template <bool pool>
  static bool compare(const ava6::internal::expr::NodeValue* nv1,
                      const ava6::internal::expr::NodeValue* nv2);
  static size_t constHash(const ava6::internal::expr::NodeValue* nv);
}; /* struct NodeValueCompare */

/**
 * "metakinds" represent the "kinds" of kinds at the meta-level.
 * "metakind" is an ugly name but it's not used by client code, just
 * by the expr package, and the intent here is to keep it from
 * polluting the kind namespace.  For more documentation on what these
 * mean, see src/theory/builtin/kinds.
 */
enum MetaKind_t
{
  INVALID = -1,    /**< special node non-kinds like NULL_EXPR or LAST_KIND */
  VARIABLE,        /**< special node kinds: no operator */
  OPERATOR,        /**< operators that get "inlined" */
  PARAMETERIZED,   /**< parameterized ops (like APPLYs) that carry extra data */
  CONSTANT,        /**< constants */
  NULLARY_OPERATOR /**< nullary operator */
}; /* enum MetaKind_t */

/**
 * Write the string representation of a payload of a constant node to an output
 * stream.
 *
 * @param out the stream to write to
 * @param nv the node value representing a constant node
 */
void nodeValueConstantToStream(std::ostream& out,
                               const ava6::internal::expr::NodeValue* nv);

/**
 * Cleanup to be performed when a NodeValue zombie is collected, and
 * it has CONSTANT metakind.  This calls the destructor for the underlying
 * C++ type representing the constant value.  See
 * NodeManager::reclaimZombies() for more information.
 *
 * This doesn't support "non-inlined" NodeValues, which shouldn't need this
 * kind of cleanup.
 */
void deleteNodeValueConstant(ava6::internal::expr::NodeValue* nv);

/** Return the minimum arity of the given kind. */
uint32_t getMinArityForKind(ava6::internal::Kind k);
/** Return the maximum arity of the given kind. */
uint32_t getMaxArityForKind(ava6::internal::Kind k);

}  // namespace metakind

// import MetaKind into the "ava6::internal::kind" namespace but keep the
// individual MetaKind constants under kind::metakind::
typedef ava6::internal::kind::metakind::MetaKind_t MetaKind;

/**
 * Get the metakind for a particular kind.
 */
AVA6_EXPORT MetaKind metaKindOf(Kind k);

/**
 * Map a kind of the operator to the kind of the enclosing expression. For
 * example, since the kind of functions is just VARIABLE, it should map
 * VARIABLE to APPLY_UF.
 */
Kind operatorToKind(ava6::internal::expr::NodeValue* nv);

}  // namespace kind

namespace expr {

// Comparison predicate
struct NodeValuePoolEq
{
  bool operator()(const NodeValue* nv1, const NodeValue* nv2) const
  {
    return ava6::internal::kind::metakind::NodeValueCompare::compare<true>(nv1,
                                                                           nv2);
  }
};

}  // namespace expr
}  // namespace ava6::internal

#endif /* AVA6__KIND__METAKIND_H */
