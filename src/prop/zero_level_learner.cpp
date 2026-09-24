/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Learner for literals asserted at level zero.
 */
#include "prop/zero_level_learner.h"

#include "context/context.h"
#include "expr/node_algorithm.h"
#include "expr/skolem_manager.h"
#include "options/base_options.h"
#include "options/prop_options.h"
#include "options/smt_options.h"
#include "smt/env.h"
#include "theory/theory_engine.h"
#include "theory/trust_substitutions.h"

namespace ava6::internal {
namespace prop {

ZeroLevelLearner::ZeroLevelLearner(Env& env, TheoryEngine* theoryEngine)
    : EnvObj(env),
      d_theoryEngine(theoryEngine),
      d_levelZeroAsserts(userContext()),
      d_ldb(userContext()),
      d_nonZeroAssert(context(), false),
      d_ppnAtoms(userContext()),
      d_ppnTerms(userContext()),
      d_ppnSyms(userContext()),
      d_tsmap(env, userContext(), "ZllSimplificationMap")
{
}

ZeroLevelLearner::~ZeroLevelLearner() {}

void ZeroLevelLearner::getAtoms(TNode a,
                                std::unordered_set<TNode>& visited,
                                std::unordered_set<Node>& atoms)
{
  std::vector<TNode> visit;
  TNode cur;
  visit.push_back(a);
  do
  {
    cur = visit.back();
    visit.pop_back();
    if (visited.find(cur) == visited.end())
    {
      visited.insert(cur);
      if (expr::isBooleanConnective(cur))
      {
        visit.insert(visit.end(), cur.begin(), cur.end());
        continue;
      }
      atoms.insert(cur);
    }
  } while (!visit.empty());
}

void ZeroLevelLearner::notifyTopLevelSubstitution(const Node& lhs,
                                                  const Node& rhs)
{
  // process as a preprocess solved learned literal.
  Node eq = lhs.eqNode(rhs);
  processLearnedLiteral(eq, LearnedLitType::PREPROCESS_SOLVED);
}

void ZeroLevelLearner::notifyInputFormulas(const std::vector<Node>& assertions)
{
  std::unordered_set<TNode> visited;
  std::unordered_set<TNode> visitedWithinAtom;
  std::unordered_set<Node> inputSymbols;
  // We consider top level literals of assertions, including those occurring
  // as children of AND to be the preprocessed learned literals only, and not
  // the literals tracked by the preprocessor
  // (Preprocessor::getLearnedLiterals). This means that a learned literal from
  // e.g. circuit propagation that is not trivially a top level assertion will
  // be considered an ordinary learned literal.
  // Note that d_pplAtoms and d_ppnAtoms are disjoint
  std::vector<Node> toProcess = assertions;
  size_t index = 0;
  while (index < toProcess.size())
  {
    TNode lit = toProcess[index];
    index++;
    if (lit.getKind() == Kind::AND)
    {
      toProcess.insert(toProcess.end(), lit.begin(), lit.end());
      continue;
    }
    TNode atom = lit.getKind() == Kind::NOT ? lit[0] : lit;
    if (expr::isBooleanConnective(atom))
    {
      continue;
    }
    // we mark that we visited this
    visited.insert(atom);
    // ignore the true node
    if (!lit.isConst() || !lit.getConst<bool>())
    {
      // output learned literals from preprocessing
      {
        computeLearnedLiteralType(lit);
      }
      processLearnedLiteral(lit, LearnedLitType::PREPROCESS);
      // also get its symbols
      expr::getSymbols(atom, inputSymbols, visitedWithinAtom);
    }
    // remember we've seen it
    d_levelZeroAsserts.insert(lit);
  }
  // Compute the set of literals in the preprocessed assertions
  std::unordered_set<Node> inputAtoms;
  for (const Node& a : assertions)
  {
    getAtoms(a, visited, inputAtoms);
  }
  for (const Node& a : inputAtoms)
  {
    d_ppnAtoms.insert(a);
    // also get its symbols
    expr::getSymbols(a, inputSymbols, visitedWithinAtom);
  }
  for (const TNode& t : visitedWithinAtom)
  {
    d_ppnTerms.insert(t);
  }
  for (const Node& s : inputSymbols)
  {
    d_ppnSyms.insert(s);
  }

  Trace("level-zero") << "Preprocess status:" << std::endl;
  Trace("level-zero") << "#Non-learned lits = " << d_ppnAtoms.size()
                      << std::endl;
  Trace("level-zero") << "#Symbols = " << d_ppnSyms.size() << std::endl;
  Trace("level-zero") << "#Subterms = " << d_ppnTerms.size() << std::endl;
  Trace("level-zero") << "#Current top level subs = "
                      << d_env.getTopLevelSubstitutions().get().size()
                      << std::endl;
  Trace("level-zero") << d_ldb.toStringDebug();
  // the threshold is by default d_ppnAtoms.size()*3.0, which means we restart
  // if we have learned any literals, and the number of assertions since the
  // last learned literal is equal to the total number of literals in the
  // input problem times 3, i.e. each literal has been asserted on average 3
  // times.

}

bool ZeroLevelLearner::notifyAsserted(TNode assertion, int32_t alevel)
{
  // check if at level zero
  if (d_nonZeroAssert.get())
  {
    // already not at level zero, skip
  }
  else if (alevel != 0)
  {
    Trace("level-zero-dec") << "First non-zero: " << assertion << std::endl;
    d_nonZeroAssert = true;
  }
  else if (d_levelZeroAsserts.find(assertion) == d_levelZeroAsserts.end())
  {
    // remember we've processed this
    d_levelZeroAsserts.insert(assertion);
    // process what we should do with the learned literal
    LearnedLitType ltype = computeLearnedLiteralType(assertion);
    processLearnedLiteral(assertion, ltype);
    return true;
  }
  return true;
}

LearnedLitType ZeroLevelLearner::computeLearnedLiteralType(
    const Node& input)
{
  // literal was learned, determine its type
  // compute whether internal prior to substitution
  TNode aatom = input.getKind() == Kind::NOT ? input[0] : input;
  bool internal = d_ppnAtoms.find(aatom) == d_ppnAtoms.end();
  // apply substitutions now
  Node lit = d_tsmap.apply(input, d_env.getRewriter());
  LearnedLitType ltype =
      internal ? LearnedLitType::INTERNAL : LearnedLitType::INPUT;
  // we don't try to solve for literals that simplify to constants
  if ((internal || true) && !lit.isConst())
  {
    Subs ss;
    bool processed = false;
    if (getSolved(lit, ss))
    {
      // if we solved for any variable from input, we are SOLVABLE.
      for (size_t i = 0, nvars = ss.d_vars.size(); i < nvars; i++)
      {
        Node v = ss.d_vars[i];
        if (d_ppnSyms.find(v) != d_ppnSyms.end())
        {
          Trace("level-zero-assert") << "...solvable due to " << v << std::endl;
          if (ltype == LearnedLitType::INTERNAL)
          {
            ltype = LearnedLitType::SOLVABLE;
          }
        }
        {
          bool addSubs = ss.d_subs[i].getNumChildren() == 0;
          if (addSubs)
          {
            processed = true;
            Trace("lemma-inprocess-subs")
                << "Add subs: " << v << " -> " << ss.d_subs[i] << std::endl;
            addSimplification(v, ss.d_subs[i]);
          }
        }
      }
    }
    if ((true && !processed)
        || ltype != LearnedLitType::SOLVABLE)
    {
      // maybe a constant prop?
      if (lit.getKind() == Kind::EQUAL)
      {
        for (size_t i = 0; i < 2; i++)
        {
          // Only consider substitutions whose RHS are constants.
          // A more general policy could consider lit[i].getNumChildren()==0.
          if (lit[i].isConst())
          {
            if (ltype == LearnedLitType::INTERNAL
                && d_ppnTerms.find(lit[1 - i]) != d_ppnTerms.end())
            {
              ltype = LearnedLitType::CONSTANT_PROP;
            }
            if (true && !processed)
            {
              Trace("lemma-inprocess-subs")
                  << "Add cp: " << lit[1 - i] << " -> " << lit[i] << std::endl;
              addSimplification(lit[1 - i], lit[i]);
              processed = true;
            }
            break;
          }
          else if ((true && !processed)
                   && expr::hasSubterm(lit[1 - i], lit[i]))
          {
            Trace("lemma-inprocess-subs") << "Add cp subterm: " << lit[1 - i]
                                          << " -> " << lit[i] << std::endl;
            addSimplification(lit[1 - i], lit[i]);
            processed = true;
            break;
          }
        }
      }
      if (!processed)
      {
        Trace("lemma-inprocess-subs-n")
            << "Unused unit learned: " << lit << std::endl;
      }
    }
  }
  Trace("level-zero-assert")
      << "Level zero assert: " << lit << ", type=" << ltype << std::endl;
  return ltype;
}

void ZeroLevelLearner::addSimplification(const Node& t, const Node& s)
{
  // in rare cases we may already have a substitution for v, e.g.
  // if x -> 0, (f y) ---> a, and we learn (f (+ x y)) = b, we
  // would substitute+rewrite to get (f y) --> b despite already
  // having a substitution for (f y). We could avoid this by applying
  // substitution+rewriting until fixed point at the beginning of
  // computeLearnedLiteralType, but this may be expensive.
  if (!d_tsmap.get().hasSubstitution(t))
  {
    d_tsmap.addSubstitution(t, s);
  }
}

void ZeroLevelLearner::processLearnedLiteral(const Node& lit,
                                             LearnedLitType ltype)
{
  // add to the database
  d_ldb.addLearnedLiteral(lit, ltype);
  // print to stream
  if (isOutputOn(OutputTag::LEARNED_LITS))
  {
    // get the original form so that internally generated variables
    // are mapped back to their original form
    output(OutputTag::LEARNED_LITS)
        << "(learned-lit " << SkolemManager::getOriginalForm(lit);
    std::stringstream tss;
    tss << ltype;
    std::string ltstr = tss.str();
    std::transform(
        ltstr.begin(), ltstr.end(), ltstr.begin(), [](unsigned char c) {
          return std::tolower(c);
        });
    output(OutputTag::LEARNED_LITS) << " :" << ltstr;
    output(OutputTag::LEARNED_LITS) << ")" << std::endl;
  }
}

bool ZeroLevelLearner::getSolved(const Node& lit, Subs& subs)
{
  context::Context dummyContext;
  theory::TrustSubstitutionMap subsOut(d_env, &dummyContext);
  TrustNode tlit = TrustNode::mkTrustLemma(lit);
  bool status = d_theoryEngine->solve(tlit, subsOut);
  if (status)
  {
    Trace("level-zero-debug") << lit << " is solvable" << std::endl;
    // extract the substitution
    std::unordered_map<Node, Node> ss = subsOut.get().getSubstitutions();
    for (const std::pair<const Node, Node>& s : ss)
    {
      subs.add(s.first, s.second);
      Trace("level-zero-debug")
          << "  subs: " << s.first << " -> " << s.second << std::endl;
    }
    return true;
  }
  Trace("level-zero-debug") << lit << " is not solvable" << std::endl;
  return false;
}

}  // namespace prop
}  // namespace ava6::internal
