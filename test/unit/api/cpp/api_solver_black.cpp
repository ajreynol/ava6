/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of the Solver class of the  C++ API.
 */

#include <ava6/ava6_types.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

#include "base/output.h"
#include "test_api.h"

namespace ava6::internal {

namespace test {

class TestApiBlackSolver : public TestApi
{
};


TEST_F(TestApiBlackSolver, recoverableException)
{
  d_solver->setOption("produce-models", "true");
  Term x = d_tm.mkConst(d_bool, "x");
  d_solver->assertFormula(x.eqTerm(x).notTerm());
  ASSERT_THROW(d_solver->getValue(x), Ava6ApiRecoverableException);

  try
  {
    d_solver->getValue(x);
  }
  catch (const Ava6ApiRecoverableException& e)
  {
    ASSERT_NO_THROW(e.what());
    ASSERT_NO_THROW(e.getMessage());
  }
}

TEST_F(TestApiBlackSolver, simplify)
{
  ASSERT_THROW(d_solver->simplify(Term()), Ava6ApiException);

  Sort bvSort = d_tm.mkBitVectorSort(32);
  Sort funSort1 = d_tm.mkFunctionSort({bvSort, bvSort}, bvSort);
  Sort funSort2 = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  DatatypeDecl consListSpec = d_tm.mkDatatypeDecl("list");
  DatatypeConstructorDecl cons = d_tm.mkDatatypeConstructorDecl("cons");
  cons.addSelector("head", d_int);
  cons.addSelectorSelf("tail");
  consListSpec.addConstructor(cons);
  DatatypeConstructorDecl nil = d_tm.mkDatatypeConstructorDecl("nil");
  consListSpec.addConstructor(nil);
  Sort consListSort = d_tm.mkDatatypeSort(consListSpec);

  Term x = d_tm.mkConst(bvSort, "x");
  ASSERT_NO_THROW(d_solver->simplify(x));
  Term a = d_tm.mkConst(bvSort, "a");
  ASSERT_NO_THROW(d_solver->simplify(a));
  Term b = d_tm.mkConst(bvSort, "b");
  ASSERT_NO_THROW(d_solver->simplify(b));
  Term x_eq_x = d_tm.mkTerm(Kind::EQUAL, {x, x});
  ASSERT_NO_THROW(d_solver->simplify(x_eq_x));
  ASSERT_NE(d_tm.mkTrue(), x_eq_x);
  ASSERT_EQ(d_tm.mkTrue(), d_solver->simplify(x_eq_x));
  Term x_eq_b = d_tm.mkTerm(Kind::EQUAL, {x, b});
  ASSERT_NO_THROW(d_solver->simplify(x_eq_b));
  ASSERT_NE(d_tm.mkTrue(), x_eq_b);
  ASSERT_NE(d_tm.mkTrue(), d_solver->simplify(x_eq_b));

  Term i1 = d_tm.mkConst(d_int, "i1");
  ASSERT_NO_THROW(d_solver->simplify(i1));
  Term i2 = d_tm.mkTerm(Kind::MULT, {i1, d_tm.mkInteger("23")});
  ASSERT_NO_THROW(d_solver->simplify(i2));
  ASSERT_NE(i1, i2);
  ASSERT_NE(i1, d_solver->simplify(i2));
  Term i3 = d_tm.mkTerm(Kind::ADD, {i1, d_tm.mkInteger(0)});
  ASSERT_NO_THROW(d_solver->simplify(i3));
  ASSERT_NE(i1, i3);
  ASSERT_EQ(i1, d_solver->simplify(i3));

  Datatype consList = consListSort.getDatatype();
  Term dt1 =
      d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR,
                  {consList.getConstructor("cons").getTerm(),
                   d_tm.mkInteger(0),
                   d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR,
                               {consList.getConstructor("nil").getTerm()})});
  ASSERT_NO_THROW(d_solver->simplify(dt1));
  Term dt2 = d_tm.mkTerm(Kind::APPLY_SELECTOR,
                         {consList["cons"].getSelector("head").getTerm(), dt1});
  ASSERT_NO_THROW(d_solver->simplify(dt2));

  Term b1 = d_tm.mkVar(bvSort, "b1");
  ASSERT_NO_THROW(d_solver->simplify(b1));
  Term b2 = d_tm.mkVar(bvSort, "b1");
  ASSERT_NO_THROW(d_solver->simplify(b2));
  Term b3 = d_tm.mkVar(d_uninterpreted, "b3");
  ASSERT_NO_THROW(d_solver->simplify(b3));
  Term v1 = d_tm.mkConst(bvSort, "v1");
  ASSERT_NO_THROW(d_solver->simplify(v1));
  Term v2 = d_tm.mkConst(d_int, "v2");
  ASSERT_NO_THROW(d_solver->simplify(v2));
  Term f1 = d_tm.mkConst(funSort1, "f1");
  ASSERT_NO_THROW(d_solver->simplify(f1));
  Term f2 = d_tm.mkConst(funSort2, "f2");
  ASSERT_NO_THROW(d_solver->simplify(f2));
  d_solver->defineFunsRec({f1, f2}, {{b1, b2}, {b3}}, {v1, v2});
  ASSERT_NO_THROW(d_solver->simplify(f1));
  ASSERT_NO_THROW(d_solver->simplify(f2));

