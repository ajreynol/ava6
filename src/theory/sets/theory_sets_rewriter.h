/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Sets theory rewriter.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__SETS__THEORY_SETS_REWRITER_H
#define AVA6__THEORY__SETS__THEORY_SETS_REWRITER_H

#include "theory/rewriter.h"

namespace ava6::internal {
namespace theory {
namespace sets {

class TheorySetsRewriter : public TheoryRewriter
{
 public:
  TheorySetsRewriter(NodeManager* nm,
                     bool cardEnabled = true,
                     bool relsEnabled = true);

  /**
   * Rewrite n based on the proof rewrite rule id.
   * @param id The rewrite rule.
   * @param n The node to rewrite.
   * @return The rewritten version of n based on id, or Node::null() if n
   * cannot be rewritten.
   */
  Node rewriteViaRule(ProofRewriteRule id, const Node& n) override;

  /**
   * Rewrite a node into the normal form for the theory of sets.
   * Called in post-order (really reverse-topological order) when
   * traversing the expression DAG during rewriting.  This is the
   * main function of the rewriter, and because of the ordering,
   * it can assume its children are all rewritten already.
   *
   * This function can return one of three rewrite response codes
   * along with the rewritten node:
   *
   *   REWRITE_DONE indicates that no more rewriting is needed.
   *   REWRITE_AGAIN means that the top-level expression should be
   *     rewritten again, but that its children are in final form.
   *   REWRITE_AGAIN_FULL means that the entire returned expression
   *     should be rewritten again (top-down with preRewrite(), then
   *     bottom-up with postRewrite()).
   *
   * Even if this function returns REWRITE_DONE, if the returned
   * expression belongs to a different theory, it will be fully
   * rewritten by that theory's rewriter.
   */
  RewriteResponse postRewrite(TNode node) override;

  /**
   * Rewrite a node into the normal form for the theory of sets
   * in pre-order (really topological order)---meaning that the
   * children may not be in the normal form.  This is an optimization
   * for theories with cancelling terms (e.g., 0 * (big-nasty-expression)
   * in arithmetic rewrites to 0 without the need to look at the big
   * nasty expression).  Since it's only an optimization, the
   * implementation here can do nothing.
   */
  RewriteResponse preRewrite(TNode node) override;

  /**
   * Rewrite an equality, in case special handling is required.
   */
  Node rewriteEquality(TNode equality)
  {
    // often this will suffice
    return postRewrite(equality).d_node;
  }

  /**
   * Rewrite membership for a binary op.
   * For example, if mem is (set.member x (set.inter A B)), the returns the
   * formula (and (set.member x A) (set.member x B)).
   * @param mem The membership.
   * @return The rewritten form of the membership.
   */
  Node rewriteMembershipBinaryOp(const Node& mem);

 private:
  /**
   * Returns true if elementTerm is in setTerm, where both terms are constants.
   */
  bool checkConstantMembership(TNode elementTerm, TNode setTerm);
  /**
   * Rewrite set comprehension
   */
  RewriteResponse postRewriteComprehension(TNode n);
  /** Is sets+cardinality enabled? */
  bool d_cardEnabled;
  /** Are relations enabled? */
  bool d_relsEnabled;
}; /* class TheorySetsRewriter */

}  // namespace sets
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__SETS__THEORY_SETS_REWRITER_H */
