/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of the SkolemId enum of the C++ API.
 */

#include <ava6/ava6_skolem_id.h>

#include <algorithm>

#include "base/output.h"
#include "gtest/gtest.h"

namespace ava6::internal {

namespace test {

class TestApiBlackSkolemId : public ::testing::Test
{
};

TEST_F(TestApiBlackSkolemId, skolemIdToString)
{
  for (int32_t i = static_cast<int32_t>(SkolemId::INTERNAL);
       i <= static_cast<int32_t>(SkolemId::NONE);
       ++i)
  {
    auto rulestr = std::to_string(static_cast<SkolemId>(i));
    // If this assertion fails, the switch in enum_to_string.cpp is missing
    // id i.
    ASSERT_NE(rulestr, "?");
  }
}

TEST_F(TestApiBlackSkolemId, skolemIdHash)
{
  ASSERT_EQ(std::hash<ava6::SkolemId>()(SkolemId::PURIFY),
            static_cast<size_t>(SkolemId::PURIFY));
  ASSERT_NE(std::hash<ava6::SkolemId>()(SkolemId::INTERNAL),
            static_cast<size_t>(SkolemId::PURIFY));
}

}  // namespace test
}  // namespace ava6::internal
