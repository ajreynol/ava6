/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * White box testing of ava6::prop::CnfStream.
 */

#include "base/check.h"
#include "context/context.h"
#include "prop/cnf_stream.h"
#include "prop/prop_engine.h"
#include "prop/registrar.h"
#include "prop/sat_clause_sink.h"
#include "prop/theory_proxy.h"
#include "test_smt.h"
#include "theory/arith/theory_arith.h"
#include "theory/booleans/theory_bool.h"
#include "theory/builtin/theory_builtin.h"
#include "theory/theory.h"
#include "theory/theory_engine.h"

namespace ava6::internal {

using namespace ava6::context;
using namespace prop;
using namespace smt;
using namespace theory;

namespace test {

class FakeSatSolver : public SatClauseSink
{
 public:
  SatVariable newVar(AVA6_UNUSED bool theoryAtom) override { return d_nextVar++; }
  SatVariable trueVar() override { return d_nextVar++; }
  SatVariable falseVar() override { return d_nextVar++; }
  bool addClause(AVA6_UNUSED const SatClause& c, AVA6_UNUSED bool removable) override
  {
    d_addClauseCalled = true;
    return false;
  }
  void reset() { d_addClauseCalled = false; }
  bool addClauseCalled() const { return d_addClauseCalled; }

 private:
  SatVariable d_nextVar = 0;
  bool d_addClauseCalled = false;
};

class TestPropWhiteCnfStream : public TestSmt
{
 protected:
  void SetUp() override
  {
    TestSmt::SetUp();
    d_satSolver.reset(new FakeSatSolver());
    d_cnfContext.reset(new Context());
    d_cnfRegistrar.reset(new prop::NullRegistrar);
    d_cnfStream.reset(new prop::CnfStream(d_slvEngine->getEnv(),
                                          d_satSolver.get(),
                                          d_cnfRegistrar.get(),
                                          d_cnfContext.get()));
  }

  void TearDown() override
  {
    d_cnfStream.reset(nullptr);
    d_cnfRegistrar.reset(nullptr);
    d_cnfContext.reset(nullptr);
    d_satSolver.reset(nullptr);
    TestSmt::TearDown();
  }

  /** The SAT solver proxy */
  std::unique_ptr<FakeSatSolver> d_satSolver;
  /** The CNF converter in use */
  std::unique_ptr<CnfStream> d_cnfStream;
  /** The context of the CnfStream. */
  std::unique_ptr<Context> d_cnfContext;
  /** The registrar used by the CnfStream. */
  std::unique_ptr<prop::NullRegistrar> d_cnfRegistrar;
};

/**
 * [chris 5/26/2010] In the tests below, we don't attempt to delve into the
 * deep structure of the CNF conversion. Firstly, we just want to make sure
 * that the conversion doesn't choke on any boolean Exprs. We'll also check
 * that addClause got called. We won't check that it gets called a particular
 * number of times, or with what.
 */

TEST_F(TestPropWhiteCnfStream, and)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node b = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node c = d_nodeManager->mkVar(d_nodeManager->booleanType());
  d_cnfStream->convertAndAssert(
      d_nodeManager->mkNode(Kind::AND, a, b, c), false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, complex_expr)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node b = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node c = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node d = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node e = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node f = d_nodeManager->mkVar(d_nodeManager->booleanType());
  d_cnfStream->convertAndAssert(
      d_nodeManager->mkNode(
          Kind::IMPLIES,
          d_nodeManager->mkNode(Kind::AND, a, b),
          d_nodeManager->mkNode(
              Kind::EQUAL,
              d_nodeManager->mkNode(Kind::OR, c, d),
              d_nodeManager->mkNode(Kind::NOT,
                                    d_nodeManager->mkNode(Kind::XOR, e, f)))),
      false,
      false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, true)
{
  d_cnfStream->convertAndAssert(d_nodeManager->mkConst(true), false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, false)
{
  d_cnfStream->convertAndAssert(d_nodeManager->mkConst(false), false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, iff)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node b = d_nodeManager->mkVar(d_nodeManager->booleanType());
  d_cnfStream->convertAndAssert(
      d_nodeManager->mkNode(Kind::EQUAL, a, b), false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, implies)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node b = d_nodeManager->mkVar(d_nodeManager->booleanType());
  d_cnfStream->convertAndAssert(
      d_nodeManager->mkNode(Kind::IMPLIES, a, b), false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, not)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  d_cnfStream->convertAndAssert(
      d_nodeManager->mkNode(Kind::NOT, a), false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, or)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node b = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node c = d_nodeManager->mkVar(d_nodeManager->booleanType());
  d_cnfStream->convertAndAssert(
      d_nodeManager->mkNode(Kind::OR, a, b, c), false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, var)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node b = d_nodeManager->mkVar(d_nodeManager->booleanType());
  d_cnfStream->convertAndAssert(a, false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
  d_satSolver->reset();
  d_cnfStream->convertAndAssert(b, false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, xor)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node b = d_nodeManager->mkVar(d_nodeManager->booleanType());
  d_cnfStream->convertAndAssert(
      d_nodeManager->mkNode(Kind::XOR, a, b), false, false);
  ASSERT_TRUE(d_satSolver->addClauseCalled());
}

TEST_F(TestPropWhiteCnfStream, ensure_literal)
{
  Node a = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node b = d_nodeManager->mkVar(d_nodeManager->booleanType());
  Node a_and_b = d_nodeManager->mkNode(Kind::AND, a, b);
  d_cnfStream->ensureLiteral(a_and_b);
  // Clauses are necessary to "literal-ize" a_and_b
  ASSERT_TRUE(d_satSolver->addClauseCalled());
  ASSERT_TRUE(d_cnfStream->hasLiteral(a_and_b));
}
}  // namespace test
}  // namespace ava6::internal
