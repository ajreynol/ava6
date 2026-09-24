/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The IntToBV preprocessing pass.
 *
 * Converts integer operations into bitvector operations. The width of the
 * bitvectors is controlled through the `--solve-int-as-bv` command line
 * option.
 */

#include "ava6_private.h"

#ifndef AVA6__PREPROCESSING__PASSES__INT_TO_BV_H
#define AVA6__PREPROCESSING__PASSES__INT_TO_BV_H

#include "expr/node.h"
#include "preprocessing/preprocessing_pass.h"

namespace ava6::internal {
namespace preprocessing {
namespace passes {

using NodeMap = std::unordered_map<Node, Node>;

class IntToBV : public PreprocessingPass
{
 public:
  IntToBV(PreprocessingPassContext* preprocContext);
  Node intToBV(TNode n, NodeMap& cache);

 protected:
  PreprocessingPassResult applyInternal(
      AssertionPipeline* assertionsToPreprocess) override;
};

}  // namespace passes
}  // namespace preprocessing
}  // namespace ava6::internal

#endif /* AVA6__PREPROCESSING__PASSES__INT_TO_BV_H */
