/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Interface for constructing the clauses emitted by the CNF stream.
 */
#include "ava6_private.h"

#ifndef AVA6__PROP__SAT_CLAUSE_SINK_H
#define AVA6__PROP__SAT_CLAUSE_SINK_H

#include "prop/sat_solver_types.h"

namespace ava6::internal::prop {

class SatClauseSink
{
 public:
  virtual ~SatClauseSink() = default;
  /** Add a clause; return whether the sink accepted it. */
  virtual bool addClause(const SatClause& clause, bool removable) = 0;
  virtual SatVariable newVar(bool isTheoryAtom) = 0;
  virtual SatVariable trueVar() = 0;
  virtual SatVariable falseVar() = 0;
};

}  // namespace ava6::internal::prop
#endif
