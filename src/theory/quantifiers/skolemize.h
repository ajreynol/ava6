/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Utilities for skolemization.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__QUANTIFIERS__SKOLEMIZE_H
#define AVA6__THEORY__QUANTIFIERS__SKOLEMIZE_H

#include <unordered_map>
#include <unordered_set>

#include "context/cdhashmap.h"
#include "expr/node.h"
#include "expr/type_node.h"
#include "proof/eager_proof_generator.h"
#include "proof/trust_node.h"
#include "smt/env_obj.h"

namespace ava6::internal {

class DTypeConstructor;

namespace theory {
namespace quantifiers {

class QuantifiersState;
class TermRegistry;

/** Skolemization utility
 *
 * This class constructs Skolemization lemmas.
 * Given a quantified formula q = (forall x. P),
 * its skolemization lemma is of the form:
 *   (~ forall x. P ) => ~P * { x -> d_skolem_constants[q] }
 *
 * This class also incorporates techniques for
 * skolemization with "inductive strenghtening", see
 * Section 2 of Reynolds et al., "Induction for SMT
 * Solvers", VMCAI 2015. In the case that x is an inductive
 * datatype or an integer, then we may strengthen the conclusion
 * based on weak well-founded induction. For example, for
 * quantification on lists, a skolemization with inductive
 * strengthening is a lemma of this form:
 *   (~ forall x : List. P( x ) ) =>
 *   ~P( k ) ^ ( is-cons( k ) => P( tail( k ) ) )
 * For the integers it is:
 *   (~ forall x : Int. P( x ) ) =>
 *   ~P( k ) ^ ( x>0 => P( x-1 ) )
 *
 *
 * Inductive strenghtening is not enabled by
 * default and can be enabled by option:
 *   --quant-ind
 */
class Skolemize : protected EnvObj
{
  typedef context::CDHashMap<Node, Node> NodeNodeMap;

 public:
  Skolemize(Env& env, QuantifiersState& qs, TermRegistry& tr);
  ~Skolemize() {}
  /** skolemize quantified formula q
   * If the return value ret of this function is non-null, then ret is a trust
   * node corresponding to a new skolemization lemma we generated for q. These
   * lemmas are constructed once per user-context.
   */
  TrustNode process(Node q);
  /** get the skolem constants */
  static std::vector<Node> getSkolemConstants(const Node& q);
  /** get the i^th skolem constant for quantified formula q */
  static Node getSkolemConstant(const Node& q, size_t i);
  /** Substitute Skolem constants, or functions of the enclosing variables. */
  static Node mkSkolemizedBody(Node q, Node body,
                               const std::vector<TNode>& fvs,
                               std::vector<Node>& skolems);
  /**
   * Get skolemization vectors, where for each quantified formula that was
   * skolemized, this is the list of skolems that were used to witness the
   * negation of that quantified formula (which is equivalent to an existential
   * one).
   *
   * This is used for the command line option
   *   --dump-instantiations
   * which prints an informal justification of steps taken by the quantifiers
   * module.
   */
  void getSkolemTermVectors(std::map<Node, std::vector<Node>>& sks) const;

 private:
  /** Are proofs enabled? */
  bool isProofEnabled() const;
  /** Reference to the quantifiers state */
  QuantifiersState& d_qstate;
  /** Reference to the term registry */
  TermRegistry& d_treg;
  /** quantified formulas that have been skolemized */
  NodeNodeMap d_skolemized;
  /** map from quantified formulas to the list of skolem constants */
  std::unordered_map<Node, std::vector<Node>> d_skolem_constants;
  /** map from quantified formulas to their skolemized body */
  /** Eager proof generator for skolemization lemmas */
  std::unique_ptr<EagerProofGenerator> d_epg;
};

}  // namespace quantifiers
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__QUANTIFIERS__SKOLEMIZE_H */
