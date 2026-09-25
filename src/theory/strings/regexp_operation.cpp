/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Symbolic Regular Expresion Operations
 */

#include "theory/strings/regexp_operation.h"

#include <sstream>

#include "expr/node_algorithm.h"
#include "theory/rewriter.h"
#include "theory/strings/regexp_entail.h"
#include "theory/strings/theory_strings_utils.h"
#include "theory/strings/word.h"
#include "util/regexp.h"

using namespace ava6::internal::kind;

namespace ava6::internal {
namespace theory {
namespace strings {

RegExpOpr::RegExpOpr(Env& env, SkolemCache* sc)
    : EnvObj(env),
      d_emptyString(Word::mkEmptyWord(nodeManager()->stringType())),
      d_sc(sc)
{
}

RegExpOpr::~RegExpOpr() {}

bool RegExpOpr::checkConstRegExp(Node r)
{
  Assert(r.getType().isRegExp());
  Trace("strings-regexp-cstre")
      << "RegExpOpr::checkConstRegExp /" << mkString(r) << "/" << std::endl;
  RegExpConstType rct = getRegExpConstType(r);
  return rct != RE_C_VARIABLE;
}

RegExpConstType RegExpOpr::getRegExpConstType(Node r)
{
  Assert(r.getType().isRegExp());
  std::unordered_map<Node, RegExpConstType>::iterator it;
  std::vector<TNode> visit;
  TNode cur;
  visit.push_back(r);
  do
  {
    cur = visit.back();
    visit.pop_back();
    it = d_constCache.find(cur);

    Kind ck = cur.getKind();
    if (it == d_constCache.end())
    {
      if (ck == Kind::STRING_TO_REGEXP)
      {
        Node tmp = rewrite(cur[0]);
        d_constCache[cur] =
            tmp.isConst() ? RE_C_CONCRETE_CONSTANT : RE_C_VARIABLE;
      }
      else if (ck == Kind::REGEXP_ALLCHAR || ck == Kind::REGEXP_RANGE)
      {
        d_constCache[cur] = RE_C_CONSTANT;
      }
      else if (!utils::isRegExpKind(ck))
      {
        // non-regular expression applications, e.g. function applications
        // with regular expression return type are treated as variables.
        d_constCache[cur] = RE_C_VARIABLE;
      }
      else
      {
        d_constCache[cur] = RE_C_UNKNOWN;
        visit.push_back(cur);
        visit.insert(visit.end(), cur.begin(), cur.end());
      }
    }
    else if (it->second == RE_C_UNKNOWN)
    {
      RegExpConstType ret = ck == Kind::REGEXP_COMPLEMENT
                                ? RE_C_CONSTANT
                                : RE_C_CONCRETE_CONSTANT;
      for (const Node& cn : cur)
      {
        it = d_constCache.find(cn);
        Assert(it != d_constCache.end());
        if (it->second > ret)
        {
          ret = it->second;
        }
      }
      d_constCache[cur] = ret;
    }
  } while (!visit.empty());
  Assert(d_constCache.find(r) != d_constCache.end());
  return d_constCache[r];
}

// 0-unknown, 1-yes, 2-no
int RegExpOpr::delta(Node r, Node& exp)
{
  std::map<Node, std::pair<int, Node> >::const_iterator itd =
      d_delta_cache.find(r);
  if (itd != d_delta_cache.end())
  {
    // already computed
    exp = itd->second.second;
    return itd->second.first;
  }
  Trace("regexp-delta") << "RegExpOpr::delta: " << r << std::endl;
  int ret = 0;
  NodeManager* nm = nodeManager();
  Kind k = r.getKind();
  switch (k)
  {
    case Kind::REGEXP_NONE:
    case Kind::REGEXP_ALLCHAR:
    case Kind::REGEXP_RANGE:
    {
      // does not contain empty string
      ret = 2;
      break;
    }
    case Kind::STRING_TO_REGEXP:
    {
      Node tmp = rewrite(r[0]);
      if (tmp.isConst())
      {
        if (tmp == d_emptyString)
        {
          ret = 1;
        }
        else
        {
          ret = 2;
        }
      }
      else
      {
        ret = 0;
        if (tmp.getKind() == Kind::STRING_CONCAT)
        {
          for (const Node& tmpc : tmp)
          {
            if (tmpc.isConst())
            {
              ret = 2;
              break;
            }
          }
        }
        if (ret == 0)
        {
          exp = r[0].eqNode(d_emptyString);
        }
      }
      break;
    }
    case Kind::REGEXP_CONCAT:
    case Kind::REGEXP_UNION:
    case Kind::REGEXP_INTER:
    {
      // has there been an unknown child?
      bool hasUnknownChild = false;
      std::vector<Node> vec;
      int checkTmp = k == Kind::REGEXP_UNION ? 1 : 2;
      int retTmp = k == Kind::REGEXP_UNION ? 2 : 1;
      for (const Node& rc : r)
      {
        Node exp2;
        int tmp = delta(rc, exp2);
        if (tmp == checkTmp)
        {
          // return is implied by the child's return value
          ret = checkTmp;
          break;
        }
        else if (tmp == 0)
        {
          // unknown if child contains empty string
          Assert(!exp2.isNull());
          vec.push_back(exp2);
          hasUnknownChild = true;
        }
      }
      if (ret != checkTmp)
      {
        if (!hasUnknownChild)
        {
          ret = retTmp;
        }
        else
        {
          Kind kr = k == Kind::REGEXP_UNION ? Kind::OR : Kind::AND;
          exp = vec.size() == 1 ? vec[0] : nm->mkNode(kr, vec);
        }
      }
      break;
    }
    case Kind::REGEXP_STAR:
    case Kind::REGEXP_OPT:
    {
      // contains empty string
      ret = 1;
      break;
    }
    case Kind::REGEXP_PLUS:
    {
      ret = delta(r[0], exp);
      break;
    }
    case Kind::REGEXP_LOOP:
    {
      uint32_t lo = utils::getLoopMinOccurrences(r);
      if (lo == 0)
      {
        ret = 1;
      }
      else
      {
        ret = delta(r[0], exp);
      }
      break;
    }
    case Kind::REGEXP_COMPLEMENT:
    {
      int tmp = delta(r[0], exp);
      // flip the result if known
      ret = tmp == 0 ? 0 : (3 - tmp);
      exp = exp.isNull() ? exp : exp.negate();
      break;
    }
    default:
    {
      Assert(!utils::isRegExpKind(k));
      break;
    }
  }
  if (!exp.isNull())
  {
    exp = rewrite(exp);
  }
  std::pair<int, Node> p(ret, exp);
  d_delta_cache[r] = p;
  Trace("regexp-delta") << "RegExpOpr::delta returns " << ret << " for " << r
                        << ", expr = " << exp << std::endl;
  return ret;
}

Node RegExpOpr::simplify(Node t, bool polarity)
{
  Trace("strings-regexp-simpl")
      << "RegExpOpr::simplify: " << t << ", polarity=" << polarity << std::endl;
  Assert(t.getKind() == Kind::STRING_IN_REGEXP);
  Node tlit = polarity ? t : t.notNode();
  Node conc;
  std::map<Node, Node>::const_iterator itr = d_simpCache.find(tlit);
  if (itr != d_simpCache.end())
  {
    return itr->second;
  }
  if (polarity)
  {
    std::vector<Node> newSkolems;
    conc = reduceRegExpPos(nodeManager(), tlit, d_sc, newSkolems);
  }
  else
  {
    // see if we can use an optimized version of the reduction for re.++.
    Node r = t[1];
    if (r.getKind() == Kind::REGEXP_CONCAT)
    {
      // the index we are removing from the RE concatenation
      bool isRev;
      // As an optimization to the reduction, if we can determine that
      // all strings in the language of R1 have the same length, say n,
      // then the conclusion of the reduction is quantifier-free:
      //    ~( substr(s,0,n) in R1 ) OR ~( substr(s,len(s)-n,n) in R2)
      Node reLen = getRegExpConcatFixed(r, isRev);
      if (!reLen.isNull())
      {
        conc = reduceRegExpNegConcatFixed(nodeManager(), tlit, reLen, isRev);
      }
    }
    if (conc.isNull())
    {
      conc = reduceRegExpNeg(nodeManager(), tlit);
    }
  }
  d_simpCache[tlit] = conc;
  Trace("strings-regexp-simpl")
      << "RegExpOpr::simplify: returns " << conc << std::endl;
  return conc;
}

Node RegExpOpr::getRegExpConcatFixed(Node r, bool& isRev)
{
  Assert(r.getKind() == Kind::REGEXP_CONCAT);
  isRev = false;
  Node reLen = RegExpEntail::getFixedLengthForRegexp(r[0]);
  if (!reLen.isNull())
  {
    return reLen;
  }
  // try from the opposite end
  size_t indexE = r.getNumChildren() - 1;
  reLen = RegExpEntail::getFixedLengthForRegexp(r[indexE]);
  if (!reLen.isNull())
  {
    isRev = true;
    return reLen;
  }
  return Node::null();
}

Node RegExpOpr::reduceRegExpNeg(NodeManager* nm, Node mem)
{
  Assert(mem.getKind() == Kind::NOT
         && mem[0].getKind() == Kind::STRING_IN_REGEXP);
  Node s = mem[0][0];
  Node r = mem[0][1];
  Kind k = r.getKind();
  Node zero = nm->mkConstInt(Rational(0));
  Node conc;
  if (k == Kind::REGEXP_CONCAT)
  {
    // do not use length entailment, call regular expression concat
    Node reLen;
    conc = reduceRegExpNegConcatFixed(nm, mem, reLen, false);
  }
  else if (k == Kind::REGEXP_STAR)
  {
    Node emp = Word::mkEmptyWord(s.getType());
    Node lens = nm->mkNode(Kind::STRING_LENGTH, s);
    Node sne = s.eqNode(emp).negate();
    Node b1 = SkolemCache::mkIndexVar(nm, mem);
    Node b1v = nm->mkNode(Kind::BOUND_VAR_LIST, b1);
    Node g11n = nm->mkNode(Kind::LEQ, b1, zero);
    Node g12n = nm->mkNode(Kind::LT, lens, b1);
    // internal
    Node s1 = utils::mkPrefix(s, b1);
    Node s2 = utils::mkSuffix(s, b1);
    Node s1r1 = nm->mkNode(Kind::STRING_IN_REGEXP, s1, r[0]).negate();
    Node s2r2 = nm->mkNode(Kind::STRING_IN_REGEXP, s2, r).negate();

    conc = nm->mkNode(Kind::OR, {g11n, g12n, s1r1, s2r2});
    // must mark as an internal quantifier
    conc = utils::mkForallInternal(nm, b1v, conc);
    conc = nm->mkNode(Kind::AND, sne, conc);
  }
  else
  {
    Assert(!utils::isRegExpKind(k));
  }
  return conc;
}

Node RegExpOpr::reduceRegExpNegConcatFixed(NodeManager* nm,
                                           Node mem,
                                           Node reLen,
                                           bool isRev)
{
  Assert(mem.getKind() == Kind::NOT
         && mem[0].getKind() == Kind::STRING_IN_REGEXP);
  Node s = mem[0][0];
  Node r = mem[0][1];
  Assert(r.getKind() == Kind::REGEXP_CONCAT);
  Node zero = nm->mkConstInt(Rational(0));
  // The following simplification states that
  //    ~( s in R1 ++ R2 ++... ++ Rn )
  // is equivalent to
  //    forall x.
  //      0 <= x <= len(s) =>
  //        ~(substr(s,0,x) in R1) OR ~(substr(s,x,len(s)-x) in R2 ++ ... ++ Rn)
  // Index is the child index of r that we are stripping off, which is either
  // from the beginning or the end.
  Node lens = nm->mkNode(Kind::STRING_LENGTH, s);
  Node b1;
  Node b1v;
  Node guard1n, guard2n;
  if (reLen.isNull())
  {
    b1 = SkolemCache::mkIndexVar(nm, mem);
    b1v = nm->mkNode(Kind::BOUND_VAR_LIST, b1);
    guard1n = nm->mkNode(Kind::LT, b1, zero);
    guard2n = nm->mkNode(Kind::LT, nm->mkNode(Kind::STRING_LENGTH, s), b1);
  }
  else
  {
    b1 = reLen;
  }
  Node s1;
  Node s2;
  if (!isRev)
  {
    s1 = utils::mkPrefix(s, b1);
    s2 = utils::mkSuffix(s, b1);
  }
  else
  {
    s1 = utils::mkSuffixOfLen(s, b1);
    s2 = utils::mkPrefix(s, nm->mkNode(Kind::SUB, lens, b1));
  }
  size_t index = isRev ? r.getNumChildren() - 1 : 0;
  Node s1r1 = nm->mkNode(Kind::STRING_IN_REGEXP, s1, r[index]).negate();
  std::vector<Node> nvec;
  for (unsigned i = 0, nchild = r.getNumChildren(); i < nchild; i++)
  {
    if (i != index)
    {
      nvec.push_back(r[i]);
    }
  }
  Node r2 = nvec.size() == 1 ? nvec[0] : nm->mkNode(Kind::REGEXP_CONCAT, nvec);
  Node s2r2 = nm->mkNode(Kind::STRING_IN_REGEXP, s2, r2).negate();
  Node conc;
  if (!b1v.isNull())
  {
    conc = nm->mkNode(Kind::OR, {guard1n, guard2n, s1r1, s2r2});
    // must mark as an internal quantifier
    conc = utils::mkForallInternal(nm, b1v, conc);
  }
  else
  {
    conc = nm->mkNode(Kind::OR, s1r1, s2r2);
  }
  return conc;
}

Node RegExpOpr::reduceRegExpPos(NodeManager* nm,
                                Node mem,
                                SkolemCache* sc,
                                std::vector<Node>& newSkolems)
{
  Assert(mem.getKind() == Kind::STRING_IN_REGEXP);
  Node s = mem[0];
  Node r = mem[1];
  Kind k = r.getKind();
  Node conc;
  if (k == Kind::REGEXP_CONCAT)
  {
    std::vector<Node> nvec;
    std::vector<Node> cc;
    SkolemManager* sm = nm->getSkolemManager();
    // Look up skolems for each of the components. If sc has optimizations
    // enabled, this will return arguments of str.to_re.
    for (unsigned i = 0, nchild = r.getNumChildren(); i < nchild; ++i)
    {
      if (r[i].getKind() == Kind::STRING_TO_REGEXP)
      {
        // optimization, just take the body
        newSkolems.push_back(r[i][0]);
      }
      else
      {
        Node ivalue = nm->mkConstInt(Rational(i));
        Node sk = sm->mkSkolemFunction(SkolemId::RE_UNFOLD_POS_COMPONENT,
                                       {mem[0], mem[1], ivalue});
        newSkolems.push_back(sk);
        nvec.push_back(nm->mkNode(Kind::STRING_IN_REGEXP, newSkolems[i], r[i]));
      }
    }
    // (str.in_re x (re.++ R0 .... Rn)) =>
    // (and (= x (str.++ k0 ... kn)) (str.in_re k0 R0) ... (str.in_re kn Rn) )
    Node lem = s.eqNode(nm->mkNode(Kind::STRING_CONCAT, newSkolems));
    nvec.insert(nvec.begin(), lem);
    conc = nvec.size() == 1 ? nvec[0] : nm->mkNode(Kind::AND, nvec);
  }
  else if (k == Kind::REGEXP_STAR)
  {
    Node emp = Word::mkEmptyWord(s.getType());
    Node se = s.eqNode(emp);
    Node sinr = nm->mkNode(Kind::STRING_IN_REGEXP, s, r[0]);
    Node empr = nm->mkNode(Kind::STRING_TO_REGEXP, emp);
    Node rd = nm->mkNode(Kind::REGEXP_DIFF, r[0], empr);
    Node reExpand = nm->mkNode(Kind::REGEXP_CONCAT, rd, r, rd);
    Node sinRExp = nm->mkNode(Kind::STRING_IN_REGEXP, s, reExpand);
    // We unfold `x in R*` by considering three cases: `x` is empty, `x`
    // is matched by `R`, or `x` is matched by two or more `R`s. For the
    // last case, `x` will break into three pieces, making the beginning
    // and the end each match `R` and the middle match `R*`. Matching the
    // beginning and the end with `R` allows us to reason about the
    // beginning and the end of `x` simultaneously.
    //
    // x in R* ---> (x = "") v (x in R) v (x in (re.++ R (re.* R) R))

    // We also immediately unfold the last disjunct for re.*. The advantage
    // of doing this is that we use the same scheme for skolems above.
    std::vector<Node> newSkolemsC;
    sinRExp = reduceRegExpPos(nm, sinRExp, sc, newSkolemsC);
    Assert(newSkolemsC.size() == 3);
    // make the return lemma
    // can also assume the component match the first and last R are non-empty.
    // This means that the overall conclusion is:
    //  (x = "") v (x in R) v (x = (str.++ k1 k2 k3) ^
    //                         k1 in (R \ "") ^ k2 in (re.* R) ^ k3 in (R \ ""))
    conc = nm->mkNode(Kind::OR, se, sinr, sinRExp);
  }
  else
  {
    Assert(!utils::isRegExpKind(k));
  }
  return conc;
}

// printing
std::string RegExpOpr::niceChar(Node r)
{
  if (r.isConst())
  {
    std::string s = r.getConst<String>().toString();
    return s == "." ? "\\." : s;
  }
  else
  {
    std::string ss = "$" + r.toString();
    return ss;
  }
}
std::string RegExpOpr::mkString(Node r)
{
  std::string retStr;
  if (r.isNull())
  {
    retStr = "\\E";
  }
  else
  {
    Kind k = r.getKind();
    switch (k)
    {
      case Kind::REGEXP_NONE:
      {
        retStr += "\\E";
        break;
      }
      case Kind::REGEXP_ALLCHAR:
      {
        retStr += ".";
        break;
      }
      case Kind::STRING_TO_REGEXP:
      {
        std::string tmp(niceChar(r[0]));
        retStr += tmp.size() == 1 ? tmp : "(" + tmp + ")";
        break;
      }
      case Kind::REGEXP_CONCAT:
      {
        retStr += "(";
        for (unsigned i = 0; i < r.getNumChildren(); ++i)
        {
          // if(i != 0) retStr += ".";
          retStr += mkString(r[i]);
        }
        retStr += ")";
        break;
      }
      case Kind::REGEXP_UNION:
      {
        retStr += "(";
        for (unsigned i = 0; i < r.getNumChildren(); ++i)
        {
          if (i != 0) retStr += "|";
          retStr += mkString(r[i]);
        }
        retStr += ")";
        break;
      }
      case Kind::REGEXP_INTER:
      {
        retStr += "(";
        for (unsigned i = 0; i < r.getNumChildren(); ++i)
        {
          if (i != 0) retStr += "&";
          retStr += mkString(r[i]);
        }
        retStr += ")";
        break;
      }
      case Kind::REGEXP_STAR:
      {
        retStr += mkString(r[0]);
        retStr += "*";
        break;
      }
      case Kind::REGEXP_PLUS:
      {
        retStr += mkString(r[0]);
        retStr += "+";
        break;
      }
      case Kind::REGEXP_OPT:
      {
        retStr += mkString(r[0]);
        retStr += "?";
        break;
      }
      case Kind::REGEXP_RANGE:
      {
        retStr += "[";
        retStr += niceChar(r[0]);
        retStr += "-";
        retStr += niceChar(r[1]);
        retStr += "]";
        break;
      }
      case Kind::REGEXP_LOOP:
      {
        uint32_t l = utils::getLoopMinOccurrences(r);
        std::stringstream ss;
        ss << "(" << mkString(r[0]) << "){" << l << ",";
        if (r.getNumChildren() == 3)
        {
          uint32_t u = utils::getLoopMaxOccurrences(r);
          ss << u;
        }
        ss << "}";
        retStr += ss.str();
        break;
      }
      case Kind::REGEXP_COMPLEMENT:
      {
        retStr += "^(";
        retStr += mkString(r[0]);
        retStr += ")";
        break;
      }
      default:
      {
        std::stringstream ss;
        ss << r;
        retStr = ss.str();
        Assert(!utils::isRegExpKind(r.getKind()));
        break;
      }
    }
  }

  return retStr;
}

bool RegExpOpr::regExpIncludes(Node r1, Node r2)
{
  return RegExpEntail::regExpIncludes(r1, r2, d_inclusionCache);
}

}  // namespace strings
}  // namespace theory
}  // namespace ava6::internal
