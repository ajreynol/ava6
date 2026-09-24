/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Template for the Node kind header.
 */

#include "ava6_public.h"

#ifndef AVA6__KIND_H
#define AVA6__KIND_H

#include <iosfwd>

#include "base/exception.h"
#include "theory/theory_id.h"

namespace ava6::internal {
namespace kind {

enum class Kind_t
{
  UNDEFINED_KIND = -1, /**< undefined */
  NULL_EXPR,           /**< Null kind */
  // clang-format off
${kind_decls} LAST_KIND /**< marks the upper-bound of this enumeration */
  // clang-format on

}; /* enum Kind_t */

}  // namespace kind

// import Kind into the "ava6" namespace but keep the individual kind
// constants under kind::
typedef ava6::internal::kind::Kind_t Kind;

namespace kind {

/**
 * Converts an kind to a string. Note: This function is also used in
 * `safe_print()`. Changing this functions name or signature will result in
 * `safe_print()` printing "<unsupported>" instead of the proper strings for
 * the enum values.
 *
 * @param k The kind
 * @return The name of the kind
 */
const char* toString(ava6::internal::Kind k);

/**
 * Writes a kind name to a stream.
 *
 * @param out The stream to write to
 * @param k The kind to write to the stream
 * @return The stream
 */
std::ostream& operator<<(std::ostream&, ava6::internal::Kind);

/** Returns true if the given kind is associative. This is used by ExprManager
 * to decide whether it's safe to modify big expressions by changing the
 * grouping of the arguments. */
/* TODO: This could be generated. */
bool isAssociative(ava6::internal::Kind k);
std::string kindToString(ava6::internal::Kind k);

/** Return true if k is a closure kind. */
bool isClosureKind(ava6::internal::Kind k);

struct KindHashFunction
{
  inline size_t operator()(ava6::internal::Kind k) const
  {
    return static_cast<size_t>(k);
  }
}; /* struct KindHashFunction */

}  // namespace kind

/**
 * The enumeration for the built-in atomic types.
 */
enum TypeConstant
{
  // clang-format off
  ${type_constant_list} LAST_TYPE
  // clang-format on
}; /* enum TypeConstant */

/**
 * We hash the constants with their values.
 */
struct TypeConstantHashFunction
{
  inline size_t operator()(TypeConstant tc) const { return tc; }
}; /* struct TypeConstantHashFunction */

const char* toString(TypeConstant tc);
std::ostream& operator<<(std::ostream& out, TypeConstant typeConstant);

namespace theory {

ava6::internal::theory::TheoryId kindToTheoryId(ava6::internal::Kind k);
ava6::internal::theory::TheoryId typeConstantToTheoryId(
    ava6::internal::TypeConstant typeConstant);

}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__KIND_H */
