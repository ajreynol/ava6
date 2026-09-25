/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Incompleteness enumeration.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__INCOMPLETE_ID_H
#define AVA6__THEORY__INCOMPLETE_ID_H

#include <iosfwd>

namespace ava6::internal {
namespace theory {

/**
 * Reasons for answering "unknown" in ava6.
 *
 * Note that this enumeration is used both for marking incomplete in a
 * SAT context, and marking unsound in a user context, both of which may
 * imply that we are "unknown".
 */
enum class IncompleteId
{
  // there is no marked incompleteness
  NONE,
  // the non-linear arithmetic solver was disabled
  ARITH_NL_DISABLED,
  // the non-linear arithmetic solver was incomplete
  ARITH_NL,
  // incomplete due to lack of a complete quantifiers strategy
  QUANTIFIERS,
  // incomplete due to counterexample-guided instantiation not being complete
  QUANTIFIERS_CEGQI,
  // incomplete due to bounded-quantifier model checking
  QUANTIFIERS_FMF,
  // incomplete due to limited number of allowed instantiation rounds
  QUANTIFIERS_MAX_INST_ROUNDS,
  // we skipped processing a looping word equation
  STRINGS_LOOP_SKIP,
  // we could not simplify a regular expression membership
  STRINGS_REGEXP_NO_SIMPLIFY,
  //------------------- other causes external to theories
  // unprocessed theory conflict
  UNPROCESSED_THEORY_CONFLICT,
  // the prop layer stopped search
  STOP_SEARCH,
  // due to preprocessing
  PREPROCESSING,
  //------------------- unknown
  // the reason for the incompleteness is unknown
  UNKNOWN
};

/**
 * Converts an incompleteness id to a string.
 *
 * @param i The incompleteness identifier
 * @return The name of the incompleteness identifier
 */
const char* toString(IncompleteId i);

/**
 * Writes an incompleteness identifier to a stream.
 *
 * @param out The stream to write to
 * @param i The incompleteness identifier to write to the stream
 * @return The stream
 */
std::ostream& operator<<(std::ostream& out, IncompleteId i);

}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__INCOMPLETE_ID_H */
