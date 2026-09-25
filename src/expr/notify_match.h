/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Notifications for term matches.
 */

#include "ava6_private.h"

#ifndef AVA6__EXPR__NOTIFY_MATCH_H
#define AVA6__EXPR__NOTIFY_MATCH_H

#include <vector>

#include "expr/node.h"

namespace ava6::internal {
namespace expr {

/** A virtual class for notifications regarding matches. */
class NotifyMatch
{
 public:
  virtual ~NotifyMatch() {}
  /**
   * A notification that s is equal to n * { vars -> subs }. This function
   * should return false if we do not wish to be notified of further matches.
   */
  virtual bool notify(Node s,
                      Node n,
                      std::vector<Node>& vars,
                      std::vector<Node>& subs) = 0;
};

}  // namespace expr
}  // namespace ava6::internal

#endif /* AVA6__EXPR__NOTIFY_MATCH_H */
