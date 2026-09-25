/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * White box testing of the core rewriter.
 */

#include "proof/proof_node.h"
#include "test_smt.h"
#include "util/rational.h"

namespace ava6::internal {
namespace test {

using namespace theory;

class TestTheoryWhiteRewriter : public TestSmt
{
};

class TestTheoryWhiteRewriterProof : public TestSmtNoFinishInit
{
 protected:
  void SetUp() override
  {
    TestSmtNoFinishInit::SetUp();
    d_slvEngine->setOption("produce-proofs", "true");
    d_slvEngine->setOption("proof-check", "eager");
    d_slvEngine->finishInit();
  }
};

TEST_F(TestTheoryWhiteRewriterProof, sharedChildProofCache)
{
  Rewriter* rr = d_slvEngine->getEnv().getRewriter();
  TypeNode intType = d_nodeManager->integerType();
  TypeNode funType = d_nodeManager->mkFunctionType(intType, intType);
  Node x = d_skolemManager->mkDummySkolem("x", intType);
  Node f = d_skolemManager->mkDummySkolem("f", funType);
  Node g = d_skolemManager->mkDummySkolem("g", funType);
  Node zero = d_nodeManager->mkConstInt(Rational(0));
  Node child = d_nodeManager->mkNode(Kind::ADD, x, zero);

  // A shared child's ordinary rewrite cache must not suppress its proof.
  // The second parent can then reuse the child's completed proof.
  ASSERT_EQ(rr->rewrite(child), x);
  for (const Node& fun : {f, g})
  {
    Node input = d_nodeManager->mkNode(Kind::APPLY_UF, fun, child);
    Node expected = d_nodeManager->mkNode(Kind::APPLY_UF, fun, x);
    ASSERT_EQ(rr->rewrite(input), expected);
    TrustNode rewritten = rr->rewriteWithProof(input);
    ASSERT_EQ(rewritten.getNode(), expected);
    std::shared_ptr<ProofNode> proof = rewritten.toProofNode();
    ASSERT_NE(proof, nullptr);
    ASSERT_EQ(proof->getResult(), input.eqNode(expected));
    ASSERT_TRUE(proof->isClosed());
  }
}

TEST_F(TestTheoryWhiteRewriter, deepFullRewrite)
{
  Rewriter* rr = d_slvEngine->getEnv().getRewriter();
  TypeNode intType = d_nodeManager->integerType();
  TypeNode setType = d_nodeManager->mkSetType(intType);
  Node x = d_skolemManager->mkDummySkolem("x", intType);
  Node tail = d_skolemManager->mkDummySkolem("tail", setType);
  Node set = tail;

  constexpr size_t kDepth = 4000;
  for (size_t i = 0; i < kDepth; ++i)
  {
    Node elem = d_nodeManager->mkConstInt(Rational(i));
    Node singleton = d_nodeManager->mkNode(Kind::SET_SINGLETON, elem);
    set = d_nodeManager->mkNode(Kind::SET_UNION, singleton, set);
  }

  Node mem = d_nodeManager->mkNode(Kind::SET_MEMBER, x, set);
  Node rewritten = rr->rewrite(mem);

  ASSERT_EQ(rewritten.getKind(), Kind::OR);
  ASSERT_EQ(rewritten.getNumChildren(), kDepth + 1);

  Node memberTail = d_nodeManager->mkNode(Kind::SET_MEMBER, x, tail);
  bool foundTail = false;
  for (const Node& c : rewritten)
  {
    if (c == memberTail)
    {
      foundTail = true;
      break;
    }
  }
  ASSERT_TRUE(foundTail);
}

}  // namespace test
}  // namespace ava6::internal
