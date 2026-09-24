/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Utilities for printing API enum values.
 *
 * In particular, we require `toString(...)` to be defined for any identifier
 * that may appear in statistics, for safe printing (src/util/safe_print.h).
 *
 * These print methods should also be used for printing enums at the API
 * level, where `std::string to_string(...)` should be declared for that enum
 * and call `toString(...)` as defined in this file.
 */

#include "ava6_private.h"

#ifndef AVA6__PRINTER__ENUM_TO_STRING_H
#define AVA6__PRINTER__ENUM_TO_STRING_H

#include <ava6/ava6_skolem_id.h>

namespace ava6::internal {

/**
 * Converts a skolem identifier to a string. Note: This function is also used in
 * `safe_print()`. Changing this function name or signature will result in
 * `safe_print()` printing "<unsupported>" instead of the proper strings for
 * the enum values.
 *
 * @param id The proof rule
 * @return The name of the proof rule
 */
const char* toString(ava6::SkolemId id);

}  // namespace ava6::internal

#endif
