/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Generated rewrites
 */

#include "ava6_private.h"

#ifndef AVA6__REWRITER__REWRITES__H
#define AVA6__REWRITER__REWRITES__H

#include "ava6/ava6_proof_rule.h"
#include "expr/node.h"

namespace ava6::internal {

namespace rewriter {

class RewriteDb;

/**
 * The body of this method is auto-generated. This populates the provided
 * rewrite rule database with rules based on the compilation of the DSL
 * rewrite rule files.
 */
void addRules(NodeManager* nm, RewriteDb& db);

/** Make node from proof rewrite rule */
Node mkRewriteRuleNode(NodeManager* nm, ProofRewriteRule rule);

/** get a proof rewrite rule from a node, return false if we fail */
bool getRewriteRule(TNode n, ProofRewriteRule& rule);

}  // namespace rewriter
}  // namespace ava6::internal

#endif
