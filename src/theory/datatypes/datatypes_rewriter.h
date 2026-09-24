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
  DatatypesRewriter(NodeManager* nm, Evaluator* sygusEval, const Options& opts);
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
  /**
   * Expand a nullable lift term with an ite expression.
   * Example:
   * input : (nullable.lift f x y) where f is a function
   *         and x,y are nullable terms.
   * output: (ite
   *           (or (nullable.is_null x) (nullable.is_null y))
   *           (nullable.null)
   *           (f (nullable.val x) (nullable.val y))
   *         )
   * @pre Higher-order logic is enabled.
   * @param n A nullable lift term.
   * @return An ite expression.
   */
  Node expandNullableLift(Node n);

  /**
   * Rewrite nullable lift terms as null if any of the arguments is null,
   * or return the some of applying the function (first child) to values
   * if all arguments are some constants.
   * - input : (nullable.lift f x1 ... (nullable.null) ... xn))
   *   output: (nullable.null)
   * - input : (nullable.lift f (nullable.some c1) ... (nullable.some cn))
   *   output: (f c1 ... cn)
   */
  RewriteResponse rewriteNullableLift(TNode n);

 private:
  /** rewrite constructor term in */
  RewriteResponse rewriteConstructor(TNode in);
  /** rewrite selector term in */
  RewriteResponse rewriteSelector(TNode in);
  /** rewrite tester term in */
  RewriteResponse rewriteTester(TNode in);
  /** rewrite updater term in */
  RewriteResponse rewriteUpdater(TNode in);

  /** Sygus to builtin eval
   *
   * This method returns the rewritten form of (DT_SYGUS_EVAL n args). Notice
   * that n does not necessarily need to be a constant.
   *
   * It does so by (1) converting constant subterms of n to builtin terms and
   * evaluating them on the arguments args, (2) unfolding non-constant
   * applications of sygus constructors in n with respect to args and (3)
   * converting all other non-constant subterms of n to applications of
   * DT_SYGUS_EVAL.
   *
   * For example, if
   *   n = C_+( C_*( C_x(), C_y() ), n' ), and args = { 3, 4 }
   * where n' is a variable, then this method returns:
   *   12 + (DT_SYGUS_EVAL n' 3 4)
   * Notice that the subterm C_*( C_x(), C_y() ) is converted to its builtin
   * equivalent x*y and evaluated under the substition { x -> 3, y -> 4 } giving
   * 12. The subterm n' is non-constant and thus we return its evaluation under
   * 3,4, giving the term (DT_SYGUS_EVAL n' 3 4). Since the top-level
   * constructor is C_+, these terms are added together to give the result.
   */
  Node sygusToBuiltinEval(Node n, const std::vector<Node>& args);
  /** Pointer to the evaluator, used as an optimization for the above method */
  Evaluator* d_sygusEval;
  /** Reference to the options */
  const Options& d_opts;
};

}  // namespace datatypes
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__DATATYPES__DATATYPES_REWRITER_H */
