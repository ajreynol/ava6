/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of skolem id.
 */

#include <ava6/ava6_skolem_id.h>

#include <iostream>

#include "printer/enum_to_string.h"

namespace std {

std::string to_string(ava6::SkolemId id)
{
  return ava6::internal::toString(id);
}
}  // namespace std

namespace ava6 {

std::ostream& operator<<(std::ostream& out, SkolemId id)
{
  out << std::to_string(id);
  return out;
}
}  // namespace ava6

namespace std {

size_t hash<ava6::SkolemId>::operator()(ava6::SkolemId id) const
{
  return static_cast<size_t>(id);
}

}  // namespace std
