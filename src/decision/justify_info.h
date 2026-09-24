/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Justification info.
 */

#include "ava6_private.h"

#ifndef AVA6__DECISION__JUSTIFY_INFO_H
#define AVA6__DECISION__JUSTIFY_INFO_H

#include "context/cdo.h"
#include "expr/node.h"
#include "prop/sat_solver_types.h"

namespace ava6::internal {
namespace decision {

/** A pair indicating a node and its desired value */
using JustifyNode = std::pair<TNode, prop::SatValue>;

/**
 * Information concerning a single formula in the justification strategy.
 */
class JustifyInfo
{
 public:
  JustifyInfo(context::Context* c);
  ~JustifyInfo();
  /** set */
  void set(TNode n, prop::SatValue desiredVal);
  /** get node */
  JustifyNode getNode() const;
  /** get next child index, and increment */
  size_t getNextChildIndex();
  /** revert child index */
  void revertChildIndex();

 private:
  /** The node we are considering */
  context::CDO<TNode> d_node;
  /** Desired value */
  context::CDO<prop::SatValue> d_desiredVal;
  /** The child index we are considering */
  context::CDO<size_t> d_childIndex;
};

}  // namespace decision
}  // namespace ava6::internal

#endif /* AVA6__DECISION__JUSTIFY_INFO_H */
