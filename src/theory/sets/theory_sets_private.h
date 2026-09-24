/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Sets theory implementation.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__SETS__THEORY_SETS_PRIVATE_H
#define AVA6__THEORY__SETS__THEORY_SETS_PRIVATE_H

#include "context/cdhashset.h"
#include "context/cdqueue.h"
#include "expr/node_trie.h"
#include "smt/env_obj.h"
#include "theory/care_pair_argument_callback.h"
#include "theory/sets/inference_manager.h"
#include "theory/sets/solver_state.h"
#include "theory/sets/strategy.h"
#include "theory/sets/term_registry.h"
#include "theory/theory.h"
#include "theory/uf/equality_engine.h"

namespace ava6::internal {
namespace theory {
namespace sets {

/** Internal classes, forward declared here */
class TheorySets;

class TheorySetsPrivate : protected EnvObj
{
  typedef context::CDHashMap<Node, bool> NodeBoolMap;
  typedef context::CDHashSet<Node> NodeSet;

 public:
  void eqNotifyNewClass(TNode t);
  void eqNotifyMerge(TNode t1, TNode t2);
  void eqNotifyDisequal(TNode t1, TNode t2, TNode reason);

 private:
  /**
   * This implements an inference schema based on the "downwards closure" of
   * set membership. This roughly corresponds to the rules SET_UNION DOWN I and
   * II, INTER DOWN I and II from Bansal et al IJCAR 2016, as well as rules for
   * set difference.
   */
  void checkDownwardsClosure();
  /**
   * This implements an inference schema based on the "upwards closure" of
   * set membership. This roughly corresponds to the rules SET_UNION UP, INTER
   * UP I and II from Bansal et al IJCAR 2016, as well as rules for set
   * difference.
   */
  void checkUpwardsClosure();

  /**
   * Apply the following rule for filter terms (set.filter p A):
   * (=>
   *   (and (set.member x B) (= A B))
   *   (= (set.member x (set.filter p A)) (p x))
   * )
   */
  void checkFilterUp();
  /**
   * Apply the following rule for filter terms (set.filter p A):
   * (=>
   *   (set.member x (set.filter p A))
   *   (and
   *    (p x)
   *    (set.member x A)
   *   )
   * )
   */
  void checkFilterDown();
  /**
   * Apply the following rule for map terms (set.map f A):
   * Positive member rule:
   * (=>
   *   (set.member x A)
   *   (set.member (f x) (set.map f A)
   * )
   */
  void checkMapUp();
  /**
   * Apply the following rules for map terms (set.map f A) where A has type
   * (Set T):
   *   (=>
   *     (set.member y (set.map f A))
   *     (and
   *       (= (f x) y)
   *       (set.member x A)
   *     )
   *   )
   *   where x is a fresh skolem
   */
  void checkMapDown();
  Node d_true;
  Node d_false;
  Node d_zero;
  NodeBoolMap d_deq;
  /**
   * The set of terms that we have reduced via a lemma in the current user
   * context.
   */
  NodeSet d_termProcessed;

  // propagation
  class EqcInfo
  {
   public:
    EqcInfo(context::Context* c);
    ~EqcInfo() {}
    // singleton or emptyset equal to this eqc
    context::CDO<Node> d_singleton;
  };
  /** information necessary for equivalence classes */
  std::map<Node, EqcInfo*> d_eqc_info;
  /** get or make eqc info */
  EqcInfo* getOrMakeEqcInfo(TNode n, bool doMake = false);

  /** full check incomplete
   *
   * This flag is set to true during a full effort check if this theory
   * is incomplete for some reason (for instance, if we combine cardinality
   * with a relation or extended function kind).
   */
  bool d_fullCheckIncomplete;
  /** The reason we set the above flag to true */
  IncompleteId d_fullCheckIncompleteId;

 public:
  /**
   * Constructs a new instance of TheorySetsPrivate w.r.t. the provided
   * contexts.
   */
  TheorySetsPrivate(Env& env,
                    TheorySets& external,
                    SolverState& state,
                    InferenceManager& im,
                    SkolemCache& skc,
                    CarePairArgumentCallback& cpacb);

  ~TheorySetsPrivate();

  /** Get the solver state */
  SolverState* getSolverState() { return &d_state; }

  /**
   * Finish initialize, called after the equality engine of theory sets has
   * been determined.
   */
  void finishInit();

