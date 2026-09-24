/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of the Op class.
 */

#include "test_api.h"

namespace ava6::internal {

namespace test {

class TestApiBlackOp : public TestApi
{
};

TEST_F(TestApiBlackOp, hash)
{
  std::hash<Op> h;
  ASSERT_EQ(h(d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {31, 1})),
            h(d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {31, 1})));
  ASSERT_NE(h(d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {31, 1})),
            h(d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {31, 2})));
  (void)std::hash<Op>{}(Op());
}

TEST_F(TestApiBlackOp, getKind)
{
  Op x = d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {31, 1});
  ASSERT_EQ(x.getKind(), Kind::BITVECTOR_EXTRACT);
}

TEST_F(TestApiBlackOp, isNull)
{
  Op x;
  ASSERT_TRUE(x.isNull());
  Op y = d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {31, 1});
  ASSERT_FALSE(y.isNull());
  ASSERT_NE(x, y);
}

TEST_F(TestApiBlackOp, opFromKind)
{
  ASSERT_NO_THROW(d_tm.mkOp(Kind::ADD));
  ASSERT_THROW(d_tm.mkOp(Kind::BITVECTOR_EXTRACT), Ava6ApiException);
}





TEST_F(TestApiBlackOp, opScopingToString)
{
  Op bitvector_repeat_ot = d_tm.mkOp(Kind::BITVECTOR_REPEAT, {5});
  std::string op_repr = bitvector_repeat_ot.toString();
  ASSERT_EQ(bitvector_repeat_ot.toString(), op_repr);
  {
    std::stringstream ss;
    ss << bitvector_repeat_ot;
    ASSERT_EQ(ss.str(), op_repr);
  }
}
}  // namespace test
}  // namespace ava6::internal
