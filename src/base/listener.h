/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Utilities for the development of a Listener interface class.
 *
 * This class provides a single notification that must be overwritten.
 */

#include "ava6_public.h"

#ifndef AVA6__LISTENER_H
#define AVA6__LISTENER_H

namespace ava6::internal {

/**
 * Listener interface class.
 *
 * The interface provides a notify() function.
 */
class Listener
{
 public:
  Listener();
  virtual ~Listener();

  /** Note that notify may throw arbitrary exceptions. */
  virtual void notify() = 0;
};

}  // namespace ava6::internal

#endif /* AVA6__LISTENER_H */
