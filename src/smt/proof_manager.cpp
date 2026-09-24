/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The proof manager of the SMT engine.
 */

#include "smt/proof_manager.h"

#include "expr/subtype_elim_node_converter.h"
#include "options/base_options.h"
#include "options/main_options.h"
#include "options/smt_options.h"
#include "proof/eo/eo_printer.h"
#include "proof/proof_checker.h"
#include "proof/proof_node_algorithm.h"
#include "proof/proof_node_manager.h"
#include "rewriter/rewrite_db.h"
#include "smt/assertions.h"
#include "smt/env.h"
#include "smt/preprocess_proof_generator.h"
#include "smt/proof_post_processor.h"
#include "smt/smt_solver.h"

using namespace ava6::internal::rewriter;
namespace ava6::internal {
namespace smt {

PfManager::PfManager(Env& env)
    : EnvObj(env),
      d_rewriteDb(nullptr),
      d_pchecker(nullptr),
      d_pnm(nullptr),
      d_pfpp(nullptr),
      d_pppg(nullptr),
      d_finalCb(env),
      d_finalizer(env, d_finalCb)
{
  // construct the rewrite db only if DSL rewrites are enabled
  if (options().proof.proofGranularityMode
          == options::ProofGranularityMode::DSL_REWRITE
      || options().proof.proofGranularityMode
             == options::ProofGranularityMode::DSL_REWRITE_STRICT)
  {
    d_rewriteDb.reset(new RewriteDb(nodeManager()));
    // maybe output rare rules?
    bool isNormalOut = isOutputOn(OutputTag::RARE_DB);
    if (isNormalOut)
    {

      proof::EoNodeConverter atp(nodeManager());
      proof::EoPrinter eop(d_env, atp, d_rewriteDb.get());
      const std::map<ProofRewriteRule, RewriteProofRule>& rules =
          d_rewriteDb->getAllRules();
      for (const std::pair<const ProofRewriteRule, RewriteProofRule>& r : rules)
      {
        // only output if the signature level is what we want
        Level l = r.second.getSignatureLevel();
        if (l == Level::NORMAL && isNormalOut)
        {
          std::ostream& os = output(OutputTag::RARE_DB);
          eop.printDslRule(os, r.first);
        }

      }
    }
  }

  // enable the proof checker and the proof node manager
  d_pchecker.reset(
      new ProofChecker(statisticsRegistry(),
                       options().proof.proofCheck,
                       static_cast<uint32_t>(options().proof.proofPedantic),
                       d_rewriteDb.get()));
  d_pnm.reset(new ProofNodeManager(env.getNodeManager(),
                                   env.getOptions(),
                                   env.getRewriter(),
                                   d_pchecker.get()));
  // Now, initialize the proof postprocessor with the environment.
  // By default the post-processor will update all assumptions, which
  // can lead to SCOPE subproofs of the form
  //   A
  //  ...
  //   B1    B2
  //  ...   ...
  // ------------
  //      C
  // ------------- SCOPE [B1, B2]
  // B1 ^ B2 => C
  //
  // where A is an available assumption from outside the scope (note
  // that B1 was an assumption of this SCOPE subproof but since it could
  // be inferred from A, it was updated). This shape is problematic for
  // the Alethe reconstruction, so we disable the update of scoped
  // assumptions (which would disable the update of B1 in this case).
  d_pfpp = std::make_unique<ProofPostprocess>(
      env,
      d_rewriteDb.get(),
      true);

  // add rules to eliminate here
  if (options().proof.proofGranularityMode
      != options::ProofGranularityMode::MACRO)
  {
    d_pfpp->setEliminateRule(ProofRule::MACRO_SR_EQ_INTRO);
    d_pfpp->setEliminateRule(ProofRule::MACRO_SR_PRED_INTRO);
    d_pfpp->setEliminateRule(ProofRule::MACRO_SR_PRED_ELIM);
    d_pfpp->setEliminateRule(ProofRule::MACRO_SR_PRED_TRANSFORM);
    // Alethe does not require chain multiset resolution to be expanded.
    if (true
        && !options().proof.proofChainMRes)
    {
      d_pfpp->setEliminateRule(ProofRule::CHAIN_M_RESOLUTION);
    }
    // The Alethe translation handles this macro directly via la_generic
    {
      d_pfpp->setEliminateRule(ProofRule::MACRO_ARITH_SCALE_SUM_UB);
    }
    if (options().proof.proofGranularityMode
        != options::ProofGranularityMode::REWRITE)
    {
      d_pfpp->setEliminateRule(ProofRule::SUBS);
      d_pfpp->setEliminateRule(ProofRule::MACRO_REWRITE);
      // if in a DSL rewrite mode
      if (options().proof.proofGranularityMode
          != options::ProofGranularityMode::THEORY_REWRITE)
      {
        // this eliminates theory rewriting steps with finer-grained DSL rules
        d_pfpp->setEliminateAllTrustedRules();
      }
    }
    // theory-specific lazy proof reconstruction
    d_pfpp->setEliminateRule(ProofRule::MACRO_STRING_INFERENCE);
    d_pfpp->setEliminateRule(ProofRule::MACRO_BV_BITBLAST);
    // we only try to eliminate TRUST if not macro level
    d_pfpp->setEliminateRule(ProofRule::TRUST);
  }
  d_false = nodeManager()->mkConst(false);

  d_pppg = std::make_unique<PreprocessProofGenerator>(
      d_env, userContext(), "smt::PreprocessProofGenerator");
}

PfManager::~PfManager() {}

// TODO: Remove in favor of `std::erase_if` with C++ 20+ (see ava6-wishues#137).
template <class T, class Alloc, class Pred>
constexpr typename std::vector<T, Alloc>::size_type erase_if(
    std::vector<T, Alloc>& c, Pred pred)
{
  typename std::vector<T, Alloc>::iterator it =
      std::remove_if(c.begin(), c.end(), pred);
  typename std::vector<T, Alloc>::size_type r = std::distance(it, c.end());
  c.erase(it, c.end());
  return r;
}

std::shared_ptr<ProofNode> PfManager::connectProofToAssertions(
    std::shared_ptr<ProofNode> pfn, Assertions& as, ProofScopeMode scopeMode)
{
  // Note this assumes that connectProofToAssertions is only called once per
  // unsat response. This method would need to cache its result otherwise.
  Trace("smt-proof")
      << "SolverEngine::connectProofToAssertions(): get proof body...\n";

  if (TraceIsOn("smt-proof-debug"))
  {
    Trace("smt-proof-debug")
        << "SolverEngine::connectProofToAssertions(): Proof node for false:\n";
    Trace("smt-proof-debug") << *pfn.get() << std::endl;
    Trace("smt-proof-debug") << "=====" << std::endl;
  }
  std::vector<Node> assertions;
  getAssertions(as, assertions);

  if (TraceIsOn("smt-proof"))
  {
    Trace("smt-proof")
        << "SolverEngine::connectProofToAssertions(): get free assumptions..."
        << std::endl;
    std::vector<Node> fassumps;
    expr::getFreeAssumptions(pfn.get(), fassumps);
    Trace("smt-proof") << "SolverEngine::connectProofToAssertions(): initial "
                          "free assumptions are:\n";
    for (const Node& a : fassumps)
    {
      Trace("smt-proof") << "- " << a << std::endl;
    }

    Trace("smt-proof")
        << "SolverEngine::connectProofToAssertions(): assertions are:\n";
    for (const Node& n : assertions)
    {
      Trace("smt-proof") << "- " << n << std::endl;
    }
    Trace("smt-proof") << "=====" << std::endl;
  }

  Trace("smt-proof")
      << "SolverEngine::connectProofToAssertions(): postprocess...\n";
  Assert(d_pfpp != nullptr);
  // Note that in incremental mode, we cannot set assertions here, as it
  // permits the postprocessor to merge subproofs at a higher user context
  // level into proofs that are used in a lower user context level.
  if (!options().base.incrementalSolving)
  {
    d_pfpp->setAssertions(assertions, false);
  }
  d_pfpp->process(pfn, d_pppg.get());

  switch (scopeMode)
  {
    case ProofScopeMode::NONE:
    {
      return pfn;
    }
    // Now make the final scope(s), which ensure(s) that the only open leaves
    // of the proof are the assertions (and definitions). If we are pruning
    // the input, we will try to minimize the used assertions (and definitions).
    case ProofScopeMode::UNIFIED:
    {
      Trace("smt-proof") << "SolverEngine::connectProofToAssertions(): make "
                            "unified scope...\n";
      return d_pnm->mkScope(
          pfn, assertions, true, false);
    }
    case ProofScopeMode::DEFINITIONS_AND_ASSERTIONS:
    {
      Trace("smt-proof")
          << "SolverEngine::connectProofToAssertions(): make split scope...\n";
      // To support proof pruning for nested scopes, we need to:
      // 1. Minimize assertions of closed unified scope.
      std::vector<Node> unifiedAssertions;
      getAssertions(as, unifiedAssertions);
      Pf pf = d_pnm->mkScope(
          pfn, unifiedAssertions, true, false);
      // if this is violated, there is unsoundness since we have shown
      // false that does not depend on the input.
      AlwaysAssert(pf->getRule() == ProofRule::SCOPE);
      // 2. Extract minimum unified assertions from the scope node.
      std::unordered_set<Node> minUnifiedAssertions;
      minUnifiedAssertions.insert(pf->getArguments().cbegin(),
                                  pf->getArguments().cend());
      // 3. Split those assertions into minimized definitions and assertions.
      std::vector<Node> minDefinitions;
      std::vector<Node> minAssertions;
      getDefinitionsAndAssertions(as, minDefinitions, minAssertions);
      std::function<bool(Node)> predicate = [&minUnifiedAssertions](Node n) {
        return minUnifiedAssertions.find(n) == minUnifiedAssertions.cend();
      };
      erase_if(minDefinitions, predicate);
      erase_if(minAssertions, predicate);
      // 4. Extract proof from unified scope and encapsulate it with split
      // scopes introducing minimized definitions and assertions.
      return d_pnm->mkNode(
          ProofRule::SCOPE,
          {d_pnm->mkNode(ProofRule::SCOPE, pf->getChildren(), minAssertions)},
          minDefinitions);
    }
    default: Unreachable();
  }
}

void PfManager::checkFinalProof(std::shared_ptr<ProofNode> pfn)
{
  // take stats and check pedantic
  d_finalCb.initializeUpdate();
  d_finalizer.process(pfn);

  std::stringstream serr;
  bool wasPedanticFailure = d_finalCb.wasPedanticFailure(serr);
  if (wasPedanticFailure)
  {
    AlwaysAssert(!wasPedanticFailure)
        << "ProofPostprocess::process: pedantic failure:" << std::endl
        << serr.str();
  }
}

void PfManager::printProof(std::ostream& out,
                           std::shared_ptr<ProofNode> fp,
                           ProofScopeMode scopeMode)
{
  proof::EoNodeConverter atp(nodeManager());
  proof::EoPrinter eop(d_env, atp, d_rewriteDb.get());
  eop.print(out, fp, scopeMode);
}

ProofChecker* PfManager::getProofChecker() const { return d_pchecker.get(); }

ProofNodeManager* PfManager::getProofNodeManager() const { return d_pnm.get(); }

rewriter::RewriteDb* PfManager::getRewriteDatabase() const
{
  return d_rewriteDb.get();
}

PreprocessProofGenerator* PfManager::getPreprocessProofGenerator() const
{
  return d_pppg.get();
}

void PfManager::getAssertions(Assertions& as, std::vector<Node>& assertions)
{
  // note that the assertion list is always available
  const context::CDList<Node>& al = as.getAssertionList();
  for (const Node& a : al)
  {
    assertions.push_back(a);
  }
}

void PfManager::getDefinitionsAndAssertions(Assertions& as,
                                            std::vector<Node>& definitions,
                                            std::vector<Node>& assertions)
{
  const context::CDList<Node>& defs = as.getAssertionListDefinitions();
  for (const Node& d : defs)
  {
    // Keep treating (mutually) recursive functions as declarations +
    // assertions.
    if (d.getKind() == Kind::EQUAL)
    {
      definitions.push_back(d);
    }
  }
  const context::CDList<Node>& asserts = as.getAssertionList();
  for (const Node& a : asserts)
  {
    if (std::find(definitions.cbegin(), definitions.cend(), a)
        == definitions.cend())
    {
      assertions.push_back(a);
    }
  }
}

}  // namespace smt
}  // namespace ava6::internal
