/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Simplifications for ITE expressions.
 *
 * This module implements preprocessing phases designed to simplify ITE
 * expressions.  Based on:
 * Kim, Somenzi, Jin.  Efficient Term-ITE Conversion for SMT.  FMCAD 2009.
 * Burch, Jerry.  Techniques for Verifying Superscalar Microprocessors.  DAC
 *'96
 */
#include "preprocessing/util/ite_utilities.h"

#include <utility>

#include "expr/skolem_manager.h"
#include "preprocessing/assertion_pipeline.h"
#include "preprocessing/passes/rewrite.h"
#include "theory/theory.h"
#include "util/rational.h"

using namespace std;
namespace ava6::internal {
namespace preprocessing {
namespace util {

namespace ite {

inline static bool isTermITE(TNode e)
{
  return (e.getKind() == Kind::ITE && !e.getType().isBoolean());
}

inline static bool triviallyContainsNoTermITEs(TNode e)
{
  return e.isConst() || e.isVar();
}

struct CTIVStackElement
{
  TNode curr;
  unsigned pos;
  CTIVStackElement() : curr(), pos(0) {}
  CTIVStackElement(TNode c) : curr(c), pos(0) {}
}; /* CTIVStackElement */

}  // namespace ite

/** ContainsTermITEVisitor. */
ContainsTermITEVisitor::ContainsTermITEVisitor() : d_cache() {}
ContainsTermITEVisitor::~ContainsTermITEVisitor() {}
bool ContainsTermITEVisitor::containsTermITE(TNode e)
{
  /* throughout execution skip through NOT nodes. */
  e = (e.getKind() == Kind::NOT) ? e[0] : e;
  if (ite::triviallyContainsNoTermITEs(e))
  {
    return false;
  }

  NodeBoolMap::const_iterator end = d_cache.end();
  NodeBoolMap::const_iterator tmp_it = d_cache.find(e);
  if (tmp_it != end)
  {
    return (*tmp_it).second;
  }

  bool foundTermIte = false;
  std::vector<ite::CTIVStackElement> stack;
  stack.push_back(ite::CTIVStackElement(e));
  while (!foundTermIte && !stack.empty())
  {
    ite::CTIVStackElement& top = stack.back();
    TNode curr = top.curr;
    if (top.pos >= curr.getNumChildren())
    {
      // all of the children have been visited
      // no term ites were found
      d_cache[curr] = false;
      stack.pop_back();
    }
    else
    {
      // this is someone's child
      TNode child = curr[top.pos];
      child = (child.getKind() == Kind::NOT) ? child[0] : child;
      ++top.pos;
      if (ite::triviallyContainsNoTermITEs(child))
      {
        // skip
      }
      else
      {
        tmp_it = d_cache.find(child);
        if (tmp_it != end)
        {
          foundTermIte = (*tmp_it).second;
        }
        else
        {
          stack.push_back(ite::CTIVStackElement(child));
          foundTermIte = ite::isTermITE(child);
        }
      }
    }
  }
  if (foundTermIte)
  {
    while (!stack.empty())
    {
      TNode curr = stack.back().curr;
      stack.pop_back();
      d_cache[curr] = true;
    }
  }
  return foundTermIte;
}
void ContainsTermITEVisitor::garbageCollect() { d_cache.clear(); }

 }  // namespace util
}  // namespace preprocessing
}  // namespace ava6::internal
