/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The theory of uninterpreted functions (UF)
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__UF__THEORY_UF_H
#define AVA6__THEORY__UF__THEORY_UF_H

#include "expr/node.h"
#include "theory/care_pair_argument_callback.h"
#include "theory/theory.h"
#include "theory/theory_eq_notify.h"
#include "theory/theory_state.h"
#include "theory/uf/diamonds_proof_generator.h"
#include "theory/uf/distinct_extension.h"
#include "theory/uf/proof_checker.h"
#include "theory/uf/theory_uf_rewriter.h"

namespace ava6::internal {
namespace theory {
namespace uf {

class ConversionsSolver;

class TheoryUF : public Theory
{
 public:
  class NotifyClass : public TheoryEqNotifyClass
  {
   public:
    NotifyClass(TheoryInferenceManager& im, TheoryUF& uf)
        : TheoryEqNotifyClass(im), d_uf(uf)
    {
    }


    void eqNotifyMerge(TNode t1, TNode t2) override
    {
      Trace("uf-notify") << "NotifyClass::eqNotifyMerge(" << t1 << ", " << t2
                         << ")" << std::endl;
      d_uf.eqNotifyMerge(t1, t2);
    }


   private:
    /** Reference to the parent theory */
    TheoryUF& d_uf;
  }; /* class TheoryUF::NotifyClass */

 private:
  /** the conversions solver */
  std::unique_ptr<ConversionsSolver> d_csolver;
  /** Diamonds proof generator */
  DiamondsProofGenerator d_dpfgen;

  /** node for true */
  Node d_true;

  /** All the function terms that the theory has seen */
  context::CDList<TNode> d_functionsTerms;


  /** called when two equivalance classes have merged */
  void eqNotifyMerge(TNode t1, TNode t2);

 public:
  /** Constructs a new instance of TheoryUF w.r.t. the provided context.*/
  TheoryUF(Env& env,
           OutputChannel& out,
           Valuation valuation,
           std::string instanceName = "");

  ~TheoryUF();

  //--------------------------------- initialization
  /** get the official theory rewriter of this theory */
  TheoryRewriter* getTheoryRewriter() override;
  /** get the proof checker of this theory */
  ProofRuleChecker* getProofChecker() override;
  /**
   * Returns true if we need an equality engine. If so, we initialize the
   * information regarding how it should be setup. For details, see the
   * documentation in Theory::needsEqualityEngine.
   */
  bool needsEqualityEngine(EeSetupInfo& esi) override;
  /** finish initialization */
  void finishInit() override;
  //--------------------------------- end initialization

  //--------------------------------- standard check
  /** Do we need a check call at last call effort? */
  bool needsCheckLastEffort() override;
  /** Post-check, called after the fact queue of the theory is processed. */
  void postCheck(Effort level) override;
  /** Notify fact */
  void notifyFact(TNode atom, bool pol, TNode fact, bool isInternal) override;
  //--------------------------------- end standard check

  TrustNode ppRewrite(TNode node, std::vector<SkolemLemma>& lems) override;
  void preRegisterTerm(TNode term) override;
  TrustNode explain(TNode n) override;

  void ppStaticLearn(TNode in, std::vector<TrustNode>& learned) override;
  void presolve() override;

  void computeCareGraph() override;

  EqualityStatus getEqualityStatus(TNode a, TNode b) override;

  std::string identify() const override { return "THEORY_UF"; }

 private:
  /** Called when preregistering function applications */
  void preRegisterFunctionTerm(TNode node);
  /** Explain why this literal is true by building an explanation */
  void explain(TNode literal, Node& exp);

  TheoryUfRewriter d_rewriter;
  /** Proof rule checker */
  UfProofRuleChecker d_checker;
  /** A (default) theory state object */
  TheoryState d_state;
  /** A (default) inference manager */
  TheoryInferenceManager d_im;
  /** the distinct extension */
  DistinctExtension d_distinct;
  /** The notify class */
  NotifyClass d_notify;
  /** The care pair argument callback, used for theory combination */
  CarePairArgumentCallback d_cpacb;
}; /* class TheoryUF */

}  // namespace uf
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__UF__THEORY_UF_H */
