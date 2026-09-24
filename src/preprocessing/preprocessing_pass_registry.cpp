/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The preprocessing pass registry
 *
 * This file defines the classes PreprocessingPassRegistry, which keeps track
 * of the available preprocessing passes.
 */

#include "preprocessing/preprocessing_pass_registry.h"

#include <algorithm>
#include <utility>

#include "base/check.h"
#include "base/map_util.h"
#include "base/output.h"
#include "preprocessing/passes/ackermann.h"
#include "preprocessing/passes/apply_substs.h"
#include "preprocessing/passes/bool_to_bv.h"
#include "preprocessing/passes/bv_eager_atoms.h"
#include "preprocessing/passes/bv_to_bool.h"
#include "preprocessing/passes/non_clausal_simp.h"
#include "preprocessing/passes/quantifiers_preprocess.h"
#include "preprocessing/passes/real_to_int.h"
#include "preprocessing/passes/rewrite.h"
#include "preprocessing/passes/static_learning.h"
#include "preprocessing/passes/static_rewrite.h"
#include "preprocessing/passes/theory_preprocess.h"
#include "preprocessing/preprocessing_pass.h"

namespace ava6::internal {
namespace preprocessing {

using namespace ava6::internal::preprocessing::passes;

PreprocessingPassRegistry& PreprocessingPassRegistry::getInstance()
{
  static thread_local PreprocessingPassRegistry* ppReg =
      new PreprocessingPassRegistry();
  return *ppReg;
}

void PreprocessingPassRegistry::registerPassInfo(
    const std::string& name,
    std::function<PreprocessingPass*(PreprocessingPassContext*)> ctor)
{
  AlwaysAssert(!ContainsKey(d_ppInfo, name));
  d_ppInfo[name] = ctor;
}

PreprocessingPass* PreprocessingPassRegistry::createPass(
    PreprocessingPassContext* ppCtx, const std::string& name)
{
  Assert(d_ppInfo.count(name));
  return d_ppInfo[name](ppCtx);
}

std::vector<std::string> PreprocessingPassRegistry::getAvailablePasses()
{
  std::vector<std::string> passes;
  for (const auto& info : d_ppInfo)
  {
    passes.push_back(info.first);
  }
  std::sort(passes.begin(), passes.end());
  return passes;
}

bool PreprocessingPassRegistry::hasPass(const std::string& name)
{
  return d_ppInfo.find(name) != d_ppInfo.end();
}

namespace {

/**
 * This method is stored by the `PreprocessingPassRegistry` and used to create
 * a new instance of the preprocessing pass T.
 *
 * @param ppCtx The preprocessing pass context passed to the constructor of
 *              the preprocessing pass
 */
template <class T>
PreprocessingPass* callCtor(PreprocessingPassContext* ppCtx)
{
  return new T(ppCtx);
}

}  // namespace

PreprocessingPassRegistry::PreprocessingPassRegistry()
{
  registerPassInfo("apply-substs", callCtor<ApplySubsts>);

  registerPassInfo("static-learning", callCtor<StaticLearning>);
  registerPassInfo("real-to-int", callCtor<RealToInt>);
  registerPassInfo("bv-to-bool", callCtor<BVToBool>);
  registerPassInfo("rewrite", callCtor<Rewrite>);
  registerPassInfo("bv-eager-atoms", callCtor<BvEagerAtoms>);
  registerPassInfo("quantifiers-preprocess", callCtor<QuantifiersPreprocess>);
  registerPassInfo("non-clausal-simp", callCtor<NonClausalSimp>);
  registerPassInfo("ackermann", callCtor<Ackermann>);
  registerPassInfo("theory-preprocess", callCtor<TheoryPreprocess>);
  registerPassInfo("bool-to-bv", callCtor<BoolToBV>);
  registerPassInfo("static-rewrite", callCtor<StaticRewrite>);
}

}  // namespace preprocessing
}  // namespace ava6::internal
