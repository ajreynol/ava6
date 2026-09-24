/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Remove ITEs from the assertions.
 */

#include "ava6_private.h"

#ifndef AVA6__PREPROCESSING__PASSES__ITE_REMOVAL_H
#define AVA6__PREPROCESSING__PASSES__ITE_REMOVAL_H

#include "preprocessing/preprocessing_pass.h"

namespace ava6::internal {
namespace preprocessing {
namespace passes {

class IteRemoval : public PreprocessingPass
{
 public:
  IteRemoval(PreprocessingPassContext* preprocContext);

 protected:
  PreprocessingPassResult applyInternal(AssertionPipeline* assertions) override;
};

}  // namespace passes
}  // namespace preprocessing
}  // namespace ava6::internal

#endif  // AVA6__PREPROCESSING__PASSES__ITE_REMOVAL_H
