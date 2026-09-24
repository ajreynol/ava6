/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Common header for replacement function sources.
 */

#ifndef AVA6__LIB__REPLACEMENTS_H
#define AVA6__LIB__REPLACEMENTS_H

#if (defined(__BUILDING_AVA6LIB) || defined(__BUILDING_AVA6LIB_UNIT_TEST)) \
    && !defined(__BUILDING_STATISTICS_FOR_EXPORT)
#include "ava6_private.h"
#else
#if defined(__BUILDING_AVA6PARSERLIB) \
    || defined(__BUILDING_AVA6PARSERLIB_UNIT_TEST)
#include "ava6parser_private.h"
#else
#if defined(__BUILDING_AVA6DRIVER) || defined(__BUILDING_AVA6_SYSTEM_TEST) \
    || defined(__BUILDING_STATISTICS_FOR_EXPORT)
#include "base/ava6config.h"
#else
#      error Must be building libava6 or libava6parser to use replacement functions.  This is because replacement function headers should never be publicly-depended upon, as they should not be installed on user machines with 'make install'.
#endif
#endif
#endif

#endif /* AVA6__LIB__REPLACEMENTS_H */