  TermManager tm;
  Solver slv(tm);
  ASSERT_THROW(slv.simplify(x), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, simplifyApplySubs)
{
  d_solver->setOption("incremental", "true");
  Term x = d_tm.mkConst(d_int, "x");
  Term zero = d_tm.mkInteger(0);
  Term eq = d_tm.mkTerm(Kind::EQUAL, {x, zero});
  d_solver->assertFormula(eq);
  ASSERT_NO_THROW(d_solver->checkSat());

  ASSERT_EQ(d_solver->simplify(x, false), x);
  ASSERT_EQ(d_solver->simplify(x, true), zero);
}

TEST_F(TestApiBlackSolver, assertFormula)
{
  ASSERT_NO_THROW(d_solver->assertFormula(d_tm.mkTrue()));
  ASSERT_THROW(d_solver->assertFormula(Term()), Ava6ApiException);
  TermManager tm;
  Solver slv(tm);
  ASSERT_THROW(slv.assertFormula(d_tm.mkTrue()), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, checkSat)
{
  d_solver->setOption("incremental", "false");
  ASSERT_NO_THROW(d_solver->checkSat());
  ASSERT_THROW(d_solver->checkSat(), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, checkSatAssuming)
{
  d_solver->setOption("incremental", "false");
  ASSERT_NO_THROW(d_solver->checkSatAssuming(d_tm.mkTrue()));
  ASSERT_THROW(d_solver->checkSatAssuming(d_tm.mkTrue()), Ava6ApiException);
  TermManager tm;
  Solver slv(tm);
  ASSERT_THROW(slv.checkSatAssuming(d_tm.mkTrue()), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, checkSatAssuming1)
{
  Term x = d_tm.mkConst(d_bool, "x");
  Term y = d_tm.mkConst(d_bool, "y");
  Term z = d_tm.mkTerm(Kind::AND, {x, y});
  d_solver->setOption("incremental", "true");
  ASSERT_NO_THROW(d_solver->checkSatAssuming(d_tm.mkTrue()));
  ASSERT_THROW(d_solver->checkSatAssuming(Term()), Ava6ApiException);
  ASSERT_NO_THROW(d_solver->checkSatAssuming(d_tm.mkTrue()));
  ASSERT_NO_THROW(d_solver->checkSatAssuming(z));
}

TEST_F(TestApiBlackSolver, checkSatAssuming2)
{
  d_solver->setOption("incremental", "true");

  Sort uToIntSort = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  Sort intPredSort = d_tm.mkFunctionSort({d_int}, d_bool);

  Term n = Term();
  // Constants
  Term x = d_tm.mkConst(d_uninterpreted, "x");
  Term y = d_tm.mkConst(d_uninterpreted, "y");
  // Functions
  Term f = d_tm.mkConst(uToIntSort, "f");
  Term p = d_tm.mkConst(intPredSort, "p");
  // Values
  Term zero = d_tm.mkInteger(0);
  Term one = d_tm.mkInteger(1);
  // Terms
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  Term f_y = d_tm.mkTerm(Kind::APPLY_UF, {f, y});
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_y});
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  Term p_f_y = d_tm.mkTerm(Kind::APPLY_UF, {p, f_y});
  // Assertions
  Term assertions =
      d_tm.mkTerm(Kind::AND,
                  {
                      d_tm.mkTerm(Kind::LEQ, {zero, f_x}),  // 0 <= f(x)
                      d_tm.mkTerm(Kind::LEQ, {zero, f_y}),  // 0 <= f(y)
                      d_tm.mkTerm(Kind::LEQ, {sum, one}),   // f(x) + f(y) <= 1
                      p_0.notTerm(),                        // not p(0)
                      p_f_y                                 // p(f(y))
                  });

  ASSERT_NO_THROW(d_solver->checkSatAssuming(d_tm.mkTrue()));
  d_solver->assertFormula(assertions);
  ASSERT_NO_THROW(
      d_solver->checkSatAssuming(d_tm.mkTerm(Kind::DISTINCT, {x, y})));
  ASSERT_NO_THROW(d_solver->checkSatAssuming(
      {d_tm.mkFalse(), d_tm.mkTerm(Kind::DISTINCT, {x, y})}));
  ASSERT_THROW(d_solver->checkSatAssuming(n), Ava6ApiException);
  ASSERT_THROW(
      d_solver->checkSatAssuming({n, d_tm.mkTerm(Kind::DISTINCT, {x, y})}),
      Ava6ApiException);
}

TEST_F(TestApiBlackSolver, declareFunFresh)
{
  Term t1 = d_solver->declareFun(std::string("b"), {}, d_bool, true);
  Term t2 = d_solver->declareFun(std::string("b"), {}, d_bool, false);
  Term t3 = d_solver->declareFun(std::string("b"), {}, d_bool, false);
  ASSERT_FALSE(t1 == t2);
  ASSERT_FALSE(t1 == t3);
  ASSERT_TRUE(t2 == t3);
  Term t4 = d_solver->declareFun(std::string("c"), {}, d_bool, false);
  ASSERT_FALSE(t2 == t4);
  Term t5 = d_solver->declareFun(std::string("b"), {}, d_int, false);
  ASSERT_FALSE(t2 == t5);

  TermManager tm;
  Solver slv(tm);
  ASSERT_THROW(slv.declareFun(std::string("b"), {}, d_int, false),
               Ava6ApiException);
}

TEST_F(TestApiBlackSolver, declareDatatype)
{
  DatatypeConstructorDecl lin = d_tm.mkDatatypeConstructorDecl("lin");
  std::vector<DatatypeConstructorDecl> ctors0 = {lin};
  ASSERT_NO_THROW(d_solver->declareDatatype(std::string(""), ctors0));

  DatatypeConstructorDecl nil = d_tm.mkDatatypeConstructorDecl("nil");
  std::vector<DatatypeConstructorDecl> ctors1 = {nil};
  ASSERT_NO_THROW(d_solver->declareDatatype(std::string("a"), ctors1));

  DatatypeConstructorDecl cons = d_tm.mkDatatypeConstructorDecl("cons");
  DatatypeConstructorDecl nil2 = d_tm.mkDatatypeConstructorDecl("nil");
  std::vector<DatatypeConstructorDecl> ctors2 = {cons, nil2};
  ASSERT_NO_THROW(d_solver->declareDatatype(std::string("b"), ctors2));

  DatatypeConstructorDecl cons2 = d_tm.mkDatatypeConstructorDecl("cons");
  DatatypeConstructorDecl nil3 = d_tm.mkDatatypeConstructorDecl("nil");
  std::vector<DatatypeConstructorDecl> ctors3 = {cons2, nil3};
  ASSERT_NO_THROW(d_solver->declareDatatype(std::string(""), ctors3));

  // must have at least one constructor
  std::vector<DatatypeConstructorDecl> ctors4;
  ASSERT_THROW(d_solver->declareDatatype(std::string("c"), ctors4),
               Ava6ApiException);
  // constructors may not be reused
  DatatypeConstructorDecl ctor1 = d_tm.mkDatatypeConstructorDecl("_x21");
  DatatypeConstructorDecl ctor2 = d_tm.mkDatatypeConstructorDecl("_x31");
  d_solver->declareDatatype(std::string("_x17"), {ctor1, ctor2});
  ASSERT_THROW(d_solver->declareDatatype(std::string("_x86"), {ctor1, ctor2}),
               Ava6ApiException);

  TermManager tm;
  Solver slv(tm);
  DatatypeConstructorDecl nnil = d_tm.mkDatatypeConstructorDecl("nil");
  ASSERT_THROW(slv.declareDatatype(std::string("a"), {nnil}), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, declareFun)
{
  Sort bvSort = d_tm.mkBitVectorSort(32);
  Sort funSort = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  ASSERT_NO_THROW(d_solver->declareFun("f1", {}, bvSort));
  ASSERT_NO_THROW(d_solver->declareFun("f3", {bvSort, d_int}, bvSort));
  ASSERT_THROW(d_solver->declareFun("f2", {}, funSort), Ava6ApiException);
  // functions as arguments is allowed
  ASSERT_THROW(d_solver->declareFun("f4", {bvSort, funSort}, bvSort),
               Ava6ApiException);
  ASSERT_THROW(d_solver->declareFun("f5", {bvSort, bvSort}, funSort),
               Ava6ApiException);

  TermManager tm;
  Solver slv(tm);
  ASSERT_THROW(slv.declareFun("f1", {}, bvSort), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, declareSort)
{
  ASSERT_NO_THROW(d_solver->declareSort("s", 0));
  ASSERT_NO_THROW(d_solver->declareSort("s", 2));
  ASSERT_NO_THROW(d_solver->declareSort("", 2));
}

TEST_F(TestApiBlackSolver, declareSortFresh)
{
  Sort t1 = d_solver->declareSort(std::string("b"), 0, true);
  Sort t2 = d_solver->declareSort(std::string("b"), 0, false);
  Sort t3 = d_solver->declareSort(std::string("b"), 0, false);
  ASSERT_FALSE(t1 == t2);
  ASSERT_FALSE(t1 == t3);
  ASSERT_TRUE(t2 == t3);
  Sort t4 = d_solver->declareSort(std::string("c"), 0, false);
  ASSERT_FALSE(t2 == t4);
  Sort t5 = d_solver->declareSort(std::string("b"), 1, false);
  ASSERT_FALSE(t2 == t5);
}

TEST_F(TestApiBlackSolver, defineFun)
{
  Sort bvSort = d_tm.mkBitVectorSort(32);
  Sort funSort = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  Term b1 = d_tm.mkVar(bvSort, "b1");
  Term b2 = d_tm.mkVar(d_int, "b2");
  Term v1 = d_tm.mkConst(bvSort, "v1");
  Term v2 = d_tm.mkConst(funSort, "v2");
  ASSERT_NO_THROW(d_solver->defineFun("f", {}, bvSort, v1));
  ASSERT_NO_THROW(d_solver->defineFun("ff", {b1, b2}, bvSort, v1));
  ASSERT_THROW(d_solver->defineFun("ff", {v1, b2}, bvSort, v1),
               Ava6ApiException);
  ASSERT_THROW(d_solver->defineFun("fff", {b1}, bvSort, v2), Ava6ApiException);
  ASSERT_THROW(d_solver->defineFun("ffff", {b1}, funSort, v2),
               Ava6ApiException);

  TermManager tm;
  Solver slv(tm);
  Sort bvSort2 = tm.mkBitVectorSort(32);
  Term v12 = tm.mkConst(bvSort2, "v1");
  Term b12 = tm.mkVar(bvSort2, "b1");
  Term b22 = tm.mkVar(tm.getIntegerSort(), "b2");
  ASSERT_THROW(slv.defineFun("f", {}, bvSort, v12), Ava6ApiException);
  ASSERT_THROW(slv.defineFun("f", {}, bvSort2, v1), Ava6ApiException);
  ASSERT_THROW(slv.defineFun("ff", {b1, b22}, bvSort2, v12), Ava6ApiException);
  ASSERT_THROW(slv.defineFun("ff", {b12, b2}, bvSort2, v12), Ava6ApiException);
  ASSERT_THROW(slv.defineFun("ff", {b12, b22}, bvSort, v12), Ava6ApiException);
  ASSERT_THROW(slv.defineFun("ff", {b12, b22}, bvSort2, v1), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, defineFunGlobal)
{
  Term bTrue = d_tm.mkBoolean(true);
  // (define-fun f () Bool true)
  Term f = d_solver->defineFun("f", {}, d_bool, bTrue, true);
  Term b = d_tm.mkVar(d_bool, "b");
  // (define-fun g (b Bool) Bool b)
  Term g = d_solver->defineFun("g", {b}, d_bool, b, true);

  // (assert (or (not f) (not (g true))))
  d_solver->assertFormula(d_tm.mkTerm(
      Kind::OR,
      {f.notTerm(), d_tm.mkTerm(Kind::APPLY_UF, {g, bTrue}).notTerm()}));
  ASSERT_TRUE(d_solver->checkSat().isUnsat());
  d_solver->resetAssertions();
  // (assert (or (not f) (not (g true))))
  d_solver->assertFormula(d_tm.mkTerm(
      Kind::OR,
      {f.notTerm(), d_tm.mkTerm(Kind::APPLY_UF, {g, bTrue}).notTerm()}));
  ASSERT_TRUE(d_solver->checkSat().isUnsat());

  TermManager tm;
  Solver slv(tm);
  ASSERT_THROW(slv.defineFun("f", {}, d_bool, bTrue, true), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, defineFunRec)
{
  Sort bvSort = d_tm.mkBitVectorSort(32);
  Sort funSort1 = d_tm.mkFunctionSort({bvSort, bvSort}, bvSort);
  Sort funSort2 = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  Term b1 = d_tm.mkVar(bvSort, "b1");
  Term b11 = d_tm.mkVar(bvSort, "b1");
  Term b2 = d_tm.mkVar(d_int, "b2");
  Term v1 = d_tm.mkConst(bvSort, "v1");
  Term v2 = d_tm.mkConst(d_int, "v2");
  Term v3 = d_tm.mkConst(funSort2, "v3");
  Term f1 = d_tm.mkConst(funSort1, "f1");
  Term f2 = d_tm.mkConst(funSort2, "f2");
  Term f3 = d_tm.mkConst(bvSort, "f3");
  ASSERT_NO_THROW(d_solver->defineFunRec("f", {}, bvSort, v1));
  ASSERT_NO_THROW(d_solver->defineFunRec("ff", {b1, b2}, bvSort, v1));
  ASSERT_NO_THROW(d_solver->defineFunRec(f1, {b1, b11}, v1));
  ASSERT_THROW(d_solver->defineFunRec("fff", {b1}, bvSort, v3),
               Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunRec("ff", {b1, v2}, bvSort, v1),
               Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunRec("ffff", {b1}, funSort2, v3),
               Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunRec(f1, {b1}, v1), Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunRec(f1, {b1, b11}, v2), Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunRec(f1, {b1, b11}, v3), Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunRec(f2, {b1}, v2), Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunRec(f3, {b1}, v1), Ava6ApiException);

  TermManager tm;
  Solver slv(tm);
  Sort bvSort2 = tm.mkBitVectorSort(32);
  Term v12 = tm.mkConst(bvSort2, "v1");
  Term b12 = tm.mkVar(bvSort2, "b1");
  Term b22 = tm.mkVar(tm.getIntegerSort(), "b2");
  ASSERT_NO_THROW(slv.defineFunRec("f", {}, bvSort2, v12));
  ASSERT_NO_THROW(slv.defineFunRec("ff", {b12, b22}, bvSort2, v12));
  ASSERT_THROW(slv.defineFunRec("f", {}, bvSort, v12), Ava6ApiException);
  ASSERT_THROW(slv.defineFunRec("f", {}, bvSort2, v1), Ava6ApiException);
  ASSERT_THROW(slv.defineFunRec("ff", {b1, b22}, bvSort2, v12),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunRec("ff", {b12, b2}, bvSort2, v12),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunRec("ff", {b12, b22}, bvSort, v12),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunRec("ff", {b12, b22}, bvSort2, v1),
               Ava6ApiException);
}

TEST_F(TestApiBlackSolver, defineFunRecWrongLogic)
{
  d_solver->setLogic("QF_BV");
  Sort bvSort = d_tm.mkBitVectorSort(32);
  Sort funSort = d_tm.mkFunctionSort({bvSort, bvSort}, bvSort);
  Term b = d_tm.mkVar(bvSort, "b");
  Term v = d_tm.mkConst(bvSort, "v");
  Term f = d_tm.mkConst(funSort, "f");
  ASSERT_THROW(d_solver->defineFunRec("f", {}, bvSort, v), Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunRec(f, {b, b}, v), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, defineFunRecGlobal)
{
  d_solver->push();
  Term bTrue = d_tm.mkBoolean(true);
  // (define-fun f () Bool true)
  Term f = d_solver->defineFunRec("f", {}, d_bool, bTrue, true);
  Term b = d_tm.mkVar(d_bool, "b");
  // (define-fun g (b Bool) Bool b)
  Term g = d_solver->defineFunRec(
      d_tm.mkConst(d_tm.mkFunctionSort({d_bool}, d_bool), "g"), {b}, b, true);

  // (assert (or (not f) (not (g true))))
  d_solver->assertFormula(d_tm.mkTerm(
      Kind::OR,
      {f.notTerm(), d_tm.mkTerm(Kind::APPLY_UF, {g, bTrue}).notTerm()}));
  ASSERT_TRUE(d_solver->checkSat().isUnsat());
  d_solver->pop();
  // (assert (or (not f) (not (g true))))
  d_solver->assertFormula(d_tm.mkTerm(
      Kind::OR,
      {f.notTerm(), d_tm.mkTerm(Kind::APPLY_UF, {g, bTrue}).notTerm()}));
  ASSERT_TRUE(d_solver->checkSat().isUnsat());

  TermManager tm;
  Solver slv(tm);
  Term bb = tm.mkVar(tm.getBooleanSort(), "b");
  ASSERT_THROW(
      slv.defineFunRec(d_tm.mkConst(d_tm.mkFunctionSort({d_bool}, d_bool), "g"),
                       {bb},
                       bb,
                       true),
      Ava6ApiException);
  ASSERT_THROW(
      slv.defineFunRec(
          tm.mkConst(tm.mkFunctionSort({d_bool}, d_bool), "g"), {b}, b, true),
      Ava6ApiException);
}

TEST_F(TestApiBlackSolver, defineFunsRec)
{
  Sort bvSort = d_tm.mkBitVectorSort(32);
  Sort funSort1 = d_tm.mkFunctionSort({bvSort, bvSort}, bvSort);
  Sort funSort2 = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  Term b1 = d_tm.mkVar(bvSort, "b1");
  Term b11 = d_tm.mkVar(bvSort, "b1");
  Term b2 = d_tm.mkVar(d_int, "b2");
  Term b4 = d_tm.mkVar(d_uninterpreted, "b4");
  Term v1 = d_tm.mkConst(bvSort, "v1");
  Term v2 = d_tm.mkConst(d_int, "v2");
  Term v4 = d_tm.mkConst(d_uninterpreted, "v4");
  Term f1 = d_tm.mkConst(funSort1, "f1");
  Term f2 = d_tm.mkConst(funSort2, "f2");
  Term f3 = d_tm.mkConst(bvSort, "f3");
  ASSERT_NO_THROW(
      d_solver->defineFunsRec({f1, f2}, {{b1, b11}, {b4}}, {v1, v2}));
  ASSERT_THROW(d_solver->defineFunsRec({f1, f2}, {{v1, b11}, {b4}}, {v1, v2}),
               Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunsRec({f1, f3}, {{b1, b11}, {b4}}, {v1, v2}),
               Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunsRec({f1, f2}, {{b1}, {b4}}, {v1, v2}),
               Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunsRec({f1, f2}, {{b1, b2}, {b4}}, {v1, v2}),
               Ava6ApiException);
  ASSERT_THROW(d_solver->defineFunsRec({f1, f2}, {{b1, b11}, {b4}}, {v1, v4}),
               Ava6ApiException);

  TermManager tm;
  Solver slv(tm);
  Sort uSort2 = tm.mkUninterpretedSort("u");
  Sort bvSort2 = tm.mkBitVectorSort(32);
  Sort funSort12 = tm.mkFunctionSort({bvSort2, bvSort2}, bvSort2);
  Sort funSort22 = tm.mkFunctionSort({uSort2}, tm.getIntegerSort());
  Term b12 = tm.mkVar(bvSort2, "b1");
  Term b112 = tm.mkVar(bvSort2, "b1");
  Term b42 = tm.mkVar(uSort2, "b4");
  Term v12 = tm.mkConst(bvSort2, "v1");
  Term v22 = tm.mkConst(tm.getIntegerSort(), "v2");
  Term f12 = tm.mkConst(funSort12, "f1");
  Term f22 = tm.mkConst(funSort22, "f2");
  ASSERT_NO_THROW(
      slv.defineFunsRec({f12, f22}, {{b12, b112}, {b42}}, {v12, v22}));
  ASSERT_THROW(slv.defineFunsRec({f1, f22}, {{b12, b112}, {b42}}, {v12, v22}),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunsRec({f12, f2}, {{b12, b112}, {b42}}, {v12, v22}),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunsRec({f12, f22}, {{b1, b112}, {b42}}, {v12, v22}),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunsRec({f12, f22}, {{b12, b11}, {b42}}, {v12, v22}),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunsRec({f12, f22}, {{b12, b112}, {b4}}, {v12, v22}),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunsRec({f12, f22}, {{b12, b112}, {b42}}, {v1, v22}),
               Ava6ApiException);
  ASSERT_THROW(slv.defineFunsRec({f12, f22}, {{b12, b112}, {b42}}, {v12, v2}),
               Ava6ApiException);
}

TEST_F(TestApiBlackSolver, defineFunsRecWrongLogic)
{
  d_solver->setLogic("QF_BV");
  Sort bvSort = d_tm.mkBitVectorSort(32);
  Sort funSort1 = d_tm.mkFunctionSort({bvSort, bvSort}, bvSort);
  Sort funSort2 = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  Term b = d_tm.mkVar(bvSort, "b");
  Term u = d_tm.mkVar(d_uninterpreted, "u");
  Term v1 = d_tm.mkConst(bvSort, "v1");
  Term v2 = d_tm.mkConst(d_int, "v2");
  Term f1 = d_tm.mkConst(funSort1, "f1");
  Term f2 = d_tm.mkConst(funSort2, "f2");
  ASSERT_THROW(d_solver->defineFunsRec({f1, f2}, {{b, b}, {u}}, {v1, v2}),
               Ava6ApiException);
}

TEST_F(TestApiBlackSolver, defineFunsRecGlobal)
{
  Sort fSort = d_tm.mkFunctionSort({d_bool}, d_bool);

  d_solver->push();
  Term bTrue = d_tm.mkBoolean(true);
  Term b = d_tm.mkVar(d_bool, "b");
  Term gSym = d_tm.mkConst(fSort, "g");
  // (define-funs-rec ((g ((b Bool)) Bool)) (b))
  d_solver->defineFunsRec({gSym}, {{b}}, {b}, true);

  // (assert (not (g true)))
  d_solver->assertFormula(d_tm.mkTerm(Kind::APPLY_UF, {gSym, bTrue}).notTerm());
  ASSERT_TRUE(d_solver->checkSat().isUnsat());
  d_solver->pop();
  // (assert (not (g true)))
  d_solver->assertFormula(d_tm.mkTerm(Kind::APPLY_UF, {gSym, bTrue}).notTerm());
  ASSERT_TRUE(d_solver->checkSat().isUnsat());
}

TEST_F(TestApiBlackSolver, getAssertions)
{
  Term a = d_tm.mkConst(d_bool, "a");
  Term b = d_tm.mkConst(d_bool, "b");
  d_solver->assertFormula(a);
  d_solver->assertFormula(b);
  std::vector<Term> asserts{a, b};
  ASSERT_EQ(d_solver->getAssertions(), asserts);
}

TEST_F(TestApiBlackSolver, getInfo)
{
  ASSERT_NO_THROW(d_solver->getInfo("name"));
  ASSERT_THROW(d_solver->getInfo("asdf"), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getOption)
{
  ASSERT_NO_THROW(d_solver->getOption("incremental"));
  ASSERT_THROW(d_solver->getOption("asdf"), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getOptionNames)
{
  std::vector<std::string> names = d_solver->getOptionNames();
  ASSERT_TRUE(names.size() > 100);
  ASSERT_NE(std::find(names.begin(), names.end(), "verbose"), names.end());
  ASSERT_EQ(std::find(names.begin(), names.end(), "foobar"), names.end());
}


TEST_F(TestApiBlackSolver, getUnsatAssumptions1)
{
  d_solver->setOption("incremental", "false");
  d_solver->checkSatAssuming(d_tm.mkFalse());
  ASSERT_THROW(d_solver->getUnsatAssumptions(), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getUnsatAssumptions2)
{
  d_solver->setOption("incremental", "true");
  d_solver->setOption("produce-unsat-assumptions", "false");
  d_solver->checkSatAssuming(d_tm.mkFalse());
  ASSERT_THROW(d_solver->getUnsatAssumptions(), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getUnsatAssumptions3)
{
  d_solver->setOption("incremental", "true");
  d_solver->setOption("produce-unsat-assumptions", "true");
  d_solver->checkSatAssuming(d_tm.mkFalse());
  ASSERT_NO_THROW(d_solver->getUnsatAssumptions());
  d_solver->checkSatAssuming(d_tm.mkTrue());
  ASSERT_THROW(d_solver->getUnsatAssumptions(), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getUnsatCore1)
{
  d_solver->setOption("incremental", "false");
  d_solver->assertFormula(d_tm.mkFalse());
  d_solver->checkSat();
  ASSERT_THROW(d_solver->getUnsatCore(), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getUnsatCore2)
{
  d_solver->setOption("incremental", "false");
  d_solver->setOption("produce-unsat-cores", "false");
  d_solver->assertFormula(d_tm.mkFalse());
  d_solver->checkSat();
  ASSERT_THROW(d_solver->getUnsatCore(), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getUnsatCoreAndProof)
{
  d_solver->setOption("incremental", "true");
  d_solver->setOption("produce-unsat-cores", "true");
  d_solver->setOption("produce-proofs", "true");

  Sort uToIntSort = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  Sort intPredSort = d_tm.mkFunctionSort({d_int}, d_bool);
  std::vector<Term> uc;

  Term x = d_tm.mkConst(d_uninterpreted, "x");
  Term y = d_tm.mkConst(d_uninterpreted, "y");
  Term f = d_tm.mkConst(uToIntSort, "f");
  Term p = d_tm.mkConst(intPredSort, "p");
  Term zero = d_tm.mkInteger(0);
  Term one = d_tm.mkInteger(1);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  Term f_y = d_tm.mkTerm(Kind::APPLY_UF, {f, y});
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_y});
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  Term p_f_y = d_tm.mkTerm(Kind::APPLY_UF, {p, f_y});
  d_solver->assertFormula(d_tm.mkTerm(Kind::GT, {zero, f_x}));
  d_solver->assertFormula(d_tm.mkTerm(Kind::GT, {zero, f_y}));
  d_solver->assertFormula(d_tm.mkTerm(Kind::GT, {sum, one}));
  d_solver->assertFormula(p_0);
  d_solver->assertFormula(p_f_y.notTerm());
  ASSERT_TRUE(d_solver->checkSat().isUnsat());

  ASSERT_NO_THROW(uc = d_solver->getUnsatCore());
  ASSERT_FALSE(uc.empty());

  ASSERT_NO_THROW(d_solver->getProof());
  ASSERT_NO_THROW(d_solver->getProof(modes::ProofComponent::SAT));

  d_solver->resetAssertions();
  for (const auto& t : uc)
  {
    d_solver->assertFormula(t);
  }
  ava6::Result res = d_solver->checkSat();
  ASSERT_TRUE(res.isUnsat());
  ASSERT_NO_THROW(d_solver->getProof());
}


TEST_F(TestApiBlackSolver, getUnsatCoreLemmas2)
{
  d_solver->setOption("incremental", "true");
  d_solver->setOption("produce-unsat-cores", "true");
  d_solver->setOption("produce-proofs", "true");

  Sort uToIntSort = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  Sort intPredSort = d_tm.mkFunctionSort({d_int}, d_bool);

  Term x = d_tm.mkConst(d_uninterpreted, "x");
  Term y = d_tm.mkConst(d_uninterpreted, "y");
  Term f = d_tm.mkConst(uToIntSort, "f");
  Term p = d_tm.mkConst(intPredSort, "p");
  Term zero = d_tm.mkInteger(0);
  Term one = d_tm.mkInteger(1);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  Term f_y = d_tm.mkTerm(Kind::APPLY_UF, {f, y});
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_y});
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  Term p_f_y = d_tm.mkTerm(Kind::APPLY_UF, {p, f_y});
  d_solver->assertFormula(d_tm.mkTerm(Kind::GT, {zero, f_x}));
  d_solver->assertFormula(d_tm.mkTerm(Kind::GT, {zero, f_y}));
  d_solver->assertFormula(d_tm.mkTerm(Kind::GT, {sum, one}));
  d_solver->assertFormula(p_0);
  d_solver->assertFormula(p_f_y.notTerm());
  ASSERT_TRUE(d_solver->checkSat().isUnsat());

  ASSERT_NO_THROW(d_solver->getUnsatCoreLemmas());
}


TEST_F(TestApiBlackSolver, getDriverOptions)
{
  auto dopts = d_solver->getDriverOptions();
  ASSERT_EQ(dopts.err().rdbuf(), std::cerr.rdbuf());
  ASSERT_EQ(dopts.in().rdbuf(), std::cin.rdbuf());
  ASSERT_EQ(dopts.out().rdbuf(), std::cout.rdbuf());
}

TEST_F(TestApiBlackSolver, getStatistics)
{
  ASSERT_NO_THROW(ava6::Stat());
  // do some array reasoning to make sure we have statistics
  {
    Sort s2 = d_tm.mkArraySort(d_int, d_int);
    Term t1 = d_tm.mkConst(d_int, "i");
    Term t2 = d_tm.mkConst(s2, "a");
    Term t3 = d_tm.mkTerm(Kind::SELECT, {t2, t1});
    d_solver->assertFormula(t3.eqTerm(t1));
    d_solver->checkSat();
  }
  ava6::Statistics stats = d_solver->getStatistics();
  std::stringstream ss;
  ss << stats;
  {
    auto s = stats.get("global::totalTime");
    ASSERT_FALSE(s.isInternal());
    ASSERT_FALSE(s.isDefault());
    ASSERT_TRUE(s.isString());
    std::string time = s.getString();
    ASSERT_TRUE(time.rfind("ms") == time.size() - 2);  // ends with "ms"
    ASSERT_FALSE(s.isDouble());
    ss << s << s.toString();
    s = stats.get("resource::resourceUnitsUsed");
    ASSERT_TRUE(s.isInternal());
    ASSERT_FALSE(s.isDefault());
    ASSERT_TRUE(s.isInt());
    ASSERT_TRUE(s.getInt() >= 0);
    ss << s << s.toString();
  }
  bool hasstats = false;
  for (const auto& s : stats)
  {
    hasstats = true;
    ASSERT_FALSE(s.first.empty());
  }
  ASSERT_TRUE(hasstats);
  hasstats = false;
  for (auto it = stats.begin(true, true); it != stats.end(); ++it)
  {
    hasstats = true;
    {
      auto tmp1 = it, tmp2 = it;
      ++tmp1;
      tmp2++;
      ASSERT_EQ(tmp1, tmp2);
      --tmp1;
      tmp2--;
      ASSERT_EQ(tmp1, tmp2);
      ASSERT_EQ(tmp1, it);
      ASSERT_EQ(it, tmp2);
    }
    const auto& s = *it;
    // check some basic utility methods
    ASSERT_TRUE(!(it == stats.end()));
    ASSERT_EQ(s.first, it->first);
    if (s.first == "theory::arrays::avgIndexListLength")
    {
      ASSERT_TRUE(s.second.isInternal());
      ASSERT_TRUE(s.second.isDouble());
      ASSERT_TRUE(std::isnan(s.second.getDouble()));
    }
  }
  ASSERT_TRUE(hasstats);
}

TEST_F(TestApiBlackSolver, printStatisticsSafe)
{
  // do some array reasoning to make sure we have statistics
  {
    Sort s2 = d_tm.mkArraySort(d_int, d_int);
    Term t1 = d_tm.mkConst(d_int, "i");
    Term t2 = d_tm.mkConst(s2, "a");
    Term t3 = d_tm.mkTerm(Kind::SELECT, {t2, t1});
    d_solver->assertFormula(t3.eqTerm(t1));
    d_solver->checkSat();
  }
  testing::internal::CaptureStdout();
  d_solver->printStatisticsSafe(STDOUT_FILENO);
  testing::internal::GetCapturedStdout();
}


TEST_F(TestApiBlackSolver, getValue1)
{
  d_solver->setOption("produce-models", "false");
  Term t = d_tm.mkTrue();
  d_solver->assertFormula(t);
  d_solver->checkSat();
  ASSERT_THROW(d_solver->getValue(t), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getValue2)
{
  d_solver->setOption("produce-models", "true");
  Term t = d_tm.mkFalse();
  d_solver->assertFormula(t);
  d_solver->checkSat();
  ASSERT_THROW(d_solver->getValue(t), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getValue3)
{
  d_solver->setOption("produce-models", "true");
  Sort uToIntSort = d_tm.mkFunctionSort({d_uninterpreted}, d_int);
  Sort intPredSort = d_tm.mkFunctionSort({d_int}, d_bool);
  std::vector<Term> unsat_core;

  Term x = d_tm.mkConst(d_uninterpreted, "x");
  Term y = d_tm.mkConst(d_uninterpreted, "y");
  Term z = d_tm.mkConst(d_uninterpreted, "z");
  Term f = d_tm.mkConst(uToIntSort, "f");
  Term p = d_tm.mkConst(intPredSort, "p");
  Term zero = d_tm.mkInteger(0);
  Term one = d_tm.mkInteger(1);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  Term f_y = d_tm.mkTerm(Kind::APPLY_UF, {f, y});
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_y});
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  Term p_f_y = d_tm.mkTerm(Kind::APPLY_UF, {p, f_y});

  d_solver->assertFormula(d_tm.mkTerm(Kind::LEQ, {zero, f_x}));
  d_solver->assertFormula(d_tm.mkTerm(Kind::LEQ, {zero, f_y}));
  d_solver->assertFormula(d_tm.mkTerm(Kind::LEQ, {sum, one}));
  d_solver->assertFormula(p_0.notTerm());
  d_solver->assertFormula(p_f_y);
  ASSERT_TRUE(d_solver->checkSat().isSat());
  ASSERT_NO_THROW(d_solver->getValue(x));
  ASSERT_NO_THROW(d_solver->getValue(y));
  ASSERT_NO_THROW(d_solver->getValue(z));
  ASSERT_NO_THROW(d_solver->getValue(sum));
  ASSERT_NO_THROW(d_solver->getValue(p_f_y));

  std::vector<Term> a;
  ASSERT_NO_THROW(a.emplace_back(d_solver->getValue(x)));
  ASSERT_NO_THROW(a.emplace_back(d_solver->getValue(y)));
  ASSERT_NO_THROW(a.emplace_back(d_solver->getValue(z)));
  std::vector<Term> b;
  ASSERT_NO_THROW(b = d_solver->getValue({x, y, z}));
  ASSERT_EQ(a, b);

  ASSERT_THROW(Solver(d_tm).getValue(x), Ava6ApiException);
  {
    Solver slv(d_tm);
    slv.setOption("produce-models", "true");
    ASSERT_THROW(slv.getValue(x), Ava6ApiException);
  }
  {
    Solver slv(d_tm);
    slv.setOption("produce-models", "true");
    slv.checkSat();
    ASSERT_NO_THROW(slv.getValue(x));
  }

  TermManager tm;
  Solver slv(tm);
  slv.setOption("produce-models", "true");
  slv.checkSat();
  ASSERT_THROW(slv.getValue(d_tm.mkConst(d_bool, "x")), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getModelDomainElements)
{
  d_solver->setOption("produce-models", "true");
  Term x = d_tm.mkConst(d_uninterpreted, "x");
  Term y = d_tm.mkConst(d_uninterpreted, "y");
  Term z = d_tm.mkConst(d_uninterpreted, "z");
  Term f = d_tm.mkTerm(Kind::DISTINCT, {x, y, z});
  d_solver->assertFormula(f);
  d_solver->checkSat();
  auto elems = d_solver->getModelDomainElements(d_uninterpreted);
  ASSERT_TRUE(elems.size() >= 3);
  ASSERT_THROW(d_solver->getModelDomainElements(d_int), Ava6ApiException);

  TermManager tm;
  Solver slv(tm);
  slv.setOption("produce-models", "true");
  slv.checkSat();
  ASSERT_THROW(slv.getModelDomainElements(d_uninterpreted), Ava6ApiException);
}




TEST_F(TestApiBlackSolver, getModel)
{
  d_solver->setOption("produce-models", "true");
  Term x = d_tm.mkConst(d_uninterpreted, "x");
  Term y = d_tm.mkConst(d_uninterpreted, "y");
  Term z = d_tm.mkConst(d_uninterpreted, "z");
  Term f = d_tm.mkTerm(Kind::NOT, {d_tm.mkTerm(Kind::EQUAL, {x, y})});
  d_solver->assertFormula(f);
  d_solver->checkSat();
  std::vector<Sort> sorts{d_uninterpreted};
  std::vector<Term> terms{x, y};
  ASSERT_NO_THROW(d_solver->getModel(sorts, terms));
  Term null;
  terms.push_back(null);
  ASSERT_THROW(d_solver->getModel(sorts, terms), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getModel2)
{
  d_solver->setOption("produce-models", "true");
  std::vector<Sort> sorts;
  std::vector<Term> terms;
  ASSERT_THROW(d_solver->getModel(sorts, terms), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, getModel3)
{
  d_solver->setOption("produce-models", "true");
  std::vector<Sort> sorts;
  std::vector<Term> terms;
  d_solver->checkSat();
  ASSERT_NO_THROW(d_solver->getModel(sorts, terms));
  sorts.push_back(d_int);
  ASSERT_THROW(d_solver->getModel(sorts, terms), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, push1)
{
  d_solver->setOption("incremental", "true");
  ASSERT_NO_THROW(d_solver->push(1));
  ASSERT_THROW(d_solver->setOption("incremental", "false"), Ava6ApiException);
  ASSERT_THROW(d_solver->setOption("incremental", "true"), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, push2)
{
  d_solver->setOption("incremental", "false");
  ASSERT_THROW(d_solver->push(1), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, pop1)
{
  d_solver->setOption("incremental", "false");
  ASSERT_THROW(d_solver->pop(1), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, pop2)
{
  d_solver->setOption("incremental", "true");
  ASSERT_THROW(d_solver->pop(1), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, pop3)
{
  d_solver->setOption("incremental", "true");
  ASSERT_NO_THROW(d_solver->push(1));
  ASSERT_NO_THROW(d_solver->pop(1));
  ASSERT_THROW(d_solver->pop(1), Ava6ApiException);
}


TEST_F(TestApiBlackSolver, getInstantiations)
{
  Term p = d_solver->declareFun("p", {d_int}, d_bool);
  Term x = d_tm.mkVar(d_int, "x");
  Term bvl = d_tm.mkTerm(Kind::VARIABLE_LIST, {x});
  Term app = d_tm.mkTerm(Kind::APPLY_UF, {p, x});
  Term q = d_tm.mkTerm(Kind::FORALL, {bvl, app});
  d_solver->assertFormula(q);
  Term five = d_tm.mkInteger(5);
  Term app2 = d_tm.mkTerm(Kind::NOT, {d_tm.mkTerm(Kind::APPLY_UF, {p, five})});
  d_solver->assertFormula(app2);
  ASSERT_THROW(d_solver->getInstantiations(), Ava6ApiException);
  d_solver->checkSat();
  ASSERT_NO_THROW(d_solver->getInstantiations());
}

TEST_F(TestApiBlackSolver, setInfo)
{
  ASSERT_THROW(d_solver->setInfo("ava6-lagic", "QF_BV"), Ava6ApiException);
  ASSERT_THROW(d_solver->setInfo("cvc2-logic", "QF_BV"), Ava6ApiException);
  ASSERT_THROW(d_solver->setInfo("ava6-logic", "asdf"), Ava6ApiException);

  ASSERT_NO_THROW(d_solver->setInfo("source", "asdf"));
  ASSERT_NO_THROW(d_solver->setInfo("category", "asdf"));
  ASSERT_NO_THROW(d_solver->setInfo("difficulty", "asdf"));
  ASSERT_NO_THROW(d_solver->setInfo("filename", "asdf"));
  ASSERT_NO_THROW(d_solver->setInfo("license", "asdf"));
  ASSERT_NO_THROW(d_solver->setInfo("name", "asdf"));
  ASSERT_NO_THROW(d_solver->setInfo("notes", "asdf"));

  ASSERT_NO_THROW(d_solver->setInfo("smt-lib-version", "2"));
  ASSERT_NO_THROW(d_solver->setInfo("smt-lib-version", "2.0"));
  ASSERT_NO_THROW(d_solver->setInfo("smt-lib-version", "2.5"));
  ASSERT_NO_THROW(d_solver->setInfo("smt-lib-version", "2.6"));
  ASSERT_THROW(d_solver->setInfo("smt-lib-version", ".0"), Ava6ApiException);

  ASSERT_NO_THROW(d_solver->setInfo("status", "sat"));
  ASSERT_NO_THROW(d_solver->setInfo("status", "unsat"));
  ASSERT_NO_THROW(d_solver->setInfo("status", "unknown"));
  ASSERT_THROW(d_solver->setInfo("status", "asdf"), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, setLogic)
{
  ASSERT_NO_THROW(d_solver->setLogic("AUFLIRA"));
  ASSERT_THROW(d_solver->setLogic("AF_BV"), Ava6ApiException);
  d_solver->assertFormula(d_tm.mkTrue());
  ASSERT_THROW(d_solver->setLogic("AUFLIRA"), Ava6ApiException);
}

TEST_F(TestApiBlackSolver, isLogicSet)
{
  ASSERT_FALSE(d_solver->isLogicSet());
  ASSERT_NO_THROW(d_solver->setLogic("QF_BV"));
  ASSERT_TRUE(d_solver->isLogicSet());
}

TEST_F(TestApiBlackSolver, getLogic)
{
  ASSERT_THROW(d_solver->getLogic(), Ava6ApiException);
  ASSERT_NO_THROW(d_solver->setLogic("QF_BV"));
  ASSERT_EQ(d_solver->getLogic(), "QF_BV");
}

TEST_F(TestApiBlackSolver, setOption)
{
  ASSERT_THROW(d_solver->setOption("bv-sat-solver", "cadical"),
               Ava6ApiUnsupportedException);
  ASSERT_THROW(d_solver->setOption("bv-sat-solver", "1"), Ava6ApiException);
  d_solver->assertFormula(d_tm.mkTrue());
  ASSERT_THROW(d_solver->setOption("bv-sat-solver", "cadical"),
               Ava6ApiException);
}

TEST_F(TestApiBlackSolver, resetAssertions)
{
  d_solver->setOption("incremental", "true");

  Sort bvSort = d_tm.mkBitVectorSort(4);
  Term one = d_tm.mkBitVector(4, 1);
  Term x = d_tm.mkConst(bvSort, "x");
  Term ule = d_tm.mkTerm(Kind::BITVECTOR_ULE, {x, one});
  Term srem = d_tm.mkTerm(Kind::BITVECTOR_SREM, {one, x});
  d_solver->push(4);
  Term slt = d_tm.mkTerm(Kind::BITVECTOR_SLT, {srem, one});
  d_solver->resetAssertions();
  d_solver->checkSatAssuming({slt, ule});
}


TEST_F(TestApiBlackSolver, tupleProject)
{
  std::vector<Term> elements = {
      d_tm.mkBoolean(true),
      d_tm.mkInteger(3),
      d_tm.mkString("C"),
      d_tm.mkTerm(Kind::SET_SINGLETON, {d_tm.mkString("Z")})};

  Term tuple = d_tm.mkTuple(elements);

  std::vector<uint32_t> indices1 = {};
  std::vector<uint32_t> indices2 = {0};
  std::vector<uint32_t> indices3 = {0, 1};
  std::vector<uint32_t> indices4 = {0, 0, 2, 2, 3, 3, 0};
  std::vector<uint32_t> indices5 = {4};
  std::vector<uint32_t> indices6 = {0, 4};

  ASSERT_NO_THROW(
      d_tm.mkTerm(d_tm.mkOp(Kind::TUPLE_PROJECT, indices1), {tuple}));
  ASSERT_NO_THROW(
      d_tm.mkTerm(d_tm.mkOp(Kind::TUPLE_PROJECT, indices2), {tuple}));
  ASSERT_NO_THROW(
      d_tm.mkTerm(d_tm.mkOp(Kind::TUPLE_PROJECT, indices3), {tuple}));
  ASSERT_NO_THROW(
      d_tm.mkTerm(d_tm.mkOp(Kind::TUPLE_PROJECT, indices4), {tuple}));

  ASSERT_THROW(d_tm.mkTerm(d_tm.mkOp(Kind::TUPLE_PROJECT, indices5), {tuple}),
               Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(d_tm.mkOp(Kind::TUPLE_PROJECT, indices6), {tuple}),
               Ava6ApiException);

  std::vector<uint32_t> indices = {0, 3, 2, 0, 1, 2};

  Op op = d_tm.mkOp(Kind::TUPLE_PROJECT, indices);
  Term projection = d_tm.mkTerm(op, {tuple});

  Datatype datatype = tuple.getSort().getDatatype();
  DatatypeConstructor constructor = datatype[0];

  for (size_t i = 0; i < indices.size(); i++)
  {
    Term selectorTerm = constructor[indices[i]].getTerm();
    Term selectedTerm =
        d_tm.mkTerm(Kind::APPLY_SELECTOR, {selectorTerm, tuple});
    Term simplifiedTerm = d_solver->simplify(selectedTerm);
    ASSERT_EQ(elements[indices[i]], simplifiedTerm);
  }

  ASSERT_EQ(
      "((_ tuple.project 0 3 2 0 1 2) (tuple true 3 \"C\" (set.singleton "
      "\"Z\")))",
      projection.toString());
}

TEST_F(TestApiBlackSolver, output)
{
  ASSERT_THROW(d_solver->isOutputOn("foo-invalid"), Ava6ApiException);
  ASSERT_THROW(d_solver->getOutput("foo-invalid"), Ava6ApiException);
  ASSERT_FALSE(d_solver->isOutputOn("inst"));
  ASSERT_EQ(null_os.rdbuf(), d_solver->getOutput("inst").rdbuf());
  d_solver->setOption("output", "inst");
  ASSERT_TRUE(d_solver->isOutputOn("inst"));
  ASSERT_NE(null_os.rdbuf(), d_solver->getOutput("inst").rdbuf());
}

TEST_F(TestApiBlackSolver, getDatatypeArity)
{
  DatatypeConstructorDecl ctor1 = d_tm.mkDatatypeConstructorDecl("_x21");
  DatatypeConstructorDecl ctor2 = d_tm.mkDatatypeConstructorDecl("_x31");
  Sort s3 = d_solver->declareDatatype(std::string("_x17"), {ctor1, ctor2});
  ASSERT_EQ(s3.getDatatypeArity(), 0);
}


class PluginUnsat : public Plugin
{
 public:
  PluginUnsat(TermManager& tm) : Plugin(tm), d_tm(tm) {}
  virtual ~PluginUnsat() {}
  std::vector<Term> check() override
  {
    std::vector<Term> lemmas;
    // add the "false" lemma.
    Term flem = d_tm.mkBoolean(false);
    lemmas.push_back(flem);
    return lemmas;
  }
  std::string getName() override { return "PluginUnsat"; }

 private:
  /** Reference to the term manager */
  TermManager& d_tm;
};

TEST_F(TestApiBlackSolver, pluginUnsat)
{
  PluginUnsat pu(d_tm);
  d_solver->addPlugin(pu);
  ASSERT_TRUE(pu.getName() == "PluginUnsat");
  // should be unsat since the plugin above asserts "false" as a lemma
  ASSERT_TRUE(d_solver->checkSat().isUnsat());
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


TEST_F(TestApiBlackSolver, verticalBars)
{
  Term a = d_solver->declareFun("|a |", {}, d_real);
  ASSERT_EQ("|a |", a.toString());
}

TEST_F(TestApiBlackSolver, getVersion)
{
  std::cout << d_solver->getVersion() << std::endl;
}

TEST_F(TestApiBlackSolver, multipleSolvers)
{
  Term function1, function2, value1, value2, definedFunction;
  Term zero;
  {
    Solver s1(d_tm);
    s1.setLogic("ALL");
    s1.setOption("produce-models", "true");
    function1 = s1.declareFun("f1", {}, d_int);
    Term x = d_tm.mkVar(d_int, "x");
    zero = d_tm.mkInteger(0);
    definedFunction = s1.defineFun("f", {x}, d_int, zero);
    s1.assertFormula(function1.eqTerm(zero));
    s1.checkSat();
    value1 = s1.getValue(function1);
  }
  ASSERT_EQ(zero, value1);
  {
    Solver s2(d_tm);
    s2.setLogic("ALL");
    s2.setOption("produce-models", "true");
    function2 = s2.declareFun("function2", {}, d_int);
    s2.assertFormula(function2.eqTerm(value1));
    s2.checkSat();
    value2 = s2.getValue(function2);
  }
  ASSERT_EQ(value1, value2);
  {
    Solver s3(d_tm);
    s3.setLogic("ALL");
    s3.setOption("produce-models", "true");
    function2 = s3.declareFun("function3", {}, d_int);
    Term apply = d_tm.mkTerm(Kind::APPLY_UF, {definedFunction, zero});
    s3.assertFormula(function2.eqTerm(apply));
    s3.checkSat();
    Term value3 = s3.getValue(function2);
    ASSERT_EQ(value1, value3);
  }
}

#ifdef AVA6_USE_COCOA


#endif  // AVA6_USE_COCOA

}  // namespace test
}  // namespace ava6::internal
