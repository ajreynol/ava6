/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Simplifications for ITE expressions.
 *
 * This module implements preprocessing phases designed to simplify ITE
 * expressions.  Based on:
 * Kim, Somenzi, Jin.  Efficient Term-ITE Conversion for SMT.  FMCAD 2009.
 * Burch, Jerry.  Techniques for Verifying Superscalar Microprocessors.  DAC
 *'96
 */

#include "ava6_private.h"

#ifndef AVA6__ITE_UTILITIES_H
#define AVA6__ITE_UTILITIES_H

#include <unordered_map>
#include <vector>

#include "expr/node.h"
#include "smt/env_obj.h"
#include "util/hash.h"
#include "util/statistics_stats.h"

namespace ava6::internal {

namespace preprocessing {

class AssertionPipeline;

namespace util {

/**
 * A caching visitor that computes whether a node contains a term ite.
 */
class ContainsTermITEVisitor
{
 public:
  ContainsTermITEVisitor();
  ~ContainsTermITEVisitor();

  /** returns true if a node contains a term ite. */
  bool containsTermITE(TNode n);

  /** Garbage collects the cache. */
  void garbageCollect();

  /** returns the size of the cache. */
  size_t cache_size() const { return d_cache.size(); }

 private:
  typedef std::unordered_map<Node, bool> NodeBoolMap;
  NodeBoolMap d_cache;
};

 }  // namespace util
}  // namespace preprocessing
}  // namespace ava6::internal

#endif
