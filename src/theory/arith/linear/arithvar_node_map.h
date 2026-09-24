/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * [[ Add one-line brief description here ]]
 *
 * [[ Add lengthier description here ]]
 * \todo document this file
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__ARITH__ARITHVAR_NODE_MAP_H
#define AVA6__THEORY__ARITH__ARITHVAR_NODE_MAP_H

#include "context/cdhashmap.h"
#include "context/cdlist.h"
#include "context/cdo.h"
#include "context/context.h"
#include "expr/node.h"
#include "theory/arith/linear/arithvar.h"
#include "util/dense_map.h"

namespace ava6::internal {
namespace theory {
namespace arith::linear {

// Maps from Nodes -> ArithVars, and vice versa
typedef std::unordered_map<Node, ArithVar> NodeToArithVarMap;
typedef DenseMap<Node> ArithVarToNodeMap;


}  // namespace arith::linear
}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__ARITH__ARITHVAR_NODE_MAP_H */
