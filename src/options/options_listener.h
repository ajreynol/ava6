/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Base class for option listener.
 */

#include "ava6_private.h"

#ifndef AVA6__OPTIONS__OPTIONS_LISTENER_H
#define AVA6__OPTIONS__OPTIONS_LISTENER_H

#include <string>

namespace ava6::internal {

class OptionsListener
{
 public:
  OptionsListener() {}
  virtual ~OptionsListener() {}
  /**
   * Notify that option key has been set.
   */
  virtual void notifySetOption(const std::string& key) = 0;
};

}  // namespace ava6::internal

#endif /* AVA6__OPTIONS__OPTION_LISTENER_H */
