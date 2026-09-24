/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The strings eager preprocess utility.
 */

#include "ava6_private.h"

#ifndef AVA6__PREPROCESSING__PASSES__STRINGS_EAGER_PP_H
#define AVA6__PREPROCESSING__PASSES__STRINGS_EAGER_PP_H

#include "preprocessing/preprocessing_pass.h"

namespace ava6::internal {
namespace preprocessing {
namespace passes {

/**
 * Eliminate all extended string functions in the input problem using
 * reductions to bounded string quantifiers.
 */
class StringsEagerPp : public PreprocessingPass
{
 public:
  StringsEagerPp(PreprocessingPassContext* preprocContext);

 protected:
  PreprocessingPassResult applyInternal(
      AssertionPipeline* assertionsToPreprocess) override;
};

}  // namespace passes
}  // namespace preprocessing
}  // namespace ava6::internal

#endif /* AVA6__PREPROCESSING__PASSES__STRINGS_EAGER_PP_H */
