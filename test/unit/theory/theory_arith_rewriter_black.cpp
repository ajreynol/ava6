/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of rewriter for arithmetic.
 */

#include "test_smt.h"
#include "theory/arith/rewriter/rewrite_atom.h"
#include "util/rational.h"

namespace ava6::internal {

using namespace kind;
using namespace context;
using namespace theory;

namespace test {

class TestTheoryArithRewriterBlack : public TestSmt
{
};

TEST_F(TestTheoryArithRewriterBlack, Equality)
{
  auto* rr = d_slvEngine->getEnv().getRewriter();
  Node x = d_skolemManager->mkDummySkolem("x", d_nodeManager->integerType());
  Node one = d_nodeManager->mkConstInt(Rational(1));
  Node two = d_nodeManager->mkConstInt(Rational(2));

  // Rewriting an equality only rewrites its sides and orients them. In
  // particular, it does not solve this equality for x by polynomial
  // normalization.
  Node lhs = rr->rewrite(d_nodeManager->mkNode(Kind::ADD, x, one));
  Node eq = lhs.eqNode(two);
  EXPECT_EQ(rr->rewrite(eq), eq);
  // The equality is oriented in the same direction as its normal form, which
  // is (= x 1) here. Hence the reverse of the above equality is oriented to
  // the above equality.
  EXPECT_EQ(rr->rewrite(two.eqNode(lhs)), eq);
  // Rewriting is idempotent, i.e. the normal form of an equality is itself in
  // rewritten form.
  Node norm = x.eqNode(one);
  EXPECT_EQ(rr->rewrite(norm), norm);

  // Equalities that are constant after normalization are still folded.
  EXPECT_EQ(rr->rewrite(x.eqNode(x)), d_nodeManager->mkConst(true));
  EXPECT_EQ(rr->rewrite(one.eqNode(two)), d_nodeManager->mkConst(false));
  Node twiceX = d_nodeManager->mkNode(Kind::MULT, two, x);
  EXPECT_EQ(rr->rewrite(twiceX.eqNode(one)), d_nodeManager->mkConst(false));

  // The normal form of an equality between real terms is an equality between
  // real terms as well, e.g. (= (to_real x) 0.0) is in normal form.
  Node rx = d_nodeManager->mkNode(Kind::TO_REAL, x);
  Node zeroReal = d_nodeManager->mkConstReal(Rational(0));
  Node oneReal = d_nodeManager->mkConstReal(Rational(1));
  Node eqReal = rx.eqNode(zeroReal);
  EXPECT_EQ(rr->rewrite(eqReal), eqReal);
  EXPECT_EQ(rr->rewrite(zeroReal.eqNode(rx)), eqReal);
  EXPECT_EQ(arith::rewriter::normalizeEquality(d_nodeManager.get(), eqReal),
            eqReal);
  // (= (+ (to_real x) 1.0) 1.0) is not normalized by the rewriter, but its
  // normal form is the equality between real terms above.
  Node sumReal = rr->rewrite(d_nodeManager->mkNode(Kind::ADD, rx, oneReal))
                     .eqNode(oneReal);
  EXPECT_EQ(rr->rewrite(sumReal), sumReal);
  EXPECT_EQ(arith::rewriter::normalizeEquality(d_nodeManager.get(), sumReal),
            eqReal);

  // Normalizing an equality whose sides are not in rewritten form does not
  // fail, e.g. when the sides of the equality cancel.
  Node y = d_skolemManager->mkDummySkolem("y", d_nodeManager->integerType());
  Node xy = d_nodeManager->mkNode(Kind::ADD, x, y);
  Node yx = d_nodeManager->mkNode(Kind::ADD, y, x);
  EXPECT_EQ(
      arith::rewriter::normalizeEquality(d_nodeManager.get(), xy.eqNode(yx)),
      d_nodeManager->mkConst(true));
}

TEST_F(TestTheoryArithRewriterBlack, Abs)
{
  {
    Node a = d_nodeManager->mkConstReal(10);
    Node b = d_nodeManager->mkConstReal(-10);
    Node m = d_nodeManager->mkNode(Kind::ABS, a);
    Node n = d_nodeManager->mkNode(Kind::ABS, b);
    m = d_slvEngine->getEnv().getRewriter()->rewrite(m);
    n = d_slvEngine->getEnv().getRewriter()->rewrite(n);
    EXPECT_EQ(m, a);
    EXPECT_EQ(n, a);
  }
  {
    Node a = d_nodeManager->mkConstReal(Rational(3, 2));
    Node b = d_nodeManager->mkConstReal(Rational(-3, 2));
    Node m = d_nodeManager->mkNode(Kind::ABS, a);
    Node n = d_nodeManager->mkNode(Kind::ABS, b);
    m = d_slvEngine->getEnv().getRewriter()->rewrite(m);
    n = d_slvEngine->getEnv().getRewriter()->rewrite(n);
    EXPECT_EQ(m, a);
    EXPECT_EQ(n, a);
  }

}

}  // namespace test
}  // namespace ava6::internal
