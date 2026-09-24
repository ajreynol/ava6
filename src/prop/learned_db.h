/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Stores learned information
 */

#include "prop/learned_literal_type.h"
#include "ava6_private.h"

#ifndef AVA6__PROP__LEARNED_DB_H
#define AVA6__PROP__LEARNED_DB_H

#include <ava6/ava6_types.h>

#include "context/cdhashset.h"
#include "context/cdo.h"
#include "expr/node.h"

namespace ava6::internal {
namespace prop {

/**
 * This class stores high-level information learned during a run of the
 * PropEngine. This includes the set of learned literals for each category
 * (LearnedLitType).
 */
class LearnedDb
{
  using NodeSet = context::CDHashSet<Node>;

 public:
  LearnedDb(context::Context* c);
  ~LearnedDb();
  /** Add learned literal of the given type */
  void addLearnedLiteral(const Node& lit, LearnedLitType ltype);
  /** Get the learned literals for the given type */
  std::vector<Node> getLearnedLiterals(
      LearnedLitType ltype = LearnedLitType::INPUT) const;
  /** Get number of learned literals for the given type */
  size_t getNumLearnedLiterals(
      LearnedLitType ltype = LearnedLitType::INPUT) const;
  /** To string debug */
  std::string toStringDebug() const;

 private:
  /** Get literal set, const and non-const versions */
  context::CDHashSet<Node>& getLiteralSet(LearnedLitType ltype);
  const context::CDHashSet<Node>& getLiteralSet(
      LearnedLitType ltype) const;
  /** To string debug for type of literals */
  std::string toStringDebugType(LearnedLitType ltype) const;
  /** preprocess solved lits */
  NodeSet d_preprocessSolvedLits;
  /** preprocess lits */
  NodeSet d_preprocessLits;
  /** Input lits */
  NodeSet d_inputLits;
  /** Solvable lits */
  NodeSet d_solvableLits;
  /** Constant propagation lits */
  NodeSet d_cpropLits;
  /** Internal lits */
  NodeSet d_internalLits;
};

}  // namespace prop
}  // namespace ava6::internal

#endif
