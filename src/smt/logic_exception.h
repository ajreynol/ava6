/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * An exception that is thrown when a feature is used outside
 * the logic that ava6 is currently using (for example, a quantifier
 * is used while running in a quantifier-free logic).
 */

#include "ava6_public.h"

#ifndef AVA6__SMT__LOGIC_EXCEPTION_H
#define AVA6__SMT__LOGIC_EXCEPTION_H

#include "base/exception.h"

namespace ava6::internal {

class LogicException : public ava6::internal::Exception
{
 public:
  LogicException()
      : Exception(
            "Feature used while operating in "
            "incorrect state")
  {
  }

  LogicException(const std::string& msg) : Exception(msg) {}

  LogicException(const char* msg) : Exception(msg) {}
}; /* class LogicException */

}  // namespace ava6::internal

#endif /* AVA6__SMT__LOGIC_EXCEPTION_H */
