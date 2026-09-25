/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Model object for the non-linear extension class.
 */

#include "theory/arith/nl/nl_model.h"

#include "expr/node_algorithm.h"
#include "options/arith_options.h"
#include "options/smt_options.h"
#include "options/theory_options.h"
#include "theory/arith/arith_msum.h"
#include "theory/arith/arith_utilities.h"
#include "theory/rewriter.h"

using namespace ava6::internal::kind;

namespace ava6::internal {
namespace theory {
namespace arith {
namespace nl {

NlModel::NlModel(Env& env) : EnvObj(env)
{
  d_zero = nodeManager()->mkConstReal(Rational(0));
  d_one = nodeManager()->mkConstReal(Rational(1));
}

NlModel::~NlModel() {}

void NlModel::reset(const std::map<Node, Node>& arithModel)
{
  d_concreteModelCache.clear();
  d_abstractModelCache.clear();
  d_arithVal = arithModel;
}

void NlModel::resetCheck()
{
  d_check_model_solved.clear();
  d_substitutions.clear();
}

Node NlModel::computeConcreteModelValue(TNode n)
{
  return computeModelValue(n, true);
}

Node NlModel::computeAbstractModelValue(TNode n)
{
  return computeModelValue(n, false);
}

Node NlModel::computeModelValue(TNode n, bool isConcrete)
{
  auto& cache = isConcrete ? d_concreteModelCache : d_abstractModelCache;
  if (auto it = cache.find(n); it != cache.end())
  {
    return it->second;
  }
  Trace("nl-ext-mv-debug") << "computeModelValue " << n
                           << ", isConcrete=" << isConcrete << std::endl;
  Node ret;
  if (n.isConst())
  {
    ret = n;
  }
  else if (!isConcrete && hasLinearModelValue(n, ret))
  {
    // use model value for abstraction
  }
  else if (n.getNumChildren() == 0)
  {
    ret = getValueInternal(n);
  }
  else
  {
    // otherwise, compute true value
    TheoryId ctid = theory::kindToTheoryId(n.getKind());
    if (ctid != THEORY_ARITH && ctid != THEORY_BOOL && ctid != THEORY_BUILTIN)
    {
      // we directly look up terms not belonging to arithmetic
      ret = getValueInternal(n);
    }
    else
    {
      std::vector<Node> children;
      if (n.getMetaKind() == metakind::PARAMETERIZED)
      {
        children.emplace_back(n.getOperator());
      }
      for (size_t i = 0, nchild = n.getNumChildren(); i < nchild; i++)
      {
        children.emplace_back(computeModelValue(n[i], isConcrete));
      }
      ret = nodeManager()->mkNode(n.getKind(), children);
      ret = rewrite(ret);
    }
  }
  Trace("nl-ext-mv-debug") << "computed " << (isConcrete ? "M" : "M_A") << "["
                           << n << "] = " << ret << std::endl;
  AssertEqual(n.getType(), ret.getType());
  cache[n] = ret;
  return ret;
}

int NlModel::compare(TNode i, TNode j, bool isConcrete, bool isAbsolute)
{
  if (i == j)
  {
    return 0;
  }
  Node ci = computeModelValue(i, isConcrete);
  Node cj = computeModelValue(j, isConcrete);
  if (ci.isConst())
  {
    if (cj.isConst())
    {
      return compareValue(ci, cj, isAbsolute);
    }
    return 1;
  }
  return cj.isConst() ? -1 : 0;
}

int NlModel::compareValue(TNode i, TNode j, bool isAbsolute) const
{
  Assert(i.isConst() && j.isConst());
  if (i == j)
  {
    return 0;
  }
  if (!isAbsolute)
  {
    return i.getConst<Rational>() < j.getConst<Rational>() ? -1 : 1;
  }
  Rational iabs = i.getConst<Rational>().abs();
  Rational jabs = j.getConst<Rational>().abs();
  if (iabs == jabs)
  {
    return 0;
  }
  return iabs < jabs ? -1 : 1;
}

bool NlModel::checkModel(const std::vector<Node>& assertions)
{
  Trace("nl-ext-cm-debug") << "NlModel::checkModel: solve for equalities..."
                           << std::endl;
  for (const Node& atom : assertions)
  {
    Trace("nl-ext-cm-debug") << "- assertion: " << atom << std::endl;
    // Try to solve the equality using exact substitutions.
    if (atom.getKind() == Kind::EQUAL)
    {
      // we substitute inside of solve equality simple
      if (!solveEqualitySimple(atom))
      {
        // no chance we will satisfy this equality
        Trace("nl-ext-cm") << "...check-model : failed to solve equality : "
                           << atom << std::endl;
      }
    }
  }

  // all remaining variables are constrained to their exact model values
  Trace("nl-ext-cm-debug") << "  set exact bounds for remaining variables..."
                           << std::endl;
  std::unordered_set<TNode> visited;
  std::vector<TNode> visit;
  TNode cur;
  for (const Node& a : assertions)
  {
    visit.push_back(a);
    do
    {
      cur = visit.back();
      visit.pop_back();
      if (visited.find(cur) == visited.end())
      {
        visited.insert(cur);
        if (cur.getType().isRealOrInt() && !cur.isConst())
        {
          Kind k = cur.getKind();
          if (k != Kind::MULT && k != Kind::ADD && k != Kind::NONLINEAR_MULT
              && k != Kind::TO_REAL
              && k != Kind::POW2)
          {
            // If we have not assigned it yet, use its exact model value.
            if (!hasAssignment(cur))
            {
              // set its exact model value in the substitution, if we compute
              // a constant value
              Node curv = computeConcreteModelValue(cur);
              if (curv.isConst())
              {
                if (TraceIsOn("nl-ext-cm"))
                {
                  Trace("nl-ext-cm")
                      << "check-model-bound : exact : " << cur << " = ";
                  printRationalApprox("nl-ext-cm", curv);
                  Trace("nl-ext-cm") << std::endl;
                }
                bool ret = addSubstitution(cur, curv);
                AlwaysAssert(ret);
              }
            }
          }
        }
        visit.insert(visit.end(), cur.begin(), cur.end());
      }
    } while (!visit.empty());
  }

  Trace("nl-ext-cm-debug") << "  check assertions..." << std::endl;
  std::vector<Node> check_assertions;
  for (const Node& a : assertions)
  {
    if (d_check_model_solved.find(a) == d_check_model_solved.end())
    {
      // apply the substitution to a
      Node av = getSubstitutedForm(a);
      Trace("nl-ext-cm") << "simpleCheckModelLit " << av << " (from " << a
                         << ")" << std::endl;
      // Every remaining assertion must hold under the exact substitution.
      if (!av.isConst() || !av.getConst<bool>())
      {
        Trace("nl-ext-cm") << "...check-model : assertion failed : " << a
                           << std::endl;
        check_assertions.push_back(av);
        Trace("nl-ext-cm-debug")
            << "...check-model : failed assertion, value : " << av << std::endl;
      }
    }
  }

  if (!check_assertions.empty())
  {
    Trace("nl-ext-cm") << "...simple check failed." << std::endl;
    // TODO (#1450) check model for general case
    return false;
  }
  Trace("nl-ext-cm") << "...simple check succeeded!" << std::endl;
  return true;
}

bool NlModel::addSubstitution(TNode v, TNode s)
{
  Assert(v.getKind() != Kind::TO_REAL);
  Trace("nl-ext-model") << "* check model substitution : " << v << " -> " << s
                        << std::endl;
  Assert(getSubstitutedForm(s) == s)
      << "Added a substitution whose range is not in substituted form " << s;
  // cannot substitute real for integer
  Assert(v.getType().isReal() || s.getType().isInteger());
  // should not substitute the same variable twice
  // should not set exact bound more than once
  if (d_substitutions.contains(v))
  {
    Node cur = d_substitutions.getSubs(v);
    if (cur != s)
    {
      Trace("nl-ext-model")
          << "...warning: already has value: " << cur << std::endl;
      // We set two different substitutions for a variable v. If both are
      // constant, then we throw an error. Otherwise, we ignore the newer
      // substitution and return false here.
      Assert(!cur.isConst() || !s.isConst())
          << "Conflicting exact bounds given for a variable (" << cur << " and "
          << s << ") for " << v;
      return false;
    }
  }
  // Check if the substitution is cyclic, considering arithmetic subterms.
  // This prevents an assignment like x -> (* 2 x) but allows an assignment
  // like x -> (f x) where f is an uninterpreted function.
  Node subsFull = d_substitutions.applyArith(s);
  if (ArithSubs::hasArithSubterm(subsFull, v))
  {
    Trace("nl-ext-model") << "ERROR: has subterm " << subsFull << std::endl;
    return false;
  }

  ArithSubs tmp;
  tmp.addArith(v, s);
  for (auto& sub : d_substitutions.d_subs)
  {
    Node ms = tmp.applyArith(sub);
    if (ms != sub)
    {
      sub = rewrite(ms);
    }
  }
  d_substitutions.addArith(v, s);
  return true;
}

bool NlModel::solveEqualitySimple(Node eq)
{
  Node seq = eq;
  if (!d_substitutions.empty())
  {
    seq = getSubstitutedForm(eq);
    if (seq.isConst())
    {
      if (seq.getConst<bool>())
      {
        // already true
        d_check_model_solved[eq] = Node::null();
        return true;
      }
      return false;
    }
  }
  Trace("nl-ext-cms") << "simple solve equality " << seq << "..." << std::endl;
  Assert(seq.getKind() == Kind::EQUAL);
  std::map<Node, Node> msum;
  if (!ArithMSum::getMonomialSumLit(seq, msum))
  {
    Trace("nl-ext-cms") << "...fail, could not determine monomial sum."
                        << std::endl;
    return false;
  }
  bool is_valid = true;
  // The variable to solve for in a linear equation.
  Node var;
  Node b = d_zero;
  Node c = d_zero;
  NodeManager* nm = nodeManager();
  // the list of variables that occur as a monomial in msum, and whose value
  // is so far unconstrained in the model.
  std::unordered_set<Node> unc_vars;
  // the list of variables that occur as a factor in a monomial, and whose
  // value is so far unconstrained in the model.
  std::unordered_set<Node> unc_vars_factor;
  for (std::pair<const Node, Node>& m : msum)
  {
    Node v = m.first;
    Node coeff = m.second.isNull() ? d_one : m.second;
    if (v.isNull())
    {
      c = coeff;
    }
    else if (v.getKind() == Kind::NONLINEAR_MULT)
    {
      is_valid = false;
      Trace("nl-ext-cms-debug")
          << "...invalid due to non-linear monomial " << v << std::endl;
      // may wish to set an exact bound for a factor and repeat
      for (const Node& vc : v)
      {
        unc_vars_factor.insert(vc);
      }
    }
    else if (!v.isVar() || (!var.isNull() && var != v))
    {
      Trace("nl-ext-cms-debug")
          << "...invalid due to factor " << v << std::endl;
      // cannot solve multivariate
      if (is_valid)
      {
        is_valid = false;
        // if b is non-zero, then var is also an unconstrained variable
        if (b != d_zero)
        {
          unc_vars.insert(var);
          unc_vars_factor.insert(var);
        }
      }
      // if v is unconstrained, we may turn this equality into a substitution
      unc_vars.insert(v);
      unc_vars_factor.insert(v);
    }
    else
    {
      // set the variable to solve for
      b = coeff;
      var = v;
    }
  }
  if (!is_valid)
  {
    // see if we can solve for a variable?
    for (const Node& uv : unc_vars)
    {
      Trace("nl-ext-cm-debug") << "check subs var : " << uv << std::endl;
      // cannot already have a bound
      if (uv.isVar() && !hasAssignment(uv))
      {
        Node slv;
        Node veqc;
        if (ArithMSum::isolate(uv, msum, veqc, slv, Kind::EQUAL) != 0)
        {
          Assert(!slv.isNull());
          // must rewrite here to be in substituted form
          slv = rewrite(slv);
          // Currently do not support substitution-with-coefficients.
          // We also ensure types are correct here, which avoids substituting
          // a term of non-integer type for a variable of integer type.
          if (veqc.isNull() && !expr::hasSubterm(slv, uv)
              && AVA6_EQUAL(slv.getType(), uv.getType()))
          {
            Trace("nl-ext-cm")
                << "check-model-subs : " << uv << " -> " << slv << std::endl;
            bool ret = addSubstitution(uv, slv);
            if (ret)
            {
              Trace("nl-ext-cms") << "...success, model substitution " << uv
                                  << " -> " << slv << std::endl;
              d_check_model_solved[eq] = uv;
            }
            return ret;
          }
        }
      }
    }
    // see if we can assign a variable to a constant
    for (const Node& uvf : unc_vars_factor)
    {
      Trace("nl-ext-cm-debug") << "check set var : " << uvf << std::endl;
      // cannot already have a bound
      if (uvf.isVar() && !hasAssignment(uvf))
      {
        Node uvfv = computeConcreteModelValue(uvf);
        // fail if model value is non-constant
        if (!uvfv.isConst())
        {
          return false;
        }
        if (TraceIsOn("nl-ext-cm"))
        {
          Trace("nl-ext-cm") << "check-model-bound : exact : " << uvf << " = ";
          printRationalApprox("nl-ext-cm", uvfv);
          Trace("nl-ext-cm") << std::endl;
        }
        bool ret = addSubstitution(uvf, uvfv);
        // recurse
        return ret ? solveEqualitySimple(eq) : false;
      }
    }
    Trace("nl-ext-cms") << "...fail due to constrained invalid terms."
                        << std::endl;
    return false;
  }
  else if (var.isNull() || var.getType().isInteger())
  {
    // Integer substitutions require additional divisibility reasoning.
    Trace("nl-ext-cms") << "...fail due to variable to solve for." << std::endl;
    return false;
  }

  // we are linear, it is simple
  if (b == d_zero)
  {
    Trace("nl-ext-cms") << "...fail due to zero a/b." << std::endl;
    DebugUnhandled();
    return false;
  }
  Node val = nm->mkConstReal(-c.getConst<Rational>() / b.getConst<Rational>());
  if (TraceIsOn("nl-ext-cm"))
  {
    Trace("nl-ext-cm") << "check-model-bound : exact : " << var << " = ";
    printRationalApprox("nl-ext-cm", val);
    Trace("nl-ext-cm") << std::endl;
  }
  bool ret = addSubstitution(var, val);
  if (ret)
  {
    Trace("nl-ext-cms") << "...success, solved linear." << std::endl;
    d_check_model_solved[eq] = var;
  }
  return ret;
}

void NlModel::printModelValue(const char* c, Node n, unsigned prec) const
{
  if (TraceIsOn(c))
  {
    Trace(c) << "  " << n << " -> ";
    const Node& aval = d_abstractModelCache.at(n);
    if (aval.isConst())
    {
      printRationalApprox(c, aval, prec);
    }
    else
    {
      Trace(c) << "?";
    }
    Trace(c) << " [actual: ";
    const Node& cval = d_concreteModelCache.at(n);
    if (cval.isConst())
    {
      printRationalApprox(c, cval, prec);
    }
    else
    {
      Trace(c) << "?";
    }
    Trace(c) << " ]" << std::endl;
  }
}

void NlModel::getModelValueRepair(std::map<Node, Node>& arithModel)
{
  NodeManager* nm = nodeManager();
  Trace("nl-model") << "NlModel::getModelValueRepair:" << std::endl;
  // If we extended the model with entries x -> 0 for unconstrained values,
  // we first update the map to the extended one.
  if (d_arithVal.size() > arithModel.size())
  {
    arithModel = d_arithVal;
  }
  for (size_t i = 0; i < d_substitutions.size(); ++i)
  {
    // overwrite, ensure the type is correct
    Node v = d_substitutions.d_vars[i];
    Node s = d_substitutions.d_subs[i];
    Node ss = s;
    // Ensure rational constants have the type of the substituted variable.
    if (s.isConst())
    {
      ss = nm->mkConstRealOrInt(v.getType(), s.getConst<Rational>());
    }
    arithModel[v] = ss;
    Trace("nl-model") << v << " solved is " << ss << std::endl;
  }

  // multiplication terms should not be given values; their values are
  // implied by the monomials that they consist of
  std::vector<Node> amErase;
  for (const std::pair<const Node, Node>& am : arithModel)
  {
    if (am.first.getKind() == Kind::NONLINEAR_MULT)
    {
      amErase.push_back(am.first);
    }
  }
  for (const Node& ae : amErase)
  {
    arithModel.erase(ae);
  }
}

Node NlModel::getValueInternal(TNode n)
{
  if (n.isConst())
  {
    return n;
  }
  if (auto it = d_arithVal.find(n); it != d_arithVal.end())
  {
    AlwaysAssert(it->second.isConst());
    return it->second;
  }
  // It is unconstrained in the model, return 0. We additionally add it
  // to mapping from the linear solver. This ensures that if the nonlinear
  // solver assumes that n = 0, then this assumption is recorded in the overall
  // model.
  Node zero = mkZero(n.getType());
  d_arithVal[n] = zero;
  return zero;
}

bool NlModel::hasAssignment(Node v) const
{
  return d_substitutions.contains(v);
}

bool NlModel::hasLinearModelValue(TNode v, Node& val) const
{
  auto it = d_arithVal.find(v);
  if (it != d_arithVal.end())
  {
    val = it->second;
    return true;
  }
  return false;
}

Node NlModel::getSubstitutedForm(TNode s) const
{
  if (d_substitutions.empty())
  {
    // no substitutions, just return s
    return s;
  }
  return rewrite(d_substitutions.applyArith(s));
}

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace ava6::internal
