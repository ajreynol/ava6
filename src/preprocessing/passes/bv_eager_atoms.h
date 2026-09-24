/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Wrap assertions in BITVECTOR_EAGER_ATOM nodes.
 *
 * This preprocessing pass wraps all assertions in BITVECTOR_EAGER_ATOM nodes
 * and allows to use eager bit-blasting in the BV solver.
 */

#include "ava6_private.h"

#ifndef AVA6__PREPROCESSING__PASSES__BV_EAGER_ATOMS_H
#define AVA6__PREPROCESSING__PASSES__BV_EAGER_ATOMS_H

#include "preprocessing/preprocessing_pass.h"

namespace ava6::internal {
namespace preprocessing {
namespace passes {

class BVEagerAtomProofGenerator;

class BvEagerAtoms : public PreprocessingPass
{
 public:
  BvEagerAtoms(PreprocessingPassContext* preprocContext);

 protected:
  PreprocessingPassResult applyInternal(
      AssertionPipeline* assertionsToPreprocess) override;
  /** A proof generator */
  std::shared_ptr<BVEagerAtomProofGenerator> d_proof;
};

}  // namespace passes
}  // namespace preprocessing
}  // namespace ava6::internal

#endif /* AVA6__PREPROCESSING__PASSES__BV_EAGER_ATOMS_H */
