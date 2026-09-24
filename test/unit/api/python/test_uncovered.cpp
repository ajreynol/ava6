/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Testing functions that are not exposed by the Python API for code coverage.
 */

#include <ava6/ava6_parser.h>

#include "test_api.h"

namespace ava6::internal {

namespace test {

class TestApiBlackUncovered : public TestApi
{
};



TEST_F(TestApiBlackUncovered, exception_getmessage)
{
  d_solver->setOption("produce-models", "true");
  Term x = d_tm.mkConst(d_tm.getBooleanSort(), "x");
  d_solver->assertFormula(x.eqTerm(x).notTerm());

  ASSERT_THROW(d_solver->getValue(x), Ava6ApiRecoverableException);

  try
  {
    d_solver->getValue(x);
  }
  catch (const Ava6ApiRecoverableException& e)
  {
    ASSERT_NO_THROW(e.getMessage());
  }
}

TEST_F(TestApiBlackUncovered, equalHash)
{
  DatatypeDecl decl1 = d_tm.mkDatatypeDecl("list");
  DatatypeConstructorDecl cons1 = d_tm.mkDatatypeConstructorDecl("cons");
  cons1.addSelector("head", d_tm.getIntegerSort());
  decl1.addConstructor(cons1);
  DatatypeConstructorDecl nil1 = d_tm.mkDatatypeConstructorDecl("nil");
  decl1.addConstructor(nil1);
  Sort list1 = d_tm.mkDatatypeSort(decl1);
  Datatype dt1 = list1.getDatatype();
  DatatypeConstructor consConstr1 = dt1[0];
  DatatypeConstructor nilConstr1 = dt1[1];
  DatatypeSelector head1 = consConstr1.getSelector("head");

  DatatypeDecl decl2 = d_tm.mkDatatypeDecl("list");
  DatatypeConstructorDecl cons2 = d_tm.mkDatatypeConstructorDecl("cons");
  cons2.addSelector("head", d_tm.getIntegerSort());
  decl2.addConstructor(cons2);
  DatatypeConstructorDecl nil2 = d_tm.mkDatatypeConstructorDecl("nil");
  decl2.addConstructor(nil2);
  Sort list2 = d_tm.mkDatatypeSort(decl2);
  Datatype dt2 = list2.getDatatype();
  DatatypeConstructor consConstr2 = dt2[0];
  DatatypeConstructor nilConstr2 = dt2[1];
  DatatypeSelector head2 = consConstr2.getSelector("head");

  ASSERT_EQ(decl1, decl1);
  ASSERT_FALSE(decl1 == decl2);
  ASSERT_EQ(cons1, cons1);
  ASSERT_FALSE(cons1 == cons2);
  ASSERT_EQ(nil1, nil1);
  ASSERT_FALSE(nil1 == nil2);
  ASSERT_EQ(consConstr1, consConstr1);
  ASSERT_FALSE(consConstr1 == consConstr2);
  ASSERT_EQ(head1, head1);
  ASSERT_FALSE(head1 == head2);
  ASSERT_EQ(dt1, dt1);
  ASSERT_FALSE(dt1 == dt2);

  ASSERT_EQ(std::hash<DatatypeDecl>{}(decl1), std::hash<DatatypeDecl>{}(decl1));
  ASSERT_EQ(std::hash<DatatypeDecl>{}(decl1), std::hash<DatatypeDecl>{}(decl2));
  ASSERT_EQ(std::hash<DatatypeConstructorDecl>{}(cons1),
            std::hash<DatatypeConstructorDecl>{}(cons1));
  ASSERT_EQ(std::hash<DatatypeConstructorDecl>{}(cons1),
            std::hash<DatatypeConstructorDecl>{}(cons2));
  ASSERT_EQ(std::hash<DatatypeConstructorDecl>{}(nil1),
            std::hash<DatatypeConstructorDecl>{}(nil1));
  ASSERT_EQ(std::hash<DatatypeConstructorDecl>{}(nil1),
            std::hash<DatatypeConstructorDecl>{}(nil2));
  ASSERT_EQ(std::hash<DatatypeConstructor>{}(consConstr1),
            std::hash<DatatypeConstructor>{}(consConstr1));
  ASSERT_EQ(std::hash<DatatypeConstructor>{}(consConstr1),
            std::hash<DatatypeConstructor>{}(consConstr2));
  ASSERT_EQ(std::hash<DatatypeSelector>{}(head1),
            std::hash<DatatypeSelector>{}(head1));
  ASSERT_EQ(std::hash<DatatypeSelector>{}(head1),
            std::hash<DatatypeSelector>{}(head2));
  ASSERT_EQ(std::hash<Datatype>{}(dt1), std::hash<Datatype>{}(dt1));
  ASSERT_EQ(std::hash<Datatype>{}(dt1), std::hash<Datatype>{}(dt2));

  (void)std::hash<ava6::Result>{}(ava6::Result());
}





TEST_F(TestApiBlackUncovered, datatypeApi)
{
  DatatypeDecl dtypeSpec = d_tm.mkDatatypeDecl("list");
  DatatypeConstructorDecl cons = d_tm.mkDatatypeConstructorDecl("cons");
  cons.addSelector("head", d_tm.getIntegerSort());
  dtypeSpec.addConstructor(cons);
  DatatypeConstructorDecl nil = d_tm.mkDatatypeConstructorDecl("nil");
  dtypeSpec.addConstructor(nil);
  Sort listSort = d_tm.mkDatatypeSort(dtypeSpec);
  Datatype d = listSort.getDatatype();

  std::stringstream ss;
  ss << cons;
  ss << d.getConstructor("cons");
  ss << d.getSelector("head");
  ss << std::vector<DatatypeConstructorDecl>{cons, nil};
  ss << d;

  {
    DatatypeConstructor c = d.getConstructor("cons");
    DatatypeConstructor::const_iterator it;
    it = c.begin();
    ASSERT_NE(it, c.end());
    ASSERT_EQ(it, c.begin());
    *it;
    ASSERT_NO_THROW(it->getName());
    ++it;
    it++;
  }
  {
    Datatype::const_iterator it;
    it = d.begin();
    ASSERT_NE(it, d.end());
    ASSERT_EQ(it, d.begin());
    it->getName();
    *it;
    ++it;
    it++;
  }
}

TEST_F(TestApiBlackUncovered, termNativeTypes)
{
  Term t = d_tm.mkInteger(0);
  d_tm.mkReal(0);
  d_tm.mkReal(0, 1);
  d_solver->mkReal(0);
  d_solver->mkReal(0, 1);
  t.isInt32Value();
  t.getInt32Value();
  t.isInt64Value();
  t.getInt64Value();
  t.isUInt32Value();
  t.getUInt32Value();
  t.isUInt64Value();
  t.getUInt64Value();
  t.isReal32Value();
  t.getReal32Value();
  t.isReal64Value();
  t.getReal64Value();
}

TEST_F(TestApiBlackUncovered, termIterators)
{
  Term t = d_tm.mkInteger(0);
  auto it = t.begin();
  auto it2(it);
  it++;
}

TEST_F(TestApiBlackUncovered, checkSatAssumingSingle)
{
  Sort boolsort = d_tm.getBooleanSort();
  Term b = d_tm.mkConst(boolsort, "b");
  d_solver->checkSatAssuming(b);
}

TEST_F(TestApiBlackUncovered, mkOpInitializerList)
{
  d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {1, 1});
  d_solver->mkOp(Kind::BITVECTOR_EXTRACT, {1, 1});
}

TEST_F(TestApiBlackUncovered, mkTermKind)
{
  Term b = d_tm.mkConst(d_tm.getRealSort(), "b");
  d_tm.mkTerm(Kind::GT, {b, b});
  d_solver->mkTerm(Kind::GT, {b, b});
}

TEST_F(TestApiBlackUncovered, getValue)
{
  d_solver->setOption("produce-models", "true");
  Sort boolsort = d_tm.getBooleanSort();
  Term b = d_tm.mkConst(boolsort, "b");
  d_solver->assertFormula(b);
  d_solver->checkSat();
  d_solver->getValue({b, b, b});
}

TEST_F(TestApiBlackUncovered, isOutputOn)
{
  d_solver->isOutputOn("inst");
  d_solver->getOutput("inst");
}



TEST_F(TestApiBlackUncovered, Statistics)
{
  d_solver->assertFormula(d_tm.mkConst(d_tm.getBooleanSort(), "x"));
  d_solver->checkSat();
  Statistics stats = d_solver->getStatistics();
  auto s = stats.get("global::totalTime");
  std::stringstream ss;
  ss << stats << s.toString();
  auto it = stats.begin();
  ASSERT_NE(it, stats.end());
  it++;
  it--;
  ++it;
  --it;
  ASSERT_EQ(it, stats.begin());
  ss << it->first;

  testing::internal::CaptureStdout();
  d_solver->printStatisticsSafe(STDOUT_FILENO);
  d_tm.printStatisticsSafe(STDOUT_FILENO);
  testing::internal::GetCapturedStdout();
}



// Copied from api/cpp/solver_black.cpp


TEST_F(TestApiBlackUncovered, Proof)
{
  Proof proof;
  ASSERT_EQ(proof.getRule(), ProofRule::UNKNOWN);
  ASSERT_TRUE(proof.getResult().isNull());
  ASSERT_TRUE(proof.getChildren().empty());
  ASSERT_TRUE(proof.getArguments().empty());
}

TEST_F(TestApiBlackUncovered, ProofRewriteRule)
{
  ASSERT_EQ(std::hash<ava6::ProofRewriteRule>()(ProofRewriteRule::NONE),
            static_cast<size_t>(ProofRewriteRule::NONE));
}

TEST_F(TestApiBlackUncovered, SkolemId)
{
  ASSERT_EQ(std::hash<ava6::SkolemId>()(SkolemId::PURIFY),
            static_cast<size_t>(SkolemId::PURIFY));
}

TEST_F(TestApiBlackUncovered, Parser)
{
  parser::Command command;
  Solver solver;
  parser::InputParser inputParser(&solver);
  ASSERT_EQ(inputParser.getSolver(), &solver);
  parser::SymbolManager* sm = inputParser.getSymbolManager();
  ASSERT_EQ(sm->isLogicSet(), false);
  std::stringstream ss;
  ss << command << std::endl;
  inputParser.setStreamInput(modes::InputLanguage::SMT_LIB_2_6, ss, "Parser");
  parser::ParserException defaultConstructor;
  std::string message = "error";
  const char* cMessage = "error";
  std::string filename = "file.smt2";
  parser::ParserException stringConstructor(message);
  parser::ParserException cStringConstructor(cMessage);
  parser::ParserException exception(message, filename, 10, 11);
  exception.toStream(ss);
  ASSERT_EQ(message, exception.getMessage());
  ASSERT_EQ(message, exception.getMessage());
  ASSERT_EQ(filename, exception.getFilename());
  ASSERT_EQ(10, exception.getLine());
  ASSERT_EQ(11, exception.getColumn());

  parser::ParserEndOfFileException eofDefault;
  parser::ParserEndOfFileException eofString(message);
  parser::ParserEndOfFileException eofCMessage(cMessage);
  parser::ParserEndOfFileException eof(message, filename, 10, 11);
}

}  // namespace test
}  // namespace ava6::internal
