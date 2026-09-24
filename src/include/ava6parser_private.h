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

#ifndef AVA6PARSER_PRIVATE_H
#define AVA6PARSER_PRIVATE_H

#if !(defined(__BUILDING_AVA6PARSERLIB) \
      || defined(__BUILDING_AVA6PARSERLIB_UNIT_TEST))
#  error A private ava6 parser header was included when not building the parser library or private unit test code.
#endif

#include "ava6parser_public.h"
// It would be nice to #include "base/ava6config.h" here, but there are
// conflicts with antlr3's autoheader stuff, which they export :(
//
// #include "base/ava6config.h"

#endif /* AVA6PARSER_PRIVATE_H */
