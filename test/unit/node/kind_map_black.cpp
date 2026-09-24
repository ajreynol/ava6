/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of ava6::KindMap.
 */

#include <iostream>
#include <sstream>
#include <string>

#include "expr/kind_map.h"
#include "test.h"

namespace ava6::internal {

using namespace internal;
using namespace internal::kind;

namespace test {

class TestNodeBlackKindMap : public TestInternal
{
};

TEST_F(TestNodeBlackKindMap, simple)
{
  KindMap map;
  ASSERT_FALSE(map.test(Kind::AND));
  map.set(Kind::AND);
  ASSERT_TRUE(map.test(Kind::AND));
  map.reset(Kind::AND);
  ASSERT_FALSE(map.test(Kind::AND));
#ifdef AVA6_ASSERTIONS
  ASSERT_THROW(map.set(Kind::LAST_KIND), AssertArgumentException);
#endif
}

}  // namespace test
}  // namespace ava6::internal
