/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The TheoryPreprocess preprocessing pass.
 *
 * Calls Theory::preprocess(...) on every assertion of the formula.
 */

#include "ava6_private.h"

#ifndef AVA6__PREPROCESSING__PASSES__THEORY_PREPROCESS_H
#define AVA6__PREPROCESSING__PASSES__THEORY_PREPROCESS_H

#include "preprocessing/preprocessing_pass.h"

namespace ava6::internal {
namespace preprocessing {
namespace passes {

class TheoryPreprocess : public PreprocessingPass
{
 public:
  TheoryPreprocess(PreprocessingPassContext* preprocContext);

 protected:
  PreprocessingPassResult applyInternal(
      AssertionPipeline* assertionsToPreprocess) override;
};

}  // namespace passes
}  // namespace preprocessing
}  // namespace ava6::internal

#endif /* AVA6__PREPROCESSING__PASSES__THEORY_PREPROCESS_H */
