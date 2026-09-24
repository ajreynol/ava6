/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The static learning preprocessing pass.
 */

#include "ava6_private.h"

#ifndef AVA6__PREPROCESSING__PASSES__STATIC_LEARNING_H
#define AVA6__PREPROCESSING__PASSES__STATIC_LEARNING_H

#include "context/cdhashset.h"
#include "preprocessing/preprocessing_pass.h"

namespace ava6::internal {
namespace preprocessing {
namespace passes {

class StaticLearning : public PreprocessingPass
{
 public:
  StaticLearning(PreprocessingPassContext* preprocContext);

 protected:
  PreprocessingPassResult applyInternal(
      AssertionPipeline* assertionsToPreprocess) override;

 private:
  /** Collect children of flattened AND term. */
  void flattenAnd(TNode node, std::vector<TNode>& children);

  /** CD-cache for visiting nodes used by `flattenAnd`. */
  context::CDHashSet<Node> d_cache;
};

}  // namespace passes
}  // namespace preprocessing
}  // namespace ava6::internal

#endif /* AVA6__PREPROCESSING__PASSES__STATIC_LEARNING_H */
