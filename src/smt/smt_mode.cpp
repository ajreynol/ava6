/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Enumeration type for the mode of an SolverEngine.
 */

#include "smt/smt_mode.h"

#include <iostream>

namespace ava6::internal {

std::ostream& operator<<(std::ostream& out, SmtMode m)
{
  switch (m)
  {
    case SmtMode::START: out << "START"; break;
    case SmtMode::ASSERT: out << "ASSERT"; break;
    case SmtMode::SAT: out << "SAT"; break;
    case SmtMode::SAT_UNKNOWN: out << "UNKNOWN"; break;
    case SmtMode::UNSAT: out << "UNSAT"; break;
    default: out << "SmtMode!Unknown"; break;
  }
  return out;
}

}  // namespace ava6::internal
