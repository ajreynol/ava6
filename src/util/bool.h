/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * A hash function for Boolean.
 */

#include "ava6_public.h"

#ifndef AVA6__BOOL_H
#define AVA6__BOOL_H

namespace ava6::internal {

struct BoolHashFunction
{
  inline size_t operator()(bool b) const { return b; }
}; /* struct BoolHashFunction */

}  // namespace ava6::internal

#endif /* AVA6__BOOL_H */
