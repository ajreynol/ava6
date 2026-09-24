/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Testing functions that are not exposed by the Java API for code coverage.
 */

#include <ava6/ava6_parser.h>

#include "test_api.h"

namespace ava6::internal {

namespace test {

class TestApiBlackUncovered : public TestApi
{
};



TEST_F(TestApiBlackUncovered, comparison_operators)
{
  ava6::Result res;
  ASSERT_FALSE(res != res);
  ava6::Sort sort;
  ASSERT_FALSE(sort != sort);
  ASSERT_TRUE(sort <= sort);
  ASSERT_TRUE(sort >= sort);
  ASSERT_FALSE(sort > sort);
  ava6::Op op;
  ASSERT_FALSE(op != op);
  ava6::Term term;
  ASSERT_FALSE(term != term);
  ASSERT_TRUE(term <= term);
  ASSERT_TRUE(term >= term);
  ASSERT_FALSE(term > term);
  ava6::Proof proof;
  ASSERT_FALSE(proof != proof);
}

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

TEST_F(TestApiBlackUncovered, term_native_types)
{
  Term t = d_tm.mkInteger(0);
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

TEST_F(TestApiBlackUncovered, term_iterators)
{
  Term t = d_tm.mkInteger(0);
  t = d_tm.mkTerm(Kind::GT, {t, t});
  Term::const_iterator it;
  it = t.begin();
  auto it2(it);
  ASSERT_FALSE(it == t.end());
  ASSERT_FALSE(it != it2);
  *it2;
  ++it;
  it++;
}



TEST_F(TestApiBlackUncovered, mkString)
{
  std::u32string s;
  ASSERT_EQ(d_tm.mkString(s).getU32StringValue(), s);
}

TEST_F(TestApiBlackUncovered, isOutputOn)
{
  d_solver->isOutputOn("inst");
  d_solver->getOutput("inst");
}

TEST_F(TestApiBlackUncovered, Grammar)
{
  Grammar g;
  ASSERT_FALSE(g != g);
}

TEST_F(TestApiBlackUncovered, Options)
{
  auto dopts = d_solver->getDriverOptions();
  dopts.err();
  dopts.in();
  dopts.out();
}

TEST_F(TestApiBlackUncovered, Statistics)
{
  Stat stat;
  stat = Stat();
  Statistics stats = d_solver->getStatistics();
  auto it = stats.begin();
  it++;
  it--;
  ++it;
  --it;
  std::stringstream ss;
  ss << it->first;
  testing::internal::CaptureStdout();
  d_solver->printStatisticsSafe(STDOUT_FILENO);
  d_tm.printStatisticsSafe(STDOUT_FILENO);
  testing::internal::GetCapturedStdout();
}

TEST_F(TestApiBlackUncovered, Datatypes)
{
  // default constructors
  DatatypeConstructorDecl dtcd;
  DatatypeSelector dts;
  DatatypeConstructor dc;
  DatatypeDecl dtd;
  Datatype d;

  dtd = d_tm.mkDatatypeDecl("list");
  dtcd = d_tm.mkDatatypeConstructorDecl("cons");
  dtcd.addSelector("head", d_tm.getIntegerSort());
  dtd.addConstructor(dtcd);
  Sort s = d_tm.mkDatatypeSort(dtd);
  d = s.getDatatype();
  dc = d.getConstructor("cons");
  dc.getSelector("head");

  {
    Datatype::const_iterator it;
    it = d.begin();
    ASSERT_TRUE(it != d.end());
    *it;
    it->getName();
    ++it;
    ASSERT_TRUE(it == d.end());
    it++;
  }
  {
    DatatypeConstructor::const_iterator it;
    it = dc.begin();
    ASSERT_TRUE(it != dc.end());
    *it;
    it->getName();
    ++it;
    it = dc.begin();
    it++;
    ASSERT_TRUE(it == dc.end());
  }

  {
    std::stringstream ss;
    ss << d;
    ss << dtcd;
    ss << dc;
    ss << dtd;
    ss << d.getSelector("head");
  }
}

TEST_F(TestApiBlackUncovered, Proof)
{
  Proof proof;
  ASSERT_FALSE(proof != proof);
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
  Solver solver(d_tm);
  parser::InputParser inputParser(&solver);
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

class PluginListen : public Plugin
{
 public:
  PluginListen(TermManager& tm)
      : Plugin(tm), d_hasSeenTheoryLemma(false), d_hasSeenSatClause(false)
  {
  }
  virtual ~PluginListen() {}
  void notifySatClause(const Term& cl) override
  {
    Plugin::notifySatClause(cl);  // Cover default implementation
    d_hasSeenSatClause = true;
  }
  bool hasSeenSatClause() const { return d_hasSeenSatClause; }
  void notifyTheoryLemma(const Term& lem) override
  {
    Plugin::notifyTheoryLemma(lem);  // Cover default implementation
    d_hasSeenTheoryLemma = true;
  }
  bool hasSeenTheoryLemma() const { return d_hasSeenTheoryLemma; }
  std::string getName() override { return "PluginListen"; }

 private:
  /** have we seen a theory lemma? */
  bool d_hasSeenTheoryLemma;
  /** have we seen a SAT clause? */
  bool d_hasSeenSatClause;
};


}  // namespace test
}  // namespace ava6::internal
