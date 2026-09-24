/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The default CleanUp class that does nothing.
 */

#include "ava6_public.h"

#ifndef AVA6__CONTEXT__DEFAULT_CLEAN_UP_H
#define AVA6__CONTEXT__DEFAULT_CLEAN_UP_H

#include <vector>

namespace ava6::context {

template <class T>
class DefaultCleanUp
{
 public:
  void operator()(typename std::vector<T>::reference) const {}
};

}  // namespace ava6::context

#endif /* AVA6__CONTEXT__DEFAULT_CLEAN_UP_H */
