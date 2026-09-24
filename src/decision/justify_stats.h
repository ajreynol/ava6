/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Justification stats.
 */

#include "ava6_private.h"

#ifndef AVA6__DECISION__JUSTIFY_STATS_H
#define AVA6__DECISION__JUSTIFY_STATS_H

#include "util/statistics_registry.h"

namespace ava6::internal {
namespace decision {

class JustifyStatistics
{
 public:
  JustifyStatistics(StatisticsRegistry& sr);
  ~JustifyStatistics();
  /** Number of times we considered an assertion not leading to a decision */
  IntStat d_numStatusNoDecision;
  /** Number of times we considered an assertion that led to a decision */
  IntStat d_numStatusDecision;
  /** Number of times we considered an assertion that led to backtracking */
  IntStat d_numStatusBacktrack;
  /** Maximum stack size we considered */
  IntStat d_maxStackSize;
  /** Maximum assertion size we considered */
  IntStat d_maxAssertionsSize;
  /** Maximum skolem definition size we considered */
  IntStat d_maxSkolemDefsSize;
  /** Get next decision time */
  TimerStat d_time;
};

}  // namespace decision
}  // namespace ava6::internal

#endif /* AVA6__DECISION__JUSTIFY_STATS_H */
