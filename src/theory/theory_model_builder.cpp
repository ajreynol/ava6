/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of theory model buidler class.
 */
#include "theory/theory_model_builder.h"

#include "expr/dtype.h"
#include "expr/dtype_cons.h"
#include "expr/skolem_manager.h"
#include "expr/sort_to_term.h"
#include "expr/sort_type_size.h"
#include "options/quantifiers_options.h"
#include "options/smt_options.h"
#include "options/strings_options.h"
#include "options/theory_options.h"
#include "options/uf_options.h"
#include "smt/env.h"
#include "theory/rewriter.h"
#include "theory/uf/function_const.h"
#include "theory/uf/theory_uf_model.h"
#include "util/uninterpreted_sort_value.h"

using namespace std;
using namespace ava6::internal::kind;
using namespace ava6::context;

namespace ava6::internal {
namespace theory {

TheoryEngineModelBuilder::TheoryEngineModelBuilder(Env& env) : EnvObj(env) {}

Node TheoryEngineModelBuilder::evaluateEqc(TheoryModel* m, TNode r)
{
  eq::EqClassIterator eqc_i = eq::EqClassIterator(r, m->d_equalityEngine);
  for (; !eqc_i.isFinished(); ++eqc_i)
  {
    Node n = *eqc_i;
    Trace("model-builder-debug") << "Look at term : " << n << std::endl;
    if (!isAssignable(n))
    {
      Trace("model-builder-debug") << "...try to normalize" << std::endl;
      Node normalized = normalize(m, n, true);
      Trace("model-builder-debug")
          << "...return " << normalized
          << ", isValue=" << m->isValue(normalized) << std::endl;
      if (m->isValue(normalized))
      {
        return normalized;
      }
    }
  }
  return Node::null();
}

bool TheoryEngineModelBuilder::isAssignable(TNode n)
{
  Kind k = n.getKind();
  if (k == Kind::SELECT || k == Kind::APPLY_SELECTOR || k == Kind::SEQ_NTH)
  {
    // selectors are always assignable (where we guarantee that they are not
    // evaluatable here)
    {
      Assert(!n.getType().isFunction());
      return true;
    }
  }
  else {
    // non-function variables, and fully applied functions
    {
      // no functions exist, all functions are fully applied
      Assert(true);
      Assert(!n.getType().isFunction());
      return n.isVar() || k == Kind::APPLY_UF;
    }
  }
}

void TheoryEngineModelBuilder::addAssignableSubterms(TNode n,
                                                     TheoryModel* tm,
                                                     NodeSet& cache)
{
  if (n.isClosure())
  {
    return;
  }
  if (cache.find(n) != cache.end())
  {
    return;
  }
  if (isAssignable(n))
  {
    tm->d_equalityEngine->addTerm(n);
  }
  for (TNode::iterator child_it = n.begin(); child_it != n.end(); ++child_it)
  {
    addAssignableSubterms(*child_it, tm, cache);
  }
  cache.insert(n);
}

void TheoryEngineModelBuilder::assignConstantRep(TheoryModel* tm,
                                                 Node eqc,
                                                 Node constRep)
{
  d_constantReps[eqc] = constRep;
  Trace("model-builder") << "    Assign: Setting constant rep of " << eqc
                         << " to " << constRep << endl;
  tm->d_rep_set.setTermForRepresentative(constRep, eqc);
}

void TheoryEngineModelBuilder::addToTypeList(
    TypeNode tn,
    std::vector<TypeNode>& type_list,
    std::unordered_set<TypeNode>& visiting)
{
  if (std::find(type_list.begin(), type_list.end(), tn) == type_list.end())
  {
    if (visiting.find(tn) == visiting.end())
    {
      visiting.insert(tn);
      /* This must make a recursive call on all types that are subterms of
       * values of the current type.
       * Note that recursive traversal here is over enumerated expressions
       * (very low expression depth). */
      if (tn.isArray())
      {
        addToTypeList(tn.getArrayIndexType(), type_list, visiting);
        addToTypeList(tn.getArrayConstituentType(), type_list, visiting);
      }
      else if (tn.isSet())
      {
        addToTypeList(tn.getSetElementType(), type_list, visiting);
      }
      else if (tn.isDatatype())
      {
        const DType& dt = tn.getDType();
        for (unsigned i = 0; i < dt.getNumConstructors(); i++)
        {
          // Note that we may be a parameteric datatype, in which case the
          // instantiated sorts need to be considered.
          TypeNode ctn = dt[i].getInstantiatedConstructorType(tn);
          for (const TypeNode& ctnc : ctn)
          {
            addToTypeList(ctnc, type_list, visiting);
          }
        }
      }
      Assert(std::find(type_list.begin(), type_list.end(), tn)
             == type_list.end());
      type_list.push_back(tn);
    }
  }
}

bool TheoryEngineModelBuilder::buildModel(TheoryModel* tm)
{
  Trace("model-builder") << "TheoryEngineModelBuilder: buildModel" << std::endl;

  Trace("model-builder")
      << "TheoryEngineModelBuilder: Preprocess build model..." << std::endl;
  // model-builder specific initialization
  if (!preProcessBuildModel(tm))
  {
    Trace("model-builder")
        << "TheoryEngineModelBuilder: fail preprocess build model."
        << std::endl;
    return false;
  }

  Trace("model-builder")
      << "TheoryEngineModelBuilder: Add assignable subterms "
         ", collect representatives and compute assignable information..."
      << std::endl;

  // In the first step of model building, we do a traversal of the
  // equality engine and record the information in the following:

  // The constant representatives, per equivalence class
  d_constantReps.clear();
  // The representatives that have been asserted by theories. This includes
  // non-constant "skeletons" that have been specified by parametric theories.
  std::map<Node, Node> assertedReps;
  // A parition of the set of equivalence classes that have:
  // (1) constant representatives,
  // (2) an assigned representative specified by a theory in collectModelInfo,
  // (3) no assigned representative.
  TypeSet typeConstSet, typeRepSet, typeNoRepSet;
  // An ordered list of types, such that T1 comes before T2 if T1 is a
  // "component type" of T2, e.g. U comes before (Set U). This is only strictly
  // necessary for finite model finding + parametric types instantiated with
  // uninterpreted sorts, but is probably a good idea to do in general since it
  // leads to models with smaller term sizes. -AJR
  std::vector<TypeNode> type_list;
  // the set of equivalence classes that are "assignable", i.e. those that have
  // an assignable expression in them (see isAssignable), and have not already
  // been assigned a constant.
  std::unordered_set<Node> assignableEqc;
  // The set of equivalence classes that are "evaluable", i.e. those that have
  // an expression in them that is not assignable, and have not already been
  // assigned a constant.
  std::unordered_set<Node> evaluableEqc;
  // Loop through equivalence classes of the equality engine of the model.
  eq::EqualityEngine* ee = tm->d_equalityEngine;
  NodeSet assignableCache;
  std::map<Node, Node>::iterator itm;
  eq::EqClassesIterator eqcs_i = eq::EqClassesIterator(ee);
  for (; !eqcs_i.isFinished(); ++eqcs_i)
  {
    Node eqc = *eqcs_i;
    Trace("model-builder") << "  Processing EQC " << eqc << std::endl;
    // Information computed for each equivalence class

    // The assigned represenative and constant representative
    Node rep, constRep;
    // is constant rep a "base model value" (see TheoryModel::isBaseModelValue)
    bool constRepBaseModelValue = false;
    // A flag set to true if the current equivalence class is assignable (see
    // assignableEqc).
    bool assignable = false;
    // Set to true if the current equivalence class is evaluatable (see
    // evaluableEqc).
    bool evaluable = false;
    // Loop through terms in this EC
    eq::EqClassIterator eqc_i = eq::EqClassIterator(eqc, ee);
    for (; !eqc_i.isFinished(); ++eqc_i)
    {
      Node n = *eqc_i;
      Trace("model-builder") << "    Processing Term: " << n << endl;

      // For each term n in this equivalence class, below we register its
      // assignable subterms, compute whether it is a constant or assigned
      // representative, then if we don't have a constant representative,
      // compute information regarding how we will assign values.

      // (1) Add assignable subterms, which ensures that e.g. models for
      // uninterpreted functions take into account all subterms in the
      // equality engine of the model
      addAssignableSubterms(n, tm, assignableCache);
      // model-specific processing of the term
      tm->addTermInternal(n);

      // compute whether n is assignable
      if (!isAssignable(n))
      {
        // (2) Record constant representative or assign representative, if
        // applicable. We check if n is a value here, e.g. a term for which
        // isConst returns true, or a lambda. The latter is required only for
        // higher-order.
        if (tm->isValue(n))
        {
          // In some cases, there can be multiple terms in the same equivalence
          // class are considered values, e.g., when real algebraic numbers did
          // not simplify to rational values or real.pi was used as a model
          // value. We distinguish three kinds of model values: constants,
          // non-constant base values and non-base values, and we use them in
          // this order of preference.
          // We print a trace message if there is more than one model value in
          // the same equivalence class. We throw a debug failure if there are
          // at least two base model values in the same equivalence class that
          // do not compare equal.
          bool assignConstRep = false;
          bool isBaseValue = tm->isBaseModelValue(n);
          if (constRep.isNull())
          {
            assignConstRep = true;
          }
          else
          {
            // This is currently a trace message, as it often triggers for
            // non-linear arithmetic before the model is refined enough to
            // e.g. show transcendental function apps are not equal to rationals
            Trace("model-warn") << "Model values in the same equivalence class "
                                << constRep << " " << n << std::endl;
            if (!constRepBaseModelValue)
            {
              assignConstRep = isBaseValue;
            }
            else if (isBaseValue)
            {
              Node isEqual = rewrite(constRep.eqNode(n));
              if (isEqual.isConst() && isEqual.getConst<bool>())
              {
                assignConstRep = n.isConst();
              }
              else
              {
                DebugUnhandled() << "Distinct base model values in the same "
                                    "equivalence class "
                                 << constRep << " " << n << std::endl;
              }
            }
          }
          if (assignConstRep)
          {
            constRep = n;
            Trace("model-builder") << "    ..ConstRep( " << eqc
                                   << " ) = " << constRep << std::endl;
            constRepBaseModelValue = isBaseValue;
          }
          // if we have a constant representative, nothing else matters
          continue;
        }

        // If we don't have a constant rep, check if this is an assigned rep.
        itm = tm->d_reps.find(n);
        if (itm != tm->d_reps.end())
        {
          // Notice that this equivalence class may contain multiple terms that
          // were specified as being a representative, since e.g. datatypes may
          // assert representative for two constructor terms that are not in the
          // care graph and are merged during collectModeInfo due to equality
          // information from another theory. We overwrite the value of rep in
          // these cases here.
          rep = itm->second;
          Trace("model-builder")
              << "    ..Rep( " << eqc << " ) = " << rep << std::endl;
        }

        // (3) Finally, process assignable information
        // We are evaluable typically if we are not assignable. However the
        // one exception is that higher-order variables when in HOL should be
        // considered neither assignable nor evaluable, which we check for here.
        evaluable = !n.isVar();
        continue;
      }
      assignable = true;
    }

    // finished traversing the equality engine
    TypeNode eqct = eqc.getType();
    // count the number of equivalence classes of sorts in finite model finding

    // Assign representative for this equivalence class
    if (!constRep.isNull())
    {
      // Theories should not specify a rep if there is already a constant in the
      // equivalence class. However, it may be the case that the representative
      // specified by a theory may be merged with a constant based on equality
      // information from another class. Thus, rep may be non-null here.
      // Regardless, we assign constRep as the representative here.
      assignConstantRep(tm, eqc, constRep);
      typeConstSet.add(eqct, constRep);
      continue;
    }
    else if (!rep.isNull())
    {
      assertedReps[eqc] = rep;
      typeRepSet.add(eqct, eqc);
      std::unordered_set<TypeNode> visiting;
      addToTypeList(eqct, type_list, visiting);
    }
    else
    {
      typeNoRepSet.add(eqct, eqc);
      std::unordered_set<TypeNode> visiting;
      addToTypeList(eqct, type_list, visiting);
    }

    if (assignable)
    {
      assignableEqc.insert(eqc);
    }
    if (evaluable)
    {
      evaluableEqc.insert(eqc);
    }
  }

  // Now finished initialization

  // Need to ensure that each EC has a constant representative.

  Trace("model-builder") << "Processing EC's..." << std::endl;

  TypeSet::iterator it;
  vector<TypeNode>::iterator type_it;
  set<Node>::iterator i, i2;
  bool changed, unassignedAssignable, assignOne = false;
  set<TypeNode> evaluableSet;

  // Double-fixed-point loop
  // Outer loop handles a special corner case (see code at end of loop for
  // details)
  for (;;)
  {
    // Inner fixed-point loop: we are trying to learn constant values for every
    // EC.  Each time through this loop, we process all of the
    // types by type and may learn some new EC values.  EC's in one type may
    // depend on EC's in another type, so we need a fixed-point loop
    // to ensure that we learn as many EC values as possible
    do
    {
      changed = false;
      unassignedAssignable = false;
      evaluableSet.clear();

      // Iterate over all types we've seen
      for (type_it = type_list.begin(); type_it != type_list.end(); ++type_it)
      {
        TypeNode t = *type_it;
        TypeNode tb = t;
        set<Node>* noRepSet = typeNoRepSet.getSet(t);

        // 1. Try to evaluate the EC's in this type
        if (noRepSet != nullptr && !noRepSet->empty())
        {
          Trace("model-builder")
              << "  Eval phase, working on type: " << t << endl;
          bool evaluable;
          d_normalizedCache.clear();
          for (i = noRepSet->begin(); i != noRepSet->end();)
          {
            i2 = i;
            ++i;
            Trace("model-builder-debug")
                << "Look at eqc : " << (*i2) << std::endl;
            Node normalized;
            // only possible to normalize if we are evaluable
            evaluable = evaluableEqc.find(*i2) != evaluableEqc.end();
            if (evaluable)
            {
              normalized = evaluateEqc(tm, *i2);
            }
            if (!normalized.isNull())
            {
              Assert(tm->isValue(normalized));
              typeConstSet.add(tb, normalized);
              assignConstantRep(tm, *i2, normalized);
              Trace("model-builder") << "    Eval: Setting constant rep of "
                                     << (*i2) << " to " << normalized << endl;
              changed = true;
              noRepSet->erase(i2);
            }
            else
            {
              if (evaluable)
              {
                evaluableSet.insert(tb);
              }
              // If assignable, remember there is an equivalence class that is
              // not assigned and assignable.
              if (assignableEqc.find(*i2) != assignableEqc.end())
              {
                unassignedAssignable = true;
              }
            }
          }
        }

        // 2. Normalize any non-const representative terms for this type
        set<Node>* repSet = typeRepSet.getSet(t);
        if (repSet != nullptr && !repSet->empty())
        {
          Trace("model-builder")
              << "  Normalization phase, working on type: " << t << endl;
          d_normalizedCache.clear();
          for (i = repSet->begin(); i != repSet->end();)
          {
            Assert(assertedReps.find(*i) != assertedReps.end());
            Node rep = assertedReps[*i];
            Node normalized = normalize(tm, rep, false);
            Trace("model-builder")
                << "    Normalizing rep (" << rep << "), normalized to ("
                << normalized << ")"
                << ", isValue=" << tm->isValue(normalized) << std::endl;
            if (tm->isValue(normalized))
            {
              changed = true;
              typeConstSet.add(tb, normalized);
              assignConstantRep(tm, *i, normalized);
              assertedReps.erase(*i);
              i2 = i;
              ++i;
              repSet->erase(i2);
            }
            else
            {
              if (normalized != rep)
              {
                assertedReps[*i] = normalized;
                changed = true;
              }
              ++i;
            }
          }
        }
      }
    } while (changed);

    if (!unassignedAssignable)
    {
      break;
    }

    // 3. Assign unassigned assignable EC's using type enumeration - assign a
    // value *different* from all other EC's if the type is infinite
    // Assign first value from type enumerator otherwise - for finite types, we
    // rely on polite framework to ensure that EC's that have to be
    // different are different.

    // Only make assignments on a type if:
    // 1. there are no terms that share the same base type with un-normalized
    // representatives
    // 2. there are no terms that share teh same base type that are unevaluated
    // evaluable terms
    // Alternatively, if 2 or 3 don't hold but we are in a special
    // deadlock-breaking mode where assignOne is true, go ahead and make one
    // assignment
    changed = false;
    // must iterate over the ordered type list to ensure that we do not
    // enumerate values with subterms
    //  having types that we are currently enumerating (when possible)
    //  for example, this ensures we enumerate uninterpreted sort U before (List
    //  of U) and (Array U U)
    //  however, it does not break cyclic type dependencies for mutually
    //  recursive datatypes, but this is handled
    //  by recording all subterms of enumerated values in TypeSet::addSubTerms.
    for (type_it = type_list.begin(); type_it != type_list.end(); ++type_it)
    {
      TypeNode t = *type_it;
      // continue if there are no more equivalence classes of this type to
      // assign
      std::set<Node>* noRepSetPtr = typeNoRepSet.getSet(t);
      if (noRepSetPtr == nullptr)
      {
        continue;
      }
      set<Node>& noRepSet = *noRepSetPtr;
      if (noRepSet.empty())
      {
        continue;
      }

      TypeNode tb = t;
      if (!assignOne)
      {
        set<Node>* repSet = typeRepSet.getSet(tb);
        if (repSet != nullptr && !repSet->empty())
        {
          continue;
        }
        if (evaluableSet.find(tb) != evaluableSet.end())
        {
          continue;
        }
      }
      Trace("model-builder")
          << "  Assign phase, working on type: " << t << endl;
      bool assignable, evaluable AVA6_UNUSED;
      for (i = noRepSet.begin(); i != noRepSet.end();)
      {
        i2 = i;
        ++i;
        if (evaluableEqc.find(*i2) != evaluableEqc.end())
        {
          Trace("model-builder")
              << "  ...do not assign to evaluatable eqc " << *i2 << std::endl;
          // we never assign to evaluable equivalence classes
          continue;
        }
        assignable = assignableEqc.find(*i2) != assignableEqc.end();
        Trace("model-builder-debug")
            << "    eqc " << *i2 << " is assignable=" << assignable
            << std::endl;
        if (assignable)
        {
          // this assertion ensures that if we are assigning to a term of
          // Boolean type, then the term must be assignable.
          // Note we only assign to terms of Boolean type if the term occurs in
          // a singleton equivalence class; otherwise the term would have been
          // in the equivalence class of true or false and would not need
          // assigning.
          Assert(!t.isBoolean() || isAssignable(*i2));
          Node n;
          if (!d_env.isFiniteType(t))
          {
            // Assign a fresh value of infinite type.
            n = typeConstSet.nextTypeEnum(t);
            Assert(!n.isNull());
            Trace("model-value-enum") << "Enum infinite " << t << " " << n
                                      << " for " << *i2 << std::endl;
          }
          else
          {
            // Otherwise, we get the first value from the type enumerator.
            Trace("model-builder-debug")
                << "Get first value from finite type..." << std::endl;
            TypeEnumerator te(t);
            n = *te;
            Trace("model-value-enum") << "Enum finite " << t << " " << n
                                      << " for " << *i2 << std::endl;
          }
          Trace("model-builder-debug") << "...got " << n << std::endl;
          assignConstantRep(tm, *i2, n);
          changed = true;
          noRepSet.erase(i2);
          if (assignOne)
          {
            assignOne = false;
            break;
          }
        }
      }
    }

    // Corner case - I'm not sure this can even happen - but it's theoretically
    // possible to have a cyclical dependency
    // in EC assignment/evaluation, e.g. EC1 = {a, b + 1}; EC2 = {b, a - 1}.  In
    // this case, neither one will get assigned because we are waiting
    // to be able to evaluate.  But we will never be able to evaluate because
    // the variables that need to be assigned are in
    // these same EC's.  In this case, repeat the whole fixed-point computation
    // with the difference that the first EC
    // that has both assignable and evaluable expressions will get assigned.
    if (!changed)
    {
      Trace("model-builder-debug") << "...must assign one" << std::endl;
      // Avoid infinite loops: if we are in a deadlock, we abort model building
      // unsuccessfully here.
      if (assignOne)
      {
        Assert(false) << "Reached a deadlock during model construction";
        Trace("model-builder-debug") << "...avoid loop, fail" << std::endl;
        return false;
      }
      assignOne = true;
    }
  }

#ifdef AVA6_ASSERTIONS
  // Assert that all representatives have been converted to constants
  for (it = typeRepSet.begin(); it != typeRepSet.end(); ++it)
  {
    std::set<Node>& repSet = TypeSet::getSet(it);
    if (!repSet.empty())
    {
      Trace("model-builder") << "***Non-empty repSet, size = " << repSet.size()
                             << ", repSet = " << repSet << endl;
      Trace("model-builder-debug") << tm->getEqualityEngine()->debugPrintEqc();
      DebugUnhandled();
    }
  }
#endif /* AVA6_ASSERTIONS */

  Trace("model-builder") << "Copy representatives to model..." << std::endl;
  tm->d_reps.clear();
  std::map<Node, Node>::iterator itMap;
  for (itMap = d_constantReps.begin(); itMap != d_constantReps.end(); ++itMap)
  {
    // The "constant" representative is a model value, which may be a lambda
    // if higher-order. We now can go back and normalize its subterms.
    // This is necessary if we assigned a lambda value whose body contains
    // a free constant symbol that was assigned in this method.
    Node normc = itMap->second;
    if (!normc.isConst())
    {
      normc = normalize(tm, normc, true);
    }
    // mark this as the final representative
    tm->assignRepresentative(itMap->first, normc, true);
  }

  Trace("model-builder") << "Make sure ECs have reps..." << std::endl;
  // Make sure every EC has a rep
  for (itMap = assertedReps.begin(); itMap != assertedReps.end(); ++itMap)
  {
    tm->assignRepresentative(itMap->first, itMap->second, false);
  }
  for (it = typeNoRepSet.begin(); it != typeNoRepSet.end(); ++it)
  {
    set<Node>& noRepSet = TypeSet::getSet(it);
    for (const Node& node : noRepSet)
    {
      tm->assignRepresentative(node, node, false);
    }
  }

  // modelBuilder-specific initialization
  if (!processBuildModel(tm))
  {
    Trace("model-builder")
        << "TheoryEngineModelBuilder: fail process build model." << std::endl;
    return false;
  }
  Trace("model-builder") << "TheoryEngineModelBuilder: success" << std::endl;
  return true;
}

void TheoryEngineModelBuilder::postProcessModel(bool incomplete, TheoryModel* m)
{
  Trace("model-builder") << "postProcessModel" << std::endl;
  // Note that we do not insist that functions are assigned here, they can
  // continue to be built on demand in the theory model.
  // if we are incomplete, there is no guarantee on the model.
  // thus, we do not check the model here.
  if (incomplete)
  {
    return;
  }
  Assert(m != nullptr);
  // debug-check the model if the checkModels() is enabled.
  if (options().smt.debugCheckModels)
  {
    debugCheckModel(m);
  }
}

void TheoryEngineModelBuilder::debugCheckModel(TheoryModel* tm)
{
  eq::EqClassesIterator eqcs_i = eq::EqClassesIterator(tm->d_equalityEngine);
  std::map<Node, Node>::iterator itMap;
  // Check that every term evaluates to its representative in the model
  for (eqcs_i = eq::EqClassesIterator(tm->d_equalityEngine);
       !eqcs_i.isFinished();
       ++eqcs_i)
  {
    // eqc is the equivalence class representative
    Node eqc = (*eqcs_i);
    // get the representative
    Node rep = tm->getRepresentative(eqc);
    if (!rep.isConst() && eqc.getType().isBoolean())
    {
      // if Boolean, it does not necessarily have a constant representative, use
      // get value instead
      rep = tm->getValue(eqc);
      AlwaysAssert(rep.isConst());
    }
    eq::EqClassIterator eqc_i = eq::EqClassIterator(eqc, tm->d_equalityEngine);
    for (; !eqc_i.isFinished(); ++eqc_i)
    {
      Node n = *eqc_i;
      AlwaysAssert(AVA6_EQUAL(rep.getType(), n.getType()))
          << "Representative " << rep << " of " << n
          << " violates type constraints (" << rep.getType() << " and "
          << n.getType() << ")";
      Node val = tm->getValue(n);
      if (val != rep)
      {
        std::stringstream err;
        err << "Failed representative check:" << std::endl
            << "n: " << n << std::endl
            << "getValue(n): " << val << std::endl
            << "rep: " << rep << std::endl;
        if (val.isConst() && rep.isConst())
        {
          AlwaysAssert(val == rep) << err.str();
        }
        else if (!AVA6_EQUAL(rewrite(val), rewrite(rep)))
        {
          // if it does not evaluate, it is just a warning, which may be the
          // case for non-constant values, e.g. lambdas. Furthermore we only
          // throw this warning if rewriting cannot show they are equal.
          warning() << err.str();
        }
      }
    }
  }

  // builder-specific debugging
  debugModel(tm);
}

Node TheoryEngineModelBuilder::normalize(TheoryModel* m, TNode r, bool evalOnly)
{
  std::map<Node, Node>::iterator itMap = d_constantReps.find(r);
  if (itMap != d_constantReps.end())
  {
    r = (*itMap).second;
    // if d_constantReps stores a constant, we are done, otherwise we process
    // it below.
    if (r.isConst())
    {
      return r;
    }
  }
  NodeMap::iterator it = d_normalizedCache.find(r);
  if (it != d_normalizedCache.end())
  {
    return (*it).second;
  }
  Trace("model-builder-debug") << "do normalize on " << r << std::endl;
  Node retNode = r;
  if (r.getNumChildren() > 0)
  {
    std::vector<Node> children;
    if (r.getMetaKind() == kind::metakind::PARAMETERIZED)
    {
      children.push_back(r.getOperator());
    }
    for (size_t i = 0, nchild = r.getNumChildren(); i < nchild; ++i)
    {
      Node ri = r[i];
      bool recurse = true;
      if (!ri.isConst())
      {
        if (m->d_equalityEngine->hasTerm(ri))
        {
          itMap =
              d_constantReps.find(m->d_equalityEngine->getRepresentative(ri));
          if (itMap != d_constantReps.end())
          {
            ri = (*itMap).second;
            Trace("model-builder-debug")
                << i << ": const child " << ri << std::endl;
            // need to recurse if d_constantReps stores a non-constant
            recurse = !ri.isConst();
          }
          else if (!evalOnly)
          {
            recurse = false;
            Trace("model-builder-debug") << i << ": keep " << ri << std::endl;
          }
        }
        else
        {
          Trace("model-builder-debug")
              << i << ": no hasTerm " << ri << std::endl;
        }
        if (recurse)
        {
          ri = normalize(m, ri, evalOnly);
        }
      }
      children.push_back(ri);
    }
    retNode = nodeManager()->mkNode(r.getKind(), children);
    retNode = rewrite(retNode);
  }
  d_normalizedCache[r] = retNode;
  return retNode;
}

bool TheoryEngineModelBuilder::preProcessBuildModel(AVA6_UNUSED TheoryModel* m)
{
  return true;
}

bool TheoryEngineModelBuilder::processBuildModel(AVA6_UNUSED TheoryModel* m)
{
  return true;
}

}  // namespace theory
}  // namespace ava6::internal
