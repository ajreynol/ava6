/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Rewriter of builtin theory.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__BUILTIN__THEORY_BUILTIN_REWRITER_H
#define AVA6__THEORY__BUILTIN__THEORY_BUILTIN_REWRITER_H

#include "theory/theory_rewriter.h"

namespace ava6::internal {
namespace theory {
namespace builtin {

class TheoryBuiltinRewriter : public TheoryRewriter
{
 public:
  TheoryBuiltinRewriter(NodeManager* nm);

  RewriteResponse postRewrite(TNode node) override;

  RewriteResponse preRewrite(TNode node) override;

 public:
  /**
   * The default rewriter for rewrites that occur at both pre and post rewrite.
   */
  RewriteResponse doRewrite(TNode node);
  /**
   * Main entry point for rewriting terms of the form (witness ((x T)) (P x)).
   * Returns the rewritten form of node.
   */
  Node rewriteWitness(TNode node);
  /**
   * Main entry point for rewriting APPLY_INDEXED_SYMBOLIC terms.
   */
  static Node rewriteApplyIndexedSymbolic(TNode node);
}; /* class TheoryBuiltinRewriter */

}  // namespace builtin
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__BUILTIN__THEORY_BUILTIN_REWRITER_H */
