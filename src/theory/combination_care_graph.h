/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Management of a care graph based approach for theory combination.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__COMBINATION_CARE_GRAPH__H
#define AVA6__THEORY__COMBINATION_CARE_GRAPH__H

#include <vector>

#include "theory/combination_engine.h"

namespace ava6::internal {

class TheoryEngine;

namespace theory {

/**
 * Manager for doing theory combination using care graphs. This is typically
 * done via a distributed equality engine architecture.
 */
class CombinationCareGraph : public CombinationEngine
{
 public:
  CombinationCareGraph(Env& env,
                       TheoryEngine& te,
                       const std::vector<Theory*>& paraTheories);
  ~CombinationCareGraph();

  bool buildModel() override;
  /**
   * Combine theories using a care graph.
   */
  void combineTheories() override;
};

}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__COMBINATION_DISTRIBUTED__H */
