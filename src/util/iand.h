/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The integer AND operator.
 */

#include "ava6_public.h"

#ifndef AVA6__IAND_H
#define AVA6__IAND_H

#include <iosfwd>
#include <ostream>

#include "base/exception.h"
#include "util/integer.h"

namespace ava6::internal {

struct IntAnd
{
  uint32_t d_size;
  IntAnd(uint32_t size) : d_size(size) {}
  operator uint32_t() const { return d_size; }
}; /* struct IntAnd */

/* -----------------------------------------------------------------------
 * Output stream
 * ----------------------------------------------------------------------- */

inline std::ostream& operator<<(std::ostream& os, const IntAnd& ia);
inline std::ostream& operator<<(std::ostream& os, const IntAnd& ia)
{
  return os << "(_ iand " << ia.d_size << ")";
}

}  // namespace ava6::internal

#endif /* AVA6__IAND_H */
