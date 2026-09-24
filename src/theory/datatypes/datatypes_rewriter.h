/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Rewriter for the theory of (co)inductive datatypes.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__DATATYPES__DATATYPES_REWRITER_H
#define AVA6__THEORY__DATATYPES__DATATYPES_REWRITER_H

#include "theory/evaluator.h"
#include "theory/theory_rewriter.h"

namespace ava6::internal {

class Options;

namespace theory {
namespace datatypes {

/**
 * The rewriter for datatypes. An invariant of the rewriter is that
 * postRewrite/preRewrite should not depend on the options, in particular,
 * they should not depend on whether shared selectors are enabled. Thus,
 * they should not use DTypeConstructor::getSelectorInternal. Instead,
 * the conversion from external to internal selectors is done in
 * expandDefinition. This invariant ensures that the rewritten form of a node
 * does not mix multiple option settings, which would lead to e.g. shared
 * selectors being used in an SolverEngine instance where they are disabled.
 */
class DatatypesRewriter : public TheoryRewriter
{
 public:
  explicit DatatypesRewriter(NodeManager* nm);
  RewriteResponse postRewrite(TNode in) override;
  RewriteResponse preRewrite(TNode in) override;

  /**
   * Rewrite n based on the proof rewrite rule id.
   * @param id The rewrite rule.
   * @param n The node to rewrite.
   * @return The rewritten version of n based on id, or Node::null() if n
   * cannot be rewritten.
   */
  Node rewriteViaRule(ProofRewriteRule id, const Node& n) override;

  /**
   * Expand updater term. Given n = (APPLY_UPDATER{SELECTOR_k} t s), this method
   * returns (ITE (APPLY_TESTER{C} t) (C (APPLY_SELECTOR SELECTOR_1
   * t)...s...(APPLY_SELECTOR SELECTOR_m t)) t). where 1 <= k <= m.
   */
  Node expandUpdater(const Node& n);
  /**
   * Expand a match term into its definition.
   * For example
   *   (MATCH x (((APPLY_CONSTRUCTOR CONS y z) z) (APPLY_CONSTRUCTOR NIL x)))
   * returns
   *   (ITE (APPLY_TESTER CONS x) (APPLY_SELECTOR x) x)
   */
  static Node expandMatch(Node n);
  /** expand defintions */
  Node expandDefinition(Node n) override;
 private:
  /** rewrite constructor term in */
  RewriteResponse rewriteConstructor(TNode in);
  /** rewrite selector term in */
  RewriteResponse rewriteSelector(TNode in);
  /** rewrite tester term in */
  RewriteResponse rewriteTester(TNode in);
  /** rewrite updater term in */
  RewriteResponse rewriteUpdater(TNode in);

};

}  // namespace datatypes
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__DATATYPES__DATATYPES_REWRITER_H */
