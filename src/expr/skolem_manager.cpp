/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of skolem manager class.
 */

#include "expr/skolem_manager.h"

#include <sstream>

#include "expr/attribute.h"
#include "expr/bound_var_manager.h"
#include "expr/node_algorithm.h"
#include "expr/node_manager_attributes.h"
#include "expr/sort_to_term.h"
#include "util/rational.h"
#include "util/string.h"

using namespace ava6::internal::kind;

namespace ava6::internal {

struct OriginalFormAttributeId
{
};
typedef expr::Attribute<OriginalFormAttributeId, Node> OriginalFormAttribute;

struct UnpurifiedFormAttributeId
{
};
typedef expr::Attribute<UnpurifiedFormAttributeId, Node>
    UnpurifiedFormAttribute;

SkolemManager::SkolemManager(NodeManager* nm) : d_nm(nm), d_skolemCounter(0) {}

Node SkolemManager::mkPurifySkolem(Node t)
{
  SkolemManager* skm = t.getNodeManager()->getSkolemManager();
  // We do not recursively compute the original form of t here
  Node k = skm->mkSkolemFunction(SkolemId::PURIFY, {t});
  Trace("sk-manager-skolem") << "skolem: " << k << " purify " << t << std::endl;
  return k;
}

Node SkolemManager::mkSkolemFunction(SkolemId id, Node cacheVal)
{
  std::vector<Node> cvals;
  if (!cacheVal.isNull())
  {
    if (cacheVal.getKind() == Kind::SEXPR)
    {
      cvals.insert(cvals.end(), cacheVal.begin(), cacheVal.end());
    }
    else
    {
      cvals.push_back(cacheVal);
    }
  }
  return mkSkolemFunction(id, cvals);
}

Node SkolemManager::mkSkolemFunction(SkolemId id,
                                     const std::vector<Node>& cacheVals)
{
  TypeNode ctn = getTypeFor(id, cacheVals);
  Assert(!ctn.isNull());
  return mkSkolemFunctionTyped(id, ctn, cacheVals);
}

Node SkolemManager::mkInternalSkolemFunction(InternalSkolemId id,
                                             TypeNode tn,
                                             const std::vector<Node>& cacheVals)
{
  std::vector<Node> cvals;
  cvals.push_back(d_nm->mkConstInt(Rational(static_cast<uint32_t>(id))));
  cvals.insert(cvals.end(), cacheVals.begin(), cacheVals.end());
  return mkSkolemFunctionTyped(SkolemId::INTERNAL, tn, cvals);
}

bool SkolemManager::isCommutativeSkolemId(SkolemId id)
{
  switch (id)
  {
    case ava6::SkolemId::ARRAY_DEQ_DIFF:

    case ava6::SkolemId::SETS_DEQ_DIFF:
    case ava6::SkolemId::STRINGS_DEQ_DIFF: return true;
    default: break;
  }
  return false;
}

Node SkolemManager::mkSkolemFunctionTyped(SkolemId id,
                                          TypeNode tn,
                                          Node cacheVal)
{
  std::tuple<SkolemId, TypeNode, Node> key(id, tn, cacheVal);
  std::map<std::tuple<SkolemId, TypeNode, Node>, Node>::iterator it =
      d_skolemFuns.find(key);
  if (it == d_skolemFuns.end())
  {
    // We use @ as a prefix, which follows the SMT-LIB standard indicating
    // internal symbols starting with @ or . are reserved for internal use.
    //
    std::stringstream ss;
    // Print internal skolems by the internal identifier, otherwise all would
    // be @INTERNAL_*.
    if (id == SkolemId::INTERNAL)
    {
      Node cval = cacheVal.getKind() == Kind::SEXPR ? cacheVal[0] : cacheVal;
      Assert(cval.getKind() == Kind::CONST_INTEGER);
      Rational r = cval.getConst<Rational>();
      Assert(r.sgn() >= 0 && r.getNumerator().fitsUnsignedInt());
      ss << "@"
         << static_cast<InternalSkolemId>(r.getNumerator().toUnsignedInt());
    }
    else
    {
      ss << "@" << id;
    }
    Node k = mkSkolemNode(Kind::SKOLEM, ss.str(), tn);
    if (id == SkolemId::PURIFY)
    {
      Assert(cacheVal.getType() == tn);
      // set unpurified form attribute for k
      UnpurifiedFormAttribute ufa;
      k.setAttribute(ufa, cacheVal);
      // the original form of k can be computed by calling getOriginalForm, but
      // it is not computed here
    }
    d_skolemFuns[key] = k;
    d_skolemFunMap[k] = key;
    Trace("sk-manager-skolem") << "mkSkolemFunction(" << id << ", " << cacheVal
                               << ") returns " << k << std::endl;
    return k;
  }
  return it->second;
}

Node SkolemManager::mkSkolemFunctionTyped(SkolemId id,
                                          TypeNode tn,
                                          const std::vector<Node>& cacheVals)
{
  Node cacheVal;
  // use null node if cacheVals is empty
  if (!cacheVals.empty())
  {
    cacheVal = cacheVals.size() == 1 ? cacheVals[0]
                                     : d_nm->mkNode(Kind::SEXPR, cacheVals);
  }
  return mkSkolemFunctionTyped(id, tn, cacheVal);
}

bool SkolemManager::isSkolemFunction(TNode k)
{
  return k.getKind() == Kind::SKOLEM;
}

bool SkolemManager::isSkolemFunction(TNode k, SkolemId& id, Node& cacheVal)
{
  SkolemManager* skm = k.getNodeManager()->getSkolemManager();
  if (k.getKind() != Kind::SKOLEM)
  {
    return false;
  }
  std::map<Node, std::tuple<SkolemId, TypeNode, Node>>::const_iterator it =
      skm->d_skolemFunMap.find(k);
  Assert(it != skm->d_skolemFunMap.end());
  id = std::get<0>(it->second);
  cacheVal = std::get<2>(it->second);
  return true;
}

SkolemId SkolemManager::getId(TNode k) const
{
  SkolemId id;
  Node cacheVal;
  if (isSkolemFunction(k, id, cacheVal))
  {
    return id;
  }
  return SkolemId::NONE;
}

std::vector<Node> SkolemManager::getIndices(TNode k) const
{
  std::vector<Node> vec;
  SkolemId id;
  Node cacheVal;
  if (isSkolemFunction(k, id, cacheVal))
  {
    if (!cacheVal.isNull())
    {
      if (cacheVal.getKind() == Kind::SEXPR)
      {
        vec.insert(vec.end(), cacheVal.begin(), cacheVal.end());
      }
      else
      {
        vec.push_back(cacheVal);
      }
    }
  }
  return vec;
}

InternalSkolemId SkolemManager::getInternalId(TNode k) const
{
  SkolemId id;
  Node cacheVal;
  // if its an internal skolem
  if (isSkolemFunction(k, id, cacheVal) && id == SkolemId::INTERNAL)
  {
    Assert(!cacheVal.isNull());
    Node cval = cacheVal.getKind() == Kind::SEXPR ? cacheVal[0] : cacheVal;
    Assert(cval.getKind() == Kind::CONST_INTEGER);
    Rational r = cval.getConst<Rational>();
    Assert(r.sgn() >= 0 && r.getNumerator().fitsUnsignedInt());
    return static_cast<InternalSkolemId>(r.getNumerator().toUnsignedInt());
  }
  return InternalSkolemId::NONE;
}

Node SkolemManager::mkDummySkolem(const std::string& prefix,
                                  const TypeNode& type,
                                  SkolemFlags flags)
{
  return mkSkolemNode(Kind::DUMMY_SKOLEM, prefix, type, flags);
}

bool SkolemManager::isAbstractValue(TNode n) const
{
  return (getInternalId(n) == InternalSkolemId::ABSTRACT_VALUE);
}

Node SkolemManager::getOriginalForm(Node n)
{
  if (n.isNull())
  {
    return n;
  }
  Trace("sk-manager-debug")
      << "SkolemManager::getOriginalForm " << n << std::endl;
  OriginalFormAttribute ofa;
  UnpurifiedFormAttribute ufa;
  NodeManager* nm = n.getNodeManager();
  std::unordered_set<TNode> visited;
  std::unordered_set<TNode>::iterator it;
  std::vector<TNode> visit;
  TNode cur;
  visit.push_back(n);
  do
  {
    cur = visit.back();
    if (cur.hasAttribute(ofa))
    {
      visit.pop_back();
      continue;
    }
    else if (cur.hasAttribute(ufa))
    {
      // if it has an unpurified form, compute the original form of it
      Node ucur = cur.getAttribute(ufa);
      if (ucur.hasAttribute(ofa))
      {
        // Already computed, set. This always happens after cur is visited
        // again after computing the original form of its unpurified form.
        Node ucuro = ucur.getAttribute(ofa);
        cur.setAttribute(ofa, ucuro);
        visit.pop_back();
      }
      else
      {
        // visit ucur then visit cur again
        visit.push_back(ucur);
      }
      continue;
    }
    else if (cur.getNumChildren() == 0)
    {
      cur.setAttribute(ofa, cur);
      visit.pop_back();
      continue;
    }
    it = visited.find(cur);
    if (it == visited.end())
    {
      visited.insert(cur);
      if (cur.getMetaKind() == metakind::PARAMETERIZED)
      {
        visit.push_back(cur.getOperator());
      }
      visit.insert(visit.end(), cur.begin(), cur.end());
      continue;
    }
    visit.pop_back();
    Node ret = cur;
    bool childChanged = false;
    std::vector<Node> children;
    if (cur.getMetaKind() == metakind::PARAMETERIZED)
    {
      const Node& oon = cur.getOperator().getAttribute(ofa);
      Assert(!oon.isNull());
      childChanged = childChanged || cur.getOperator() != oon;
      children.push_back(oon);
    }
    for (const Node& cn : cur)
    {
      const Node& ocn = cn.getAttribute(ofa);
      Assert(!ocn.isNull());
      childChanged = childChanged || cn != ocn;
      children.push_back(ocn);
    }
    if (childChanged)
    {
      ret = nm->mkNode(cur.getKind(), children);
    }
    cur.setAttribute(ofa, ret);

  } while (!visit.empty());
  const Node& on = n.getAttribute(ofa);
  Trace("sk-manager-debug") << "..return " << on << std::endl;
  return on;
}

Node SkolemManager::getUnpurifiedForm(Node k)
{
  UnpurifiedFormAttribute ufa;
  if (k.hasAttribute(ufa))
  {
    return k.getAttribute(ufa);
  }
  return k;
}

Node SkolemManager::mkSkolemNode(Kind k,
                                 const std::string& prefix,
                                 const TypeNode& type,
                                 SkolemFlags flags)
{
  Node n = NodeBuilder(d_nm, k);
  if ((flags & SkolemFlags::SKOLEM_EXACT_NAME)
      == SkolemFlags::SKOLEM_EXACT_NAME)
  {
    n.setAttribute(expr::VarNameAttr(), prefix);
  }
  else
  {
    std::stringstream name;
    name << prefix << '_' << ++d_skolemCounter;
    n.setAttribute(expr::VarNameAttr(), name.str());
  }
  n.setAttribute(expr::TypeAttr(), type);
  n.setAttribute(expr::TypeCheckedAttr(), true);
  return n;
}

TypeNode SkolemManager::getTypeFor(SkolemId id,
                                   const std::vector<Node>& cacheVals)
{
  switch (id)
  {
    // Type(cacheVals[0]), i.e skolems that return same type as first argument
    case SkolemId::PURIFY:

      Assert(cacheVals.size() > 0);
      return cacheVals[0].getType();
      break;
    case SkolemId::GROUND_TERM:
    case SkolemId::ARITH_VTS_INFINITY:
    case SkolemId::ARITH_VTS_INFINITY_FREE:
    {
      Assert(cacheVals[0].getKind() == Kind::SORT_TO_TERM);
      return cacheVals[0].getConst<SortToTerm>().getType();
    }
    // real -> real function
    case SkolemId::DIV_BY_ZERO:
    {
      TypeNode rtype = d_nm->realType();
      return d_nm->mkFunctionType(rtype, rtype);
    }
    // real skolems

    case SkolemId::ARITH_VTS_DELTA:
    case SkolemId::ARITH_VTS_DELTA_FREE: return d_nm->realType();
    // int -> int function
    case SkolemId::INT_DIV_BY_ZERO:
    case SkolemId::MOD_BY_ZERO:
    case SkolemId::STRINGS_OCCUR_INDEX:
    case SkolemId::STRINGS_OCCUR_INDEX_RE:
    case SkolemId::STRINGS_STOI_RESULT:
    case SkolemId::STRINGS_ITOS_RESULT:

    {
      TypeNode itype = d_nm->integerType();
      return d_nm->mkFunctionType(itype, itype);
    }
    case SkolemId::BV_EMPTY:
    {
      return d_nm->mkBitVectorType(0);
    }
    // int -> Type(args[0])
    case SkolemId::STRINGS_REPLACE_ALL_RESULT:
    case SkolemId::STRINGS_REPLACE_RE_ALL_RESULT:
    {
      Assert(cacheVals.size() == 3);
      TypeNode itype = d_nm->integerType();
      return d_nm->mkFunctionType(itype, cacheVals[0].getType());
    }
    // integer skolems
    case SkolemId::STRINGS_NUM_OCCUR:
    case SkolemId::STRINGS_NUM_OCCUR_RE:
    case SkolemId::STRINGS_DEQ_DIFF:
    case SkolemId::STRINGS_STOI_NON_DIGIT:

 return d_nm->integerType();
    // string skolems
    case SkolemId::RE_UNFOLD_POS_COMPONENT: return d_nm->stringType();
    case SkolemId::ARRAY_DEQ_DIFF:
    {
      Assert(cacheVals.size() == 2);
      TypeNode atype = cacheVals[0].getType();
      Assert(atype.isArray());
      return atype.getArrayIndexType();
    }
    case SkolemId::QUANTIFIERS_SKOLEMIZE:
    {
      Assert(cacheVals.size() == 2);
      Assert(cacheVals[0].getKind() == Kind::FORALL);
      Assert(cacheVals[1].getKind() == Kind::CONST_INTEGER);
      const Rational& r = cacheVals[1].getConst<Rational>();
      Assert(r.getNumerator().fitsUnsignedInt());
      size_t i = r.getNumerator().toUnsignedInt();
      Assert(i < cacheVals[0][0].getNumChildren());
      return cacheVals[0][0][i].getType();
    }
    break;
    case SkolemId::WITNESS_STRING_LENGTH:
    {
      Assert(cacheVals.size() == 3);
      Assert(cacheVals[0].getKind() == Kind::SORT_TO_TERM);
      Assert(cacheVals[1].getKind() == Kind::CONST_INTEGER);
      Assert(cacheVals[2].getKind() == Kind::CONST_INTEGER);
      TypeNode t = cacheVals[0].getConst<SortToTerm>().getType();
      return t;
    }
    break;
    case SkolemId::WITNESS_INV_CONDITION:
    {
      Assert(cacheVals.size() == 1);
      Assert(cacheVals[0].getKind() == Kind::EXISTS);
      Assert(cacheVals[0][0].getNumChildren() == 1);
      return cacheVals[0][0][0].getType();
    }
    break;
    // skolems that return the set element type

    case SkolemId::SETS_DEQ_DIFF:
    {
      Assert(cacheVals.size() > 0);
      TypeNode stype = cacheVals[0].getType();
      Assert(stype.getNumChildren() == 1);
      return stype[0];
    }
    // skolems that return the set to set element type

    case SkolemId::SETS_CHOOSE:
    {
      Assert(cacheVals.size() > 0);
      TypeNode stype = cacheVals[0].getType();
      Assert(stype.getNumChildren() == 1);
      return d_nm->mkFunctionType(stype, stype[0]);
    }
    default: break;
  }
  return TypeNode();
}

size_t SkolemManager::getNumIndicesForSkolemId(SkolemId id) const
{
  switch (id)
  {
    // Number of skolem indices: 0
    case SkolemId::ARITH_VTS_DELTA:
    case SkolemId::ARITH_VTS_DELTA_FREE:
    case SkolemId::BV_EMPTY:
    case SkolemId::DIV_BY_ZERO:
    case SkolemId::INT_DIV_BY_ZERO:
    case SkolemId::MOD_BY_ZERO: return 0;

    // Number of skolem indices: 1
    case SkolemId::PURIFY:
    case SkolemId::GROUND_TERM:

    case SkolemId::ARITH_VTS_INFINITY:
    case SkolemId::ARITH_VTS_INFINITY_FREE:
    case SkolemId::WITNESS_INV_CONDITION:
    case SkolemId::STRINGS_ITOS_RESULT:
    case SkolemId::STRINGS_STOI_RESULT:
    case SkolemId::STRINGS_STOI_NON_DIGIT:

    case SkolemId::SETS_CHOOSE:

 return 1;

    // Number of skolem indices: 2
    case SkolemId::ARRAY_DEQ_DIFF:
    case SkolemId::QUANTIFIERS_SKOLEMIZE:
    case SkolemId::STRINGS_NUM_OCCUR:
    case SkolemId::STRINGS_OCCUR_INDEX:
    case SkolemId::STRINGS_NUM_OCCUR_RE:
    case SkolemId::STRINGS_OCCUR_INDEX_RE:
    case SkolemId::STRINGS_DEQ_DIFF:

    case SkolemId::SETS_DEQ_DIFF:

 return 2;

    // Number of skolem indices: 3

    case SkolemId::WITNESS_STRING_LENGTH:
    case SkolemId::STRINGS_REPLACE_ALL_RESULT:
    case SkolemId::STRINGS_REPLACE_RE_ALL_RESULT:
    case SkolemId::RE_UNFOLD_POS_COMPONENT:

 return 3;

    // Number of skolem indices: 5
    default: Unimplemented() << "Unknown skolem kind " << id; break;
  }
}

}  // namespace ava6::internal
