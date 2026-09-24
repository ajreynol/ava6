/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Registration and documentation for all public statistics.
 */

#include "ava6_private_library.h"

#ifndef AVA6__UTIL__STATISTICS_PUBLIC_H
#define AVA6__UTIL__STATISTICS_PUBLIC_H

namespace ava6::internal {

class StatisticsRegistry;

/**
 * Preregisters all public statistics.
 */
void registerPublicStatistics(StatisticsRegistry& reg);

}  // namespace ava6::internal

#endif