  //--------------------------------- standard check
  /** Post-check, called after the fact queue of the theory is processed. */
  void postCheck(Theory::Effort level);
  /** Notify new fact */
  void notifyFact(TNode atom, bool polarity, TNode fact);
  //--------------------------------- end standard check

  //--------------------------------- strategy steps
  // These are the individual steps of the full-effort strategy. They are
  // invoked by sets::Strategy::runStep in the order set up by
  // Strategy::initializeStrategy. Each step asserts facts directly and/or
  // buffers lemmas; the strategy decides when to flush and when to iterate.
  /**
   * Reset the per-pass full-effort state (solver state, inference manager,
   * cardinality solver and incompleteness flags). Runs first on every strategy
   * pass, before checkBasic registers terms.
   */
  void fullEffortReset();
  /**
   * Register the relevant terms with the solver state and run the membership
   * downwards/upwards closure schemas. Returns as soon as a fact or lemma has
   * been produced, so the strategy can flush and restart.
   */
  void checkBasic();
  /** Run the set.filter inference rules (checkFilterUp / checkFilterDown). */
  void checkFilters();
  /** Run the set.map inference rules (checkMapUp / checkMapDown). */
  void checkMaps();
  /**
   * Split on set disequalities (SET DISEQUALITY rule from Bansal et al IJCAR
   * 2016). Runs after the operator rules to preserve the original inference
   * order; running it earlier slows finite model finding (see strategy order).
   */
  void checkDisequalities();
  /** Add reduction lemmas for all set comprehensions in the current context. */
  void checkReduceComprehensions();
  //--------------------------------- end strategy steps

  /** Collect model values in m based on the relevant terms given by termSet */
  bool collectModelValues(TheoryModel* m, const std::set<Node>& termSet);

  void computeCareGraph();

  void preRegisterTerm(TNode node);

  /** ppRewrite, which expands choose and is_singleton.  */
  TrustNode ppRewrite(Node n, std::vector<SkolemLemma>& lems);

  void presolve();

  /** get the valuation */
  Valuation& getValuation();
  /** Is formula n entailed to have polarity pol in the current context? */
  bool isEntailed(Node n, bool pol);

  /**
   * Adds inferences for splitting on arguments of a and b that are not
   * equal nor disequal and are sets.
   */
  void processCarePairArgs(TNode a, TNode b);

  /** returns whether the given kind is a higher order kind for sets. */
  bool isHigherOrderKind(Kind k);

 private:
  TheorySets& d_external;
  /** The state of the sets solver at full effort */
  SolverState& d_state;
  /** The inference manager of the sets solver */
  InferenceManager& d_im;
  /** The term registry */
  TermRegistry d_treg;

  /** Pointer to the equality engine of theory of sets */
  eq::EqualityEngine* d_equalityEngine;

  bool isCareArg(Node n, unsigned a);

  /** expand the definition of the choose operator */
  TrustNode expandChooseOperator(const Node& node,
                                 std::vector<SkolemLemma>& lems);
  /** expand the definition of is_singleton operator */
  TrustNode expandIsSingletonOperator(const Node& node);
  /** ensure that the set type is over first class type, throw logic exception
   * if not */
  void ensureFirstClassSetType(TypeNode tn) const;
  /** subtheory solver for the theory of relations */
  /** subtheory solver for the theory of sets with cardinality */
  /** Have we ever seen relations? */
  /** are relations enabled?
   *
   * This flag is set to true during a full effort check if any constraint
   * involving relational constraints is asserted to this theory.
   */
  /** Have we ever seen cardinality? */
  /** is cardinality enabled?
   *
   * This flag is set to true during a full effort check if any constraint
   * involving cardinality constraints is asserted to this theory.
   */

  /** are higher order set operators enabled?
   *
   * This flag is set to true during a full effort check if any
   * higher order constraints is asserted to this theory.
   */
  bool d_higher_order_kinds_enabled;

  /** a map that maps each set to an existential quantifier generated for
   * operator is_singleton */
  std::map<Node, Node> d_isSingletonNodes;
  /** Reference to care pair argument callback, used for theory combination */
  CarePairArgumentCallback& d_cpacb;
  /**
   * The relevant terms for the current full-effort check. Collected once per
   * postCheck and reused by checkBasic while registering terms on each strategy
   * pass (mirrors the hoist that used to live at the top of fullEffortCheck).
   */
  std::set<Node> d_relevantTerms;
  /** The strategy that drives the full-effort check loop. */
  Strategy d_strategy;
}; /* class TheorySetsPrivate */

}  // namespace sets
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__SETS__THEORY_SETS_PRIVATE_H */
