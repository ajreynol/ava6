/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of skolem manager class.
 */

#include "expr/internal_skolem_id.h"

#include <sstream>

namespace ava6::internal {

const char* toString(InternalSkolemId id)
{
  switch (id)
  {
    case InternalSkolemId::SEQ_MODEL_BASE_ELEMENT:
      return "SEQ_MODEL_BASE_ELEMENT";
    case InternalSkolemId::IEVAL_NONE: return "IEVAL_NONE";
    case InternalSkolemId::IEVAL_SOME: return "IEVAL_SOME";
    case InternalSkolemId::MBQI_INPUT: return "MBQI_INPUT";
    case InternalSkolemId::ABSTRACT_VALUE: return "ABSTRACT_VALUE";
    case InternalSkolemId::QUANTIFIERS_ATTRIBUTE_INTERNAL:
      return "QUANTIFIERS_ATTRIBUTE_INTERNAL";
    case InternalSkolemId::GET_VALUE_PURIFY: return "GET_VALUE_PURIFY";
    default: return "?";
  }
}

std::ostream& operator<<(std::ostream& out, InternalSkolemId id)
{
  out << toString(id);
  return out;
}

}  // namespace ava6::internal
