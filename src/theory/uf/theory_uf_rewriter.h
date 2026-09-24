/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * [[ Add one-line brief description here ]]
 *
 * [[ Add lengthier description here ]]
 * \todo document this file
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__UF__THEORY_UF_REWRITER_H
#define AVA6__THEORY__UF__THEORY_UF_REWRITER_H

#include "expr/node_algorithm.h"
#include "options/uf_options.h"
#include "theory/rewriter.h"
#include "theory/substitutions.h"
#include "theory/theory_rewriter.h"

namespace ava6::internal {
namespace theory {
namespace uf {

class TheoryUfRewriter : public TheoryRewriter
{
 public:
  TheoryUfRewriter(NodeManager* nm);
  /** post-rewrite */
  RewriteResponse postRewrite(TNode node) override;
  /** pre-rewrite */
  RewriteResponse preRewrite(TNode node) override;
  /**
   * Rewrite n based on the proof rewrite rule id.
   * @param id The rewrite rule.
   * @param n The node to rewrite.
   * @return The rewritten version of n based on id, or Node::null() if n
   * cannot be rewritten.
   */
  Node rewriteViaRule(ProofRewriteRule id, const Node& n) override;
  /**
   * Can we eliminate the lambda n? This is true if n is of the form
   * (LAMBDA x (APPLY_UF f x)), which is equivalent to f.
   * @param n The lambda in question.
   * @return the result of eliminating n, if possible, or null otherwise.
   */
  static Node canEliminateLambda(NodeManager* nm, const Node& n);
  /**
   * Blast distinct, which eliminates the distinct operator.
   */
  static Node blastDistinct(NodeManager* nm, TNode node);

 private:
  /** Entry point for rewriting lambdas */
  Node rewriteLambda(Node node);
  /** rewrite ubv_to_int */
  RewriteResponse rewriteBVToInt(TNode node);
  /** rewrite int_to_bv */
  RewriteResponse rewriteIntToBV(TNode node);
  /** rewrite distinct */
  RewriteResponse rewriteDistinct(TNode node);
}; /* class TheoryUfRewriter */

}  // namespace uf
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__UF__THEORY_UF_REWRITER_H */
