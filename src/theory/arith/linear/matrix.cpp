/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Sparse matrix implementations for different types.
 */

#include "theory/arith/linear/matrix.h"

using namespace std;
namespace ava6::internal {
namespace theory {
namespace arith::linear {

void NoEffectCCCB::update(AVA6_UNUSED RowIndex ridx,
                          AVA6_UNUSED ArithVar nb,
                          AVA6_UNUSED int oldSgn,
                          AVA6_UNUSED int currSgn)
{
}
void NoEffectCCCB::multiplyRow(AVA6_UNUSED RowIndex ridx, AVA6_UNUSED int sgn)
{
}
bool NoEffectCCCB::canUseRow(AVA6_UNUSED RowIndex ridx) const { return false; }

}  // namespace arith::linear
}  // namespace theory
}  // namespace ava6::internal
