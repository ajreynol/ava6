/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Model class.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__THEORY_MODEL_BUILDER_H
#define AVA6__THEORY__THEORY_MODEL_BUILDER_H

#include <unordered_map>
#include <unordered_set>

#include "smt/env_obj.h"
#include "theory/theory_model.h"

namespace ava6::internal {

class Env;

namespace theory {

/** TheoryEngineModelBuilder class
 *
 * This is the class used by TheoryEngine
 * for constructing TheoryModel objects, which is the class
 * that represents models for a set of assertions.
 *
 * A call to TheoryEngineModelBuilder::buildModel(...) is made
 * after a full effort check passes with no theory solvers
 * adding lemmas or conflicts, and theory combination passes
 * with no splits on shared terms. If buildModel is successful,
 * this will set up the data structures in TheoryModel to represent
 * a model for the current set of assertions.
 */
class TheoryEngineModelBuilder : protected EnvObj
{
  typedef std::unordered_map<Node, Node> NodeMap;
  typedef std::unordered_set<Node> NodeSet;

 public:
  TheoryEngineModelBuilder(Env& env);
  virtual ~TheoryEngineModelBuilder() {}
  /**
   * Should be called only on models m after they have been prepared
   * (e.g. using ModelManager). In other words, the equality engine of model
   * m contains all relevant information from each theory that is needed
   * for building a model. This class is responsible simply for ensuring
   * that all equivalence classes of the equality engine of m are assigned
   * constants.
   *
   * This constructs the model m, via the following steps:
   * (1) builder-specified pre-processing,
   * (2) find the equivalence classes of m's
   *     equality engine that initially contain constants,
   * (3) assign constants to all equivalence classes
   *     of m's equality engine, through alternating
   *     iterations of evaluation and enumeration,
   * (4) builder-specific processing, which includes assigning total
   *     interpretations to uninterpreted functions.
   *
   * This function returns false if any of the above
   * steps results in a lemma sent on an output channel.
   * Lemmas may be sent on an output channel by this
   * builder in steps (2) or (5), for instance, if the model we
   * are building fails to satisfy a quantified formula.
   *
   * @param m The model to build
   * @return true if the model was successfully built.
   */
  bool buildModel(TheoryModel* m);

  /** postprocess model
   *
   * This is called when m is a model that will be returned to the user. This
   * method checks the internal consistency of the model if we are in a debug
   * build.
   */
  void postProcessModel(bool incomplete, TheoryModel* m);

 protected:
  //-----------------------------------virtual functions
  /** pre-process build model
   * Do pre-processing specific to this model builder.
   * Called in step (2) of the build construction,
   * described above.
   */
  virtual bool preProcessBuildModel(TheoryModel* m);
  /** process build model
   * Do processing specific to this model builder.
   * Called in step (5) of the build construction,
   * described above.
   * By default, this assigns values to each function
   * that appears in m's equality engine.
   */
  virtual bool processBuildModel(TheoryModel* m);
  /** debug the model
   * Check assertions and printing debug information for the model.
   * Calls after step (5) described above is complete.
   */
  virtual void debugModel(AVA6_UNUSED TheoryModel* m) {}
  //-----------------------------------end virtual functions

  /** Debug check model.
   *
   * This throws an assertion failure if the model contains an equivalence
   * class with two terms t1 and t2 such that t1^M != t2^M.
   */
  void debugCheckModel(TheoryModel* m);

  /** Evaluate equivalence class
   *
   * If this method returns a non-null node c, then c is a constant and some
   * term in the equivalence class of r evaluates to c based on the current
   * state of the model m.
   */
  Node evaluateEqc(TheoryModel* m, TNode r);
  /** is n an assignable expression?
   *
   * A term n is an assignable expression if its value is unconstrained by a
   * standard model. Examples of assignable terms are:
   * - variables,
   * - applications of array select,
   * - applications of datatype selectors,
   * - applications of uninterpreted functions.
   * Assignable terms must be first-order, that is, all instances of the above
   * terms are not assignable if they have a higher-order (function) type.
   */
  bool isAssignable(TNode n);
  /** add assignable subterms
   * Adds all assignable subterms of n to tm's equality engine.
   */
  void addAssignableSubterms(TNode n, TheoryModel* tm, NodeSet& cache);
  /** normalize representative r
   *
   * This returns a term that is equivalent to r's
   * interpretation in the model m. It may do so
   * by rewriting the application of r's operator to the
   * result of normalizing each of r's children, if
   * each child is constant.
   */
  Node normalize(TheoryModel* m, TNode r, bool evalOnly);
  /** assign constant representative
   *
   * Called when equivalence class eqc is assigned a constant
   * representative const_rep.
   *
   * eqc should be a representative of tm's equality engine.
   */
  void assignConstantRep(TheoryModel* tm, Node eqc, Node const_rep);
  /** add to type list
   *
   * This adds to type_list the list of types that tn is built from.
   * For example, if tn is (Array Int Bool) and type_list is empty,
   * then we append ( Int, Bool, (Array Int Bool) ) to type_list.
   */
  void addToTypeList(TypeNode tn,
                     std::vector<TypeNode>& type_list,
                     std::unordered_set<TypeNode>& visiting);

 private:
  /** normalized cache
   * A temporary cache mapping terms to their
   * normalized form, used during buildModel.
   */
  NodeMap d_normalizedCache;
  /** mapping from terms to the constant associated with their equivalence class
   */
  std::map<Node, Node> d_constantReps;

}; /* class TheoryEngineModelBuilder */

}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__THEORY_MODEL_BUILDER_H */
