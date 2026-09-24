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

#include "theory/uf/theory_uf.h"

#include <memory>
#include <sstream>

#include "expr/node_algorithm.h"
#include "expr/skolem_manager.h"
#include "options/quantifiers_options.h"
#include "options/smt_options.h"
#include "options/theory_options.h"
#include "options/uf_options.h"
#include "proof/proof_node_manager.h"
#include "smt/logic_exception.h"
#include "theory/arith/arith_utilities.h"
#include "theory/theory_model.h"
#include "theory/type_enumerator.h"
#include "theory/uf/conversions_solver.h"
#include "theory/uf/theory_uf_rewriter.h"

using namespace std;

namespace ava6::internal {
namespace theory {
namespace uf {

/** Constructs a new instance of TheoryUF w.r.t. the provided context.*/
TheoryUF::TheoryUF(Env& env,
                   OutputChannel& out,
                   Valuation valuation,
                   std::string instanceName)
    : Theory(THEORY_UF, env, out, valuation, instanceName),
      d_dpfgen(env),
      d_functionsTerms(context()),
      d_rewriter(nodeManager()),
      d_checker(nodeManager()),
      d_state(env, valuation),
      d_im(env, *this, d_state, "theory::uf::" + instanceName, false),
      d_distinct(env, d_state, d_im),
      d_notify(d_im, *this),
      d_cpacb(*this)
{
  d_true = nodeManager()->mkConst(true);
  // indicate we are using the default theory state and inference managers
  d_theoryState = &d_state;
  d_inferManager = &d_im;
}

TheoryUF::~TheoryUF() {}

TheoryRewriter* TheoryUF::getTheoryRewriter() { return &d_rewriter; }

ProofRuleChecker* TheoryUF::getProofChecker() { return &d_checker; }

bool TheoryUF::needsEqualityEngine(EeSetupInfo& esi)
{
  esi.d_notify = &d_notify;
  esi.d_name = d_instanceName + "theory::uf::ee";

  return true;
}

void TheoryUF::finishInit()
{
  Assert(d_equalityEngine != nullptr);
  // combined cardinality constraints are not evaluated in getModelValue
  // distinct should not be sent to the model
  d_valuation.setIrrelevantKind(Kind::DISTINCT);

  // Initialize the cardinality constraints solver if the logic includes UF,
  // finite model finding is enabled, and it is not disabled by
  // the ufssMode option.

  // The kinds we are treating as function application in congruence

  d_equalityEngine->addFunctionKind(Kind::APPLY_UF, false, false);
  
  // conversion kinds
  d_equalityEngine->addFunctionKind(Kind::INT_TO_BITVECTOR, true);
  d_equalityEngine->addFunctionKind(Kind::BITVECTOR_UBV_TO_INT, true);
}

//--------------------------------- standard check

bool TheoryUF::needsCheckLastEffort()
{
  // last call effort needed if using finite model finding,
  // arithmetic/bit-vector conversions, or higher-order extension
  return d_csolver != nullptr || false
         || d_distinct.needsCheckLastEffort();
}

void TheoryUF::postCheck(Effort level)
{
  if (d_state.isInConflict())
  {
    return;
  }
  // check with the cardinality constraints extension

  if (!d_state.isInConflict())
  {
    if (level == Effort::EFFORT_LAST_CALL)
    {
      // check with conversions solver at last call effort
      if (d_csolver != nullptr)
      {
        d_csolver->check();
      }
    }
    d_distinct.check(level);
    // check with the higher-order extension at full effort
    
  }
}

void TheoryUF::notifyFact(TNode atom,
                          bool pol,
                          TNode fact,
                          AVA6_UNUSED bool isInternal)
{
  if (d_state.isInConflict())
  {
    return;
  }

  switch (atom.getKind())
  {
    case Kind::EQUAL:
    {
      
    }
    break;
    case Kind::DISTINCT:
    {
      // call the distinct extension
      d_distinct.assertDistinct(atom, pol, fact);
    }
    break;
    default: break;
  }
}
//--------------------------------- end standard check

TrustNode TheoryUF::ppRewrite(TNode node, std::vector<SkolemLemma>& lems)
{
  Trace("uf-exp-def") << "TheoryUF::ppRewrite: expanding definition : " << node
                      << std::endl;
  Kind k = node.getKind();

  if (node.getType().isAbstract())
  {
    std::stringstream ss;
    ss << "Cannot process term of abstract type " << node;
    throw LogicException(ss.str());
  }
  if (false || node.getType().isFunction())
  {
    {
      std::stringstream ss;
      {
        ss << "Function terms";
      }
      ss << " are only supported with "
            "higher-order logic. Try adding the logic prefix HO_.";
      throw LogicException(ss.str());
    }
  }
  else if (k == Kind::APPLY_UF)
  {
    if (!false && isHigherOrderType(node.getOperator().getType()))
    {
      // check for higher-order
      // logic exception if higher-order is not enabled
      std::stringstream ss;
      ss << "UF received an application whose operator has higher-order type "
         << node
         << ", which is only supported with higher-order logic. Try adding "
            "the logic prefix HO_.";
      throw LogicException(ss.str());
    }
  }
  else if ((k == Kind::BITVECTOR_UBV_TO_INT || k == Kind::INT_TO_BITVECTOR)
           && options().uf.eagerArithBvConv)
  {
    // eliminate if option specifies to eliminate eagerly
    Node ret = k == Kind::BITVECTOR_UBV_TO_INT ? arith::eliminateBv2Nat(node)
                                               : arith::eliminateInt2Bv(node);
    return TrustNode::mkTrustRewrite(node, ret);
  }
  
  return TrustNode::null();
}

void TheoryUF::preRegisterTerm(TNode node)
{
  Trace("uf") << "TheoryUF::preRegisterTerm(" << node << ")" << std::endl;


  Kind k = node.getKind();
  switch (k)
  {
    case Kind::EQUAL:
      // Add the trigger for equality
      d_state.addEqualityEngineTriggerPredicate(node);
      break;
    case Kind::APPLY_UF: preRegisterFunctionTerm(node); break;
    case Kind::INT_TO_BITVECTOR:
    case Kind::BITVECTOR_UBV_TO_INT:
    {
      Assert(!options().uf.eagerArithBvConv);
      d_equalityEngine->addTerm(node);
      d_functionsTerms.push_back(node);
      // initialize the conversions solver if not already done so
      if (d_csolver == nullptr)
      {
        d_csolver.reset(new ConversionsSolver(d_env, d_state, d_im));
      }
      // call preregister
      d_csolver->preRegisterTerm(node);
    }
    break;
    case Kind::UNINTERPRETED_SORT_VALUE:
    {
      // Uninterpreted sort values should only appear in models, and should
      // never appear in constraints. They are unallowed to ever appear in
      // constraints since the cardinality of an uninterpreted sort may have an
      // upper bound, e.g. if (forall ((x U) (y U)) (= x y)) holds, then @uc_U_2
      // is a ill-formed term, as its existence cannot be assumed.  The parser
      // prevents the user from ever constructing uninterpreted sort values.
      // However, they may be exported via models to API users. It is thus
      // possible that these uninterpreted sort values are asserted back in
      // constraints, hence this check is necessary.
      throw LogicException(
          "An uninterpreted constant was preregistered to the UF theory.");
    }
    break;
    default:
      // Variables etc
      d_equalityEngine->addTerm(node);
      if (node.getType().isFunction())
      {
        std::stringstream ss;
        ss << "Function terms are only supported with higher-order logic. Try "
              "adding the logic prefix HO_.";
        throw LogicException(ss.str());
      }
      break;
  }
}

void TheoryUF::preRegisterFunctionTerm(TNode node)
{
  // Maybe it's a predicate
  if (node.getType().isBoolean())
  {
    d_state.addEqualityEngineTriggerPredicate(node);
  }
  else
  {
    // Function applications/predicates
    d_equalityEngine->addTerm(node);
  }
  // Remember the function and predicate terms
  d_functionsTerms.push_back(node);
}

void TheoryUF::explain(TNode literal, Node& exp)
{
  Trace("uf") << "TheoryUF::explain(" << literal << ")" << std::endl;
  std::vector<TNode> assumptions;
  // Do the work
  bool polarity = literal.getKind() != Kind::NOT;
  TNode atom = polarity ? literal : literal[0];
  if (atom.getKind() == Kind::EQUAL)
  {
    d_equalityEngine->explainEquality(
        atom[0], atom[1], polarity, assumptions, nullptr);
  }
  else
  {
    d_equalityEngine->explainPredicate(atom, polarity, assumptions, nullptr);
  }
  exp = nodeManager()->mkAnd(assumptions);
}

TrustNode TheoryUF::explain(TNode literal) { return d_im.explainLit(literal); }

void TheoryUF::presolve()
{
  // TimerStat::CodeTimer codeTimer(d_presolveTimer);

  Trace("uf") << "uf: begin presolve()" << endl;


  Trace("uf") << "uf: end presolve()" << endl;
}

void TheoryUF::ppStaticLearn(TNode n, std::vector<TrustNode>& learned)
{
  // TimerStat::CodeTimer codeTimer(d_staticLearningTimer);

  // Use the diamonds utility
  d_dpfgen.ppStaticLearn(n, learned);


} /* TheoryUF::ppStaticLearn() */

EqualityStatus TheoryUF::getEqualityStatus(TNode a, TNode b)
{
  // Check for equality (simplest)
  if (d_equalityEngine->areEqual(a, b))
  {
    // The terms are implied to be equal
    return EQUALITY_TRUE;
  }

  // Check for disequality
  if (d_equalityEngine->areDisequal(a, b, false))
  {
    // The terms are implied to be dis-equal
    return EQUALITY_FALSE;
  }

  // All other terms we interpret as dis-equal in the model
  return EQUALITY_FALSE_IN_MODEL;
}

bool TheoryUF::areCareDisequal(TNode x, TNode y)
{
  // check for disequality first, as an optimization
  if (d_equalityEngine->hasTerm(x) && d_equalityEngine->hasTerm(y)
      && d_equalityEngine->areDisequal(x, y, false))
  {
    return true;
  }
  if (d_equalityEngine->isTriggerTerm(x, THEORY_UF)
      && d_equalityEngine->isTriggerTerm(y, THEORY_UF))
  {
    TNode x_shared =
        d_equalityEngine->getTriggerTermRepresentative(x, THEORY_UF);
    TNode y_shared =
        d_equalityEngine->getTriggerTermRepresentative(y, THEORY_UF);
    EqualityStatus eqStatus = d_valuation.getEqualityStatus(x_shared, y_shared);
    if (eqStatus == EQUALITY_FALSE || eqStatus == EQUALITY_FALSE_AND_PROPAGATED)
    {
      return true;
    }
    else if (eqStatus == EQUALITY_FALSE_IN_MODEL)
    {
      // if x or y is a lambda function, and they are neither entailed to
      // be equal or disequal, then we return false. This ensures the pair
      // (x,y) may be considered for the care graph.
      
      return true;
    }
  }
  return false;
}

void TheoryUF::processCarePairArgs(TNode a, TNode b)
{
  // if a and b are already equal, we ignore this pair
  if (d_state.areEqual(a, b))
  {
    return;
  }
  // otherwise, we add pairs for each of their arguments
  addCarePairArgs(a, b);

  // also split on functions
  
}

void TheoryUF::computeCareGraph()
{

  // note that if we are higher-order, we may still generate splits for
  // function arguments
  if (d_state.getSharedTerms().empty() && !false)
  {
    return;
  }
  NodeManager* nm = nodeManager();
  // Use term indexing. We build separate indices for APPLY_UF and HO_APPLY.
  // We maintain indices per operator for the former, and indices per
  // function type for the latter.
  Trace("uf::sharing") << "TheoryUf::computeCareGraph(): Build term indices..."
                       << std::endl;
  // temporary keep set for higher-order indexing below
  std::vector<Node> keep;
  std::map<Node, TNodeTrie> index;
  std::map<TypeNode, TNodeTrie> typeIndex;
  std::map<Node, size_t> arity;
  for (TNode app : d_functionsTerms)
  {
    std::vector<TNode> reps;
    bool has_trigger_arg = false;
    for (const Node& j : app)
    {
      reps.push_back(d_equalityEngine->getRepresentative(j));
      // if doing higher-order, higher-order arguments must all be considered as
      // well
      if (d_equalityEngine->isTriggerTerm(j, THEORY_UF)
          || (false && j.getType().isFunction()))
      {
        has_trigger_arg = true;
      }
    }
    if (has_trigger_arg)
    {
      Trace("uf::sharing-terms")
          << "...add: " << app << " / " << reps << std::endl;
      Kind k = app.getKind();
      if (k == Kind::APPLY_UF)
      {
        Node op = app.getOperator();
        index[op].addTerm(app, reps);
        arity[op] = reps.size();
        
      }
      else if (false || k == Kind::BITVECTOR_UBV_TO_INT)
      {
        // add it to the typeIndex for the function type if HO_APPLY, or the
        // bitvector type if bv2nat. The latter ensures that we compute
        // care pairs based on bv2nat only for bitvectors of the same width.
        typeIndex[app[0].getType()].addTerm(app, reps);
      }
      else
      {
        // case for other operators, e.g. int2bv
        Node op = app.getOperator();
        index[op].addTerm(app, reps);
        arity[op] = reps.size();
      }
    }
  }
  // for each index
  for (std::pair<const Node, TNodeTrie>& tt : index)
  {
    Trace("uf::sharing") << "TheoryUf::computeCareGraph(): Process index "
                         << tt.first << "..." << std::endl;
    Assert(arity.find(tt.first) != arity.end());
    nodeTriePathPairProcess(&tt.second, arity[tt.first], d_cpacb);
  }
  for (std::pair<const TypeNode, TNodeTrie>& tt : typeIndex)
  {
    // functions for HO_APPLY which has arity 2, bitvectors for bv2nat which
    // has arity one
    size_t a = tt.first.isFunction() ? 2 : 1;
    Trace("uf::sharing") << "TheoryUf::computeCareGraph(): Process ho index "
                         << tt.first << "..." << std::endl;
    // the arity of HO_APPLY is always two
    nodeTriePathPairProcess(&tt.second, a, d_cpacb);
  }
  Trace("uf::sharing") << "TheoryUf::computeCareGraph(): finished."
                       << std::endl;
} /* TheoryUF::computeCareGraph() */

void TheoryUF::eqNotifyMerge(TNode t1, TNode t2)
{

  // check if we have a conflict due to distinct
  d_distinct.eqNotifyMerge(t1, t2);
}

bool TheoryUF::isHigherOrderType(TypeNode tn)
{
  Assert(tn.isFunction());
  std::map<TypeNode, bool>::iterator it = d_isHoType.find(tn);
  if (it != d_isHoType.end())
  {
    return it->second;
  }
  bool ret = false;
  const std::vector<TypeNode>& argTypes = tn.getArgTypes();
  for (const TypeNode& tnc : argTypes)
  {
    if (tnc.isFunction())
    {
      ret = true;
      break;
    }
  }
  d_isHoType[tn] = ret;
  return ret;
}

}  // namespace uf
}  // namespace theory
}  // namespace ava6::internal
