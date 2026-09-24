/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Quotes a string if necessary for smt2.
 */

#include "ava6_private.h"

#ifndef AVA6__UTIL__SMT2_QUOTE_STRING_H
#define AVA6__UTIL__SMT2_QUOTE_STRING_H

#include <ava6/ava6_export.h>

#include <string>

namespace ava6::internal {

/**
 * SMT-LIB 2 quoting for symbols
 */
std::string quoteSymbol(const std::string& s);

/**
 * SMT-LIB 2 quoting for strings
 */
std::string quoteString(const std::string& s) AVA6_EXPORT;

}  // namespace ava6::internal

#endif /* AVA6__UTIL__SMT2_QUOTE_STRING_H */
