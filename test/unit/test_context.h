/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Header for context unit tests.
 */

#ifndef AVA6__TEST__UNIT__TEST_CONTEXT_H
#define AVA6__TEST__UNIT__TEST_CONTEXT_H

#include "context/context.h"
#include "test.h"

namespace ava6::internal {
namespace test {

class TestContext : public TestInternal
{
 protected:
  void SetUp() override { d_context.reset(new ava6::context::Context()); }
  std::unique_ptr<ava6::context::Context> d_context;
};

}  // namespace test
}  // namespace ava6::internal
#endif
