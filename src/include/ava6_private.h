/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Inclusion of this file marks a header as private and generates a warning
 * when the file is included improperly.
 */

#ifndef AVA6_PRIVATE_H
#define AVA6_PRIVATE_H

#if !(defined(__BUILDING_AVA6LIB) || defined(__BUILDING_AVA6PARSERLIB) \
      || defined(__BUILDING_AVA6LIB_UNIT_TEST))
#  error A private ava6 header was included when not building the library or private unit test code.
#endif

#include "base/ava6config.h"
#include "ava6_public.h"

#endif /* AVA6_PRIVATE_H */
