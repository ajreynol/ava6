/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of ceg_bv_instantiator
 */

#include "theory/quantifiers/cegqi/ceg_bv_instantiator.h"

#include <stack>

#include "expr/skolem_manager.h"
#include "options/quantifiers_options.h"
#include "theory/bv/theory_bv_utils.h"
#include "theory/rewriter.h"
#include "util/bitvector.h"
#include "util/random.h"

using namespace std;
using namespace ava6::internal::kind;

namespace ava6::internal {
namespace theory {
namespace quantifiers {

// this class can be used to query the model value through the CegInstaniator
// class
class CegInstantiatorBvInverterQuery : public BvInverterQuery
{
 public:
  CegInstantiatorBvInverterQuery(CegInstantiator* ci)
      : BvInverterQuery(), d_ci(ci)
  {
  }
  ~CegInstantiatorBvInverterQuery() {}
  /** return the model value of n */
  Node getModelValue(Node n) override { return d_ci->getModelValue(n); }
  /** get bound variable of type tn */
  Node getBoundVariable(TypeNode tn) override
  {
    return d_ci->getBoundVariable(tn);
  }

 protected:
  // pointer to class that is able to query model values
  CegInstantiator* d_ci;
};

BvInstantiator::BvInstantiator(Env& env, TypeNode tn, BvInverter* inv)
    : Instantiator(env, tn), d_inverter(inv), d_inst_id_counter(0)
{
  // The inverter utility d_inverter is global to all BvInstantiator classes.
  // This must be global since we need to:
  // * process Skolem functions properly across multiple variables within the
  // same quantifier
  // * cache Skolem variables uniformly across multiple quantified formulas
}

BvInstantiator::~BvInstantiator() {}
void BvInstantiator::reset(AVA6_UNUSED CegInstantiator* ci,
                           AVA6_UNUSED SolvedForm& sf,
                           AVA6_UNUSED Node pv,
                           AVA6_UNUSED CegInstEffort effort)
{
  d_inst_id_counter = 0;
  d_var_to_inst_id.clear();
  d_inst_id_to_term.clear();
  d_inst_id_to_alit.clear();
}

void BvInstantiator::processLiteral(CegInstantiator* ci,
                                    AVA6_UNUSED SolvedForm& sf,
                                    Node pv,
                                    Node lit,
                                    Node alit,
                                    AVA6_UNUSED CegInstEffort effort)
{
  Assert(d_inverter != nullptr);
  // find path to pv
  std::vector<unsigned> path;
  Node sv = d_inverter->getSolveVariable(pv.getType());
  Node pvs = ci->getModelValue(pv);
  Trace("cegqi-bv") << "Get path to " << pv << " : " << lit << std::endl;
  Node slit = d_inverter->getPathToPv(
      lit, pv, sv, pvs, path, false);
  if (!slit.isNull())
  {
    CegInstantiatorBvInverterQuery m(ci);
    unsigned iid = d_inst_id_counter;
    Trace("cegqi-bv") << "Solve lit to bv inverter : " << slit << std::endl;
    Node inst = d_inverter->solveBvLit(sv, slit, path, &m);
    if (!inst.isNull())
    {
      inst = rewrite(inst);
      if (inst.isConst() || !ci->hasNestedQuantification())
      {
        Trace("cegqi-bv") << "...solved form is " << inst << std::endl;
        // store information for id and increment
        d_var_to_inst_id[pv].push_back(iid);
        d_inst_id_to_term[iid] = inst;
        d_inst_id_to_alit[iid] = alit;
        d_inst_id_counter++;
      }
    }
    else
    {
      Trace("cegqi-bv") << "...failed to solve." << std::endl;
    }
  }
  else
  {
    Trace("cegqi-bv") << "...no path." << std::endl;
  }
}

bool BvInstantiator::hasProcessAssertion(AVA6_UNUSED CegInstantiator* ci,
                                         AVA6_UNUSED SolvedForm& sf,
                                         AVA6_UNUSED Node pv,
                                         AVA6_UNUSED CegInstEffort effort)
{
  return true;
}

Node BvInstantiator::hasProcessAssertion(CegInstantiator* ci,
                                         AVA6_UNUSED SolvedForm& sf,
                                         AVA6_UNUSED Node pv,
                                         Node lit,
                                         CegInstEffort effort)
{
  if (effort == CEG_INST_EFFORT_FULL)
  {
    // always use model values at full effort
    return Node::null();
  }
  return processAssertionInternal(ci, lit);
}

Node BvInstantiator::processAssertionInternal(CegInstantiator* ci, Node lit)
{
  NodeManager* nm = lit.getNodeManager();
  Node atom = lit.getKind() == Kind::NOT ? lit[0] : lit;
  bool pol = lit.getKind() != Kind::NOT;
  Kind k = atom.getKind();
  if (k != Kind::EQUAL && k != Kind::BITVECTOR_ULT && k != Kind::BITVECTOR_SLT)
  {
    // others are unhandled
    return Node::null();
  }
  else if (!atom[0].getType().isBitVector())
  {
    return Node::null();
  }
  else if (false
           || (pol && k == Kind::EQUAL))
  {
    return lit;
  }
  Node s = atom[0];
  Node t = atom[1];

  Node sm = ci->getModelValue(s);
  Node tm = ci->getModelValue(t);
  Trace("cegqi-bv") << "Model value: " << std::endl;
  Trace("cegqi-bv") << "   " << s << " " << k << " " << t << " is "
                    << std::endl;
  Trace("cegqi-bv") << "   " << sm << " <> " << tm << std::endl;
  // in very rare cases e.g. strings of excessive length, the model value for
  // a term may not be constant, in which case we fail to solve.
  if (sm.isNull() || !sm.isConst() || tm.isNull() || !tm.isConst())
  {
    return Node::null();
  }

  Node ret;
  {
    // turn disequality into an inequality
    // e.g. s != t becomes s < t or t < s
    if (k == Kind::EQUAL)
    {
      if (Random::getRandom().pickWithProb(0.5))
      {
        std::swap(s, t);
      }
      pol = true;
    }
    // otherwise, we optimistically solve for the boundary point of an
    // inequality, for example:
    //   for s < t, we solve s+1 = t
    //   for ~( s < t ), we solve s = t
    // notice that this equality does not necessarily hold in the model, and
    // hence the corresponding instantiation strategy is not guaranteed to be
    // monotonic.
    if (!pol)
    {
      ret = s.eqNode(t);
    }
    else
    {
      Node bv_one = bv::utils::mkOne(nm, bv::utils::getSize(s));
      ret = NodeManager::mkNode(Kind::BITVECTOR_ADD, s, bv_one).eqNode(t);
    }
  }
  Trace("cegqi-bv") << "Process " << lit << " as " << ret << std::endl;
  return ret;
}

bool BvInstantiator::useModelValue(AVA6_UNUSED CegInstantiator* ci,
                                   AVA6_UNUSED SolvedForm& sf,
                                   AVA6_UNUSED Node pv,
                                   CegInstEffort effort)
{
  return effort < CEG_INST_EFFORT_FULL || options().solver.cegqiFullEffort;
}

bool BvInstantiator::processAssertions(CegInstantiator* ci,
                                       SolvedForm& sf,
                                       Node pv,
                                       AVA6_UNUSED CegInstEffort effort)
{
  std::unordered_map<Node, std::vector<unsigned>>::iterator iti =
      d_var_to_inst_id.find(pv);
  if (iti == d_var_to_inst_id.end())
  {
    // no bounds
    return false;
  }
  Trace("cegqi-bv") << "BvInstantiator::processAssertions for " << pv
                    << std::endl;
  bool firstVar = sf.empty();
  // get inst id list
  if (TraceIsOn("cegqi-bv"))
  {
    Trace("cegqi-bv") << "  " << iti->second.size()
                      << " candidate instantiations for " << pv << " : "
                      << std::endl;
    if (firstVar)
    {
      Trace("cegqi-bv") << "  ...this is the first variable" << std::endl;
    }
  }
  // until we have a model-preserving selection function for BV, this must
  // be heuristic (we only pick one literal)
  // hence we randomize the list
  // this helps robustness, since picking the same literal every time may
  // lead to a stream of value instantiations, whereas with randomization
  // we may find an invertible literal that leads to a useful instantiation.
  std::shuffle(iti->second.begin(), iti->second.end(), Random::getRandom());

  if (TraceIsOn("cegqi-bv"))
  {
    for (unsigned j = 0, size = iti->second.size(); j < size; j++)
    {
      unsigned inst_id = iti->second[j];
      Assert(d_inst_id_to_term.find(inst_id) != d_inst_id_to_term.end());
      Node inst_term = d_inst_id_to_term[inst_id];
      Assert(d_inst_id_to_alit.find(inst_id) != d_inst_id_to_alit.end());
      Node alit = d_inst_id_to_alit[inst_id];

      // debug printing
      Trace("cegqi-bv") << "   [" << j << "] : ";
      Trace("cegqi-bv") << inst_term << std::endl;
      Trace("cegqi-bv-debug") << "   ...from : " << alit << std::endl;
      Trace("cegqi-bv") << std::endl;
    }
  }

  // Try the first candidate in the shuffled list. Trying every candidate can
  // make instantiation construction exponential in the quantifier prefix.
  Assert(!iti->second.empty());
  unsigned inst_id = iti->second.front();
  Assert(d_inst_id_to_term.find(inst_id) != d_inst_id_to_term.end());
  Node inst_term = d_inst_id_to_term[inst_id];
  Node alit = d_inst_id_to_alit[inst_id];
  TermProperties pv_prop_bv;
  Trace("cegqi-bv") << "*** try " << pv << " -> " << inst_term << std::endl;
  ci->markSolved(alit);
  bool ret = ci->constructInstantiationInc(pv, inst_term, pv_prop_bv, sf);
  ci->markSolved(alit, false);
  if (!ret)
  {
    Trace("cegqi-bv") << "...failed to add instantiation for " << pv << std::endl;
  }
  return ret;
}

/** sort bv extract interval
 *
 * This sorts lists of bitvector extract terms where
 * ((_ extract i1 i2) t) < ((_ extract j1 j2) t)
 * if i1>j1 or i1=j1 and i2>j2.
 */
struct SortBvExtractInterval
{
  bool operator()(Node i, Node j)
  {
    Assert(i.getKind() == Kind::BITVECTOR_EXTRACT);
    Assert(j.getKind() == Kind::BITVECTOR_EXTRACT);
    BitVectorExtract ie = i.getOperator().getConst<BitVectorExtract>();
    BitVectorExtract je = j.getOperator().getConst<BitVectorExtract>();
    if (ie.d_high > je.d_high)
    {
      return true;
    }
    else if (ie.d_high == je.d_high)
    {
      Assert(ie.d_low != je.d_low);
      return ie.d_low > je.d_low;
    }
    return false;
  }
};

void BvInstantiatorPreprocess::registerCounterexampleLemma(
    Node lem, std::vector<Node>& ceVars, std::vector<Node>& auxLems)
{
  // new variables
  std::vector<Node> vars;
  // new lemmas
  std::vector<Node> new_lems;

  {
    NodeManager* nm = lem.getNodeManager();
    Trace("cegqi-bv-pp") << "-----remove extracts..." << std::endl;
    // map from terms to bitvector extracts applied to that term
    std::map<Node, std::vector<Node>> extract_map;
    std::unordered_set<TNode> visited;
    Trace("cegqi-bv-pp-debug2") << "Register ce lemma " << lem << std::endl;
    collectExtracts(lem, extract_map, visited);
    for (std::pair<const Node, std::vector<Node>>& es : extract_map)
    {
      // sort based on the extract start position
      std::vector<Node>& curr_vec = es.second;

      SortBvExtractInterval sbei;
      std::sort(curr_vec.begin(), curr_vec.end(), sbei);

      unsigned width = es.first.getType().getBitVectorSize();

      // list of points b such that:
      //   b>0 and we must start a segment at (b-1)  or  b==0
      std::vector<unsigned> boundaries;
      boundaries.push_back(width);
      boundaries.push_back(0);

      Trace("cegqi-bv-pp") << "For term " << es.first << " : " << std::endl;
      for (unsigned i = 0, size = curr_vec.size(); i < size; i++)
      {
        Trace("cegqi-bv-pp") << "  " << i << " : " << curr_vec[i] << std::endl;
        BitVectorExtract e =
            curr_vec[i].getOperator().getConst<BitVectorExtract>();
        if (std::find(boundaries.begin(), boundaries.end(), e.d_high + 1)
            == boundaries.end())
        {
          boundaries.push_back(e.d_high + 1);
        }
        if (std::find(boundaries.begin(), boundaries.end(), e.d_low)
            == boundaries.end())
        {
          boundaries.push_back(e.d_low);
        }
      }
      std::sort(boundaries.rbegin(), boundaries.rend());

      // make the extract variables
      std::vector<Node> children;
      for (unsigned i = 1; i < boundaries.size(); i++)
      {
        Assert(boundaries[i - 1] > 0);
        Node ex = bv::utils::mkExtract(
            es.first, boundaries[i - 1] - 1, boundaries[i]);
        Node var = NodeManager::mkDummySkolem("ek", ex.getType());
        children.push_back(var);
        vars.push_back(var);
      }

      Node conc = nm->mkNode(Kind::BITVECTOR_CONCAT, children);
      AssertEqual(conc.getType(), es.first.getType());
      Node eq_lem = conc.eqNode(es.first);
      Trace("cegqi-bv-pp") << "Introduced : " << eq_lem << std::endl;
      new_lems.push_back(eq_lem);
      Trace("cegqi-bv-pp") << "...finished processing extracts for term "
                           << es.first << std::endl;
    }
    Trace("cegqi-bv-pp") << "-----done remove extracts" << std::endl;
  }

  if (!vars.empty())
  {
    // could try applying subs -> vars here
    // in practice, this led to worse performance

    Trace("cegqi-bv-pp") << "Adding " << new_lems.size() << " lemmas..."
                         << std::endl;
    auxLems.insert(auxLems.end(), new_lems.begin(), new_lems.end());
    Trace("cegqi-bv-pp") << "Adding " << vars.size() << " variables..."
                         << std::endl;
    ceVars.insert(ceVars.end(), vars.begin(), vars.end());
  }
}

void BvInstantiatorPreprocess::collectExtracts(
    Node lem,
    std::map<Node, std::vector<Node>>& extract_map,
    std::unordered_set<TNode>& visited)
{
  std::vector<TNode> visit;
  TNode cur;
  visit.push_back(lem);
  do
  {
    cur = visit.back();
    visit.pop_back();
    if (visited.find(cur) == visited.end())
    {
      visited.insert(cur);
      if (cur.getKind() != Kind::FORALL)
      {
        if (cur.getKind() == Kind::BITVECTOR_EXTRACT)
        {
          if (cur[0].getKind() == Kind::INST_CONSTANT)
          {
            extract_map[cur[0]].push_back(cur);
          }
        }

        for (const Node& nc : cur)
        {
          visit.push_back(nc);
        }
      }
    }
  } while (!visit.empty());
}

}  // namespace quantifiers
}  // namespace theory
}  // namespace ava6::internal
