/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of the Term class.
 */

#include "test_api.h"

namespace ava6::internal {

namespace test {

class TestApiBlackTerm : public TestApi
{
};

TEST_F(TestApiBlackTerm, equalHash)
{
  Sort uSort = d_tm.mkUninterpretedSort("u");
  Term x = d_tm.mkVar(uSort, "x");
  Term y = d_tm.mkVar(uSort, "y");
  Term z;

  ASSERT_TRUE(x == x);
  ASSERT_FALSE(x != x);
  ASSERT_FALSE(x == y);
  ASSERT_TRUE(x != y);
  ASSERT_FALSE((x == z));
  ASSERT_TRUE(x != z);

  ASSERT_EQ(std::hash<Term>{}(x), std::hash<Term>{}(x));
  ASSERT_NE(std::hash<Term>{}(x), std::hash<Term>{}(y));
  (void)std::hash<Term>{}(Term());
}

TEST_F(TestApiBlackTerm, getId)
{
  Term n;
  ASSERT_THROW(n.getId(), Ava6ApiException);
  Term x = d_tm.mkVar(d_tm.getIntegerSort(), "x");
  ASSERT_NO_THROW(x.getId());
  Term y = x;
  ASSERT_EQ(x.getId(), y.getId());

  Term z = d_tm.mkVar(d_tm.getIntegerSort(), "z");
  ASSERT_NE(x.getId(), z.getId());
}

TEST_F(TestApiBlackTerm, getKind)
{
  Sort uSort = d_tm.mkUninterpretedSort("u");
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({uSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  Term n;
  ASSERT_THROW(n.getKind(), Ava6ApiException);
  Term x = d_tm.mkVar(uSort, "x");
  ASSERT_NO_THROW(x.getKind());
  Term y = d_tm.mkVar(uSort, "y");
  ASSERT_NO_THROW(y.getKind());

  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_NO_THROW(f.getKind());
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_NO_THROW(p.getKind());

  Term zero = d_tm.mkInteger(0);
  ASSERT_NO_THROW(zero.getKind());

  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_NO_THROW(f_x.getKind());
  Term f_y = d_tm.mkTerm(Kind::APPLY_UF, {f, y});
  ASSERT_NO_THROW(f_y.getKind());
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_y});
  ASSERT_NO_THROW(sum.getKind());
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.getKind());
  Term p_f_y = d_tm.mkTerm(Kind::APPLY_UF, {p, f_y});
  ASSERT_NO_THROW(p_f_y.getKind());

  // Sequence kinds do not exist internally, test that the API properly
  // converts them back.
  Sort seqSort = d_tm.mkSequenceSort(intSort);
  Term s = d_tm.mkConst(seqSort, "s");
  Term ss = d_tm.mkTerm(Kind::SEQ_CONCAT, {s, s});
  ASSERT_EQ(ss.getKind(), Kind::SEQ_CONCAT);
}

TEST_F(TestApiBlackTerm, getSort)
{
  Sort bvSort = d_tm.mkBitVectorSort(8);
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({bvSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  Term n;
  ASSERT_THROW(n.getSort(), Ava6ApiException);
  Term x = d_tm.mkVar(bvSort, "x");
  ASSERT_NO_THROW(x.getSort());
  ASSERT_EQ(x.getSort(), bvSort);
  Term y = d_tm.mkVar(bvSort, "y");
  ASSERT_NO_THROW(y.getSort());
  ASSERT_EQ(y.getSort(), bvSort);

  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_NO_THROW(f.getSort());
  ASSERT_EQ(f.getSort(), funSort1);
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_NO_THROW(p.getSort());
  ASSERT_EQ(p.getSort(), funSort2);

  Term zero = d_tm.mkInteger(0);
  ASSERT_NO_THROW(zero.getSort());
  ASSERT_EQ(zero.getSort(), intSort);

  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_NO_THROW(f_x.getSort());
  ASSERT_EQ(f_x.getSort(), intSort);
  Term f_y = d_tm.mkTerm(Kind::APPLY_UF, {f, y});
  ASSERT_NO_THROW(f_y.getSort());
  ASSERT_EQ(f_y.getSort(), intSort);
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_y});
  ASSERT_NO_THROW(sum.getSort());
  ASSERT_EQ(sum.getSort(), intSort);
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.getSort());
  ASSERT_EQ(p_0.getSort(), boolSort);
  Term p_f_y = d_tm.mkTerm(Kind::APPLY_UF, {p, f_y});
  ASSERT_NO_THROW(p_f_y.getSort());
  ASSERT_EQ(p_f_y.getSort(), boolSort);
}

TEST_F(TestApiBlackTerm, getOp)
{
  Sort intsort = d_tm.getIntegerSort();
  Sort bvsort = d_tm.mkBitVectorSort(8);
  Sort arrsort = d_tm.mkArraySort(bvsort, intsort);
  Sort funsort = d_tm.mkFunctionSort({intsort}, bvsort);

  Term x = d_tm.mkConst(intsort, "x");
  Term a = d_tm.mkConst(arrsort, "a");
  Term b = d_tm.mkConst(bvsort, "b");

  ASSERT_FALSE(x.hasOp());
  ASSERT_THROW(x.getOp(), Ava6ApiException);

  Term ab = d_tm.mkTerm(Kind::SELECT, {a, b});
  Op ext = d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {4, 0});
  Term extb = d_tm.mkTerm(ext, {b});

  ASSERT_TRUE(ab.hasOp());
  ASSERT_FALSE(ab.getOp().isIndexed());
  // can compare directly to a Kind (will invoke Op constructor)
  ASSERT_TRUE(extb.hasOp());
  ASSERT_TRUE(extb.getOp().isIndexed());
  ASSERT_EQ(extb.getOp(), ext);

  Op bit = d_tm.mkOp(Kind::BITVECTOR_BIT, {4});
  Term bitb = d_tm.mkTerm(bit, {b});
  ASSERT_EQ(bitb.getKind(), Kind::BITVECTOR_BIT);
  ASSERT_TRUE(bitb.hasOp());
  ASSERT_EQ(bitb.getOp(), bit);
  ASSERT_TRUE(bitb.getOp().isIndexed());
  ASSERT_EQ(bit.getNumIndices(), 1);
  ASSERT_EQ(bit[0], d_tm.mkInteger(4));

  Term f = d_tm.mkConst(funsort, "f");
  Term fx = d_tm.mkTerm(Kind::APPLY_UF, {f, x});

  ASSERT_FALSE(f.hasOp());
  ASSERT_THROW(f.getOp(), Ava6ApiException);
  ASSERT_TRUE(fx.hasOp());
  std::vector<Term> children(fx.begin(), fx.end());
  // testing rebuild from op and children
  ASSERT_EQ(fx, d_tm.mkTerm(fx.getOp(), children));

  // Test Datatypes Ops
  Sort sort = d_tm.mkParamSort("T");
  DatatypeDecl listDecl = d_tm.mkDatatypeDecl("paramlist", {sort});
  DatatypeConstructorDecl cons = d_tm.mkDatatypeConstructorDecl("cons");
  DatatypeConstructorDecl nil = d_tm.mkDatatypeConstructorDecl("nil");
  cons.addSelector("head", sort);
  cons.addSelectorSelf("tail");
  listDecl.addConstructor(cons);
  listDecl.addConstructor(nil);
  Sort listSort = d_tm.mkDatatypeSort(listDecl);
  Sort intListSort =
      listSort.instantiate(std::vector<Sort>{d_tm.getIntegerSort()});
  Term c = d_tm.mkConst(intListSort, "c");
  Datatype list = listSort.getDatatype();
  // list datatype constructor and selector operator terms
  Term consOpTerm = list.getConstructor("cons").getTerm();
  Term nilOpTerm = list.getConstructor("nil").getTerm();
  Term headOpTerm = list["cons"].getSelector("head").getTerm();
  Term tailOpTerm = list["cons"].getSelector("tail").getTerm();

  Term nilTerm = d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR, {nilOpTerm});
  Term consTerm = d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR,
                              {consOpTerm, d_tm.mkInteger(0), nilTerm});
  Term headTerm = d_tm.mkTerm(Kind::APPLY_SELECTOR, {headOpTerm, consTerm});
  Term tailTerm = d_tm.mkTerm(Kind::APPLY_SELECTOR, {tailOpTerm, consTerm});

  ASSERT_FALSE(c.hasOp());
  ASSERT_TRUE(nilTerm.hasOp());
  ASSERT_TRUE(consTerm.hasOp());
  ASSERT_TRUE(headTerm.hasOp());
  ASSERT_TRUE(tailTerm.hasOp());

  // Test rebuilding
  children.clear();
  children.insert(children.begin(), headTerm.begin(), headTerm.end());
  ASSERT_EQ(headTerm, d_tm.mkTerm(headTerm.getOp(), children));
}

TEST_F(TestApiBlackTerm, hasGetSymbol)
{
  Term n;
  Term t = d_tm.mkBoolean(true);
  Term c = d_tm.mkConst(d_tm.getBooleanSort(), "|\\|");

  ASSERT_THROW(n.hasSymbol(), Ava6ApiException);
  ASSERT_FALSE(t.hasSymbol());
  ASSERT_TRUE(c.hasSymbol());

  ASSERT_THROW(n.getSymbol(), Ava6ApiException);
  ASSERT_THROW(t.getSymbol(), Ava6ApiException);
  ASSERT_EQ(c.getSymbol(), "|\\|");
}

TEST_F(TestApiBlackTerm, isNull)
{
  Term x;
  ASSERT_TRUE(x.isNull());
  x = d_tm.mkVar(d_tm.mkBitVectorSort(4), "x");
  ASSERT_FALSE(x.isNull());
}

TEST_F(TestApiBlackTerm, notTerm)
{
  Sort bvSort = d_tm.mkBitVectorSort(8);
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({bvSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  ASSERT_THROW(Term().notTerm(), Ava6ApiException);
  Term b = d_tm.mkTrue();
  ASSERT_NO_THROW(b.notTerm());
  Term x = d_tm.mkVar(d_tm.mkBitVectorSort(8), "x");
  ASSERT_THROW(x.notTerm(), Ava6ApiException);
  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_THROW(f.notTerm(), Ava6ApiException);
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_THROW(p.notTerm(), Ava6ApiException);
  Term zero = d_tm.mkInteger(0);
  ASSERT_THROW(zero.notTerm(), Ava6ApiException);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_THROW(f_x.notTerm(), Ava6ApiException);
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_x});
  ASSERT_THROW(sum.notTerm(), Ava6ApiException);
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.notTerm());
  Term p_f_x = d_tm.mkTerm(Kind::APPLY_UF, {p, f_x});
  ASSERT_NO_THROW(p_f_x.notTerm());
}

TEST_F(TestApiBlackTerm, andTerm)
{
  Sort bvSort = d_tm.mkBitVectorSort(8);
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({bvSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  Term b = d_tm.mkTrue();
  ASSERT_THROW(Term().andTerm(b), Ava6ApiException);
  ASSERT_THROW(b.andTerm(Term()), Ava6ApiException);
  ASSERT_NO_THROW(b.andTerm(b));
  Term x = d_tm.mkVar(d_tm.mkBitVectorSort(8), "x");
  ASSERT_THROW(x.andTerm(b), Ava6ApiException);
  ASSERT_THROW(x.andTerm(x), Ava6ApiException);
  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_THROW(f.andTerm(b), Ava6ApiException);
  ASSERT_THROW(f.andTerm(x), Ava6ApiException);
  ASSERT_THROW(f.andTerm(f), Ava6ApiException);
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_THROW(p.andTerm(b), Ava6ApiException);
  ASSERT_THROW(p.andTerm(x), Ava6ApiException);
  ASSERT_THROW(p.andTerm(f), Ava6ApiException);
  ASSERT_THROW(p.andTerm(p), Ava6ApiException);
  Term zero = d_tm.mkInteger(0);
  ASSERT_THROW(zero.andTerm(b), Ava6ApiException);
  ASSERT_THROW(zero.andTerm(x), Ava6ApiException);
  ASSERT_THROW(zero.andTerm(f), Ava6ApiException);
  ASSERT_THROW(zero.andTerm(p), Ava6ApiException);
  ASSERT_THROW(zero.andTerm(zero), Ava6ApiException);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_THROW(f_x.andTerm(b), Ava6ApiException);
  ASSERT_THROW(f_x.andTerm(x), Ava6ApiException);
  ASSERT_THROW(f_x.andTerm(f), Ava6ApiException);
  ASSERT_THROW(f_x.andTerm(p), Ava6ApiException);
  ASSERT_THROW(f_x.andTerm(zero), Ava6ApiException);
  ASSERT_THROW(f_x.andTerm(f_x), Ava6ApiException);
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_x});
  ASSERT_THROW(sum.andTerm(b), Ava6ApiException);
  ASSERT_THROW(sum.andTerm(x), Ava6ApiException);
  ASSERT_THROW(sum.andTerm(f), Ava6ApiException);
  ASSERT_THROW(sum.andTerm(p), Ava6ApiException);
  ASSERT_THROW(sum.andTerm(zero), Ava6ApiException);
  ASSERT_THROW(sum.andTerm(f_x), Ava6ApiException);
  ASSERT_THROW(sum.andTerm(sum), Ava6ApiException);
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.andTerm(b));
  ASSERT_THROW(p_0.andTerm(x), Ava6ApiException);
  ASSERT_THROW(p_0.andTerm(f), Ava6ApiException);
  ASSERT_THROW(p_0.andTerm(p), Ava6ApiException);
  ASSERT_THROW(p_0.andTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_0.andTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_0.andTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_0.andTerm(p_0));
  Term p_f_x = d_tm.mkTerm(Kind::APPLY_UF, {p, f_x});
  ASSERT_NO_THROW(p_f_x.andTerm(b));
  ASSERT_THROW(p_f_x.andTerm(x), Ava6ApiException);
  ASSERT_THROW(p_f_x.andTerm(f), Ava6ApiException);
  ASSERT_THROW(p_f_x.andTerm(p), Ava6ApiException);
  ASSERT_THROW(p_f_x.andTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_f_x.andTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_f_x.andTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_f_x.andTerm(p_0));
  ASSERT_NO_THROW(p_f_x.andTerm(p_f_x));
}

TEST_F(TestApiBlackTerm, orTerm)
{
  Sort bvSort = d_tm.mkBitVectorSort(8);
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({bvSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  Term b = d_tm.mkTrue();
  ASSERT_THROW(Term().orTerm(b), Ava6ApiException);
  ASSERT_THROW(b.orTerm(Term()), Ava6ApiException);
  ASSERT_NO_THROW(b.orTerm(b));
  Term x = d_tm.mkVar(d_tm.mkBitVectorSort(8), "x");
  ASSERT_THROW(x.orTerm(b), Ava6ApiException);
  ASSERT_THROW(x.orTerm(x), Ava6ApiException);
  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_THROW(f.orTerm(b), Ava6ApiException);
  ASSERT_THROW(f.orTerm(x), Ava6ApiException);
  ASSERT_THROW(f.orTerm(f), Ava6ApiException);
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_THROW(p.orTerm(b), Ava6ApiException);
  ASSERT_THROW(p.orTerm(x), Ava6ApiException);
  ASSERT_THROW(p.orTerm(f), Ava6ApiException);
  ASSERT_THROW(p.orTerm(p), Ava6ApiException);
  Term zero = d_tm.mkInteger(0);
  ASSERT_THROW(zero.orTerm(b), Ava6ApiException);
  ASSERT_THROW(zero.orTerm(x), Ava6ApiException);
  ASSERT_THROW(zero.orTerm(f), Ava6ApiException);
  ASSERT_THROW(zero.orTerm(p), Ava6ApiException);
  ASSERT_THROW(zero.orTerm(zero), Ava6ApiException);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_THROW(f_x.orTerm(b), Ava6ApiException);
  ASSERT_THROW(f_x.orTerm(x), Ava6ApiException);
  ASSERT_THROW(f_x.orTerm(f), Ava6ApiException);
  ASSERT_THROW(f_x.orTerm(p), Ava6ApiException);
  ASSERT_THROW(f_x.orTerm(zero), Ava6ApiException);
  ASSERT_THROW(f_x.orTerm(f_x), Ava6ApiException);
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_x});
  ASSERT_THROW(sum.orTerm(b), Ava6ApiException);
  ASSERT_THROW(sum.orTerm(x), Ava6ApiException);
  ASSERT_THROW(sum.orTerm(f), Ava6ApiException);
  ASSERT_THROW(sum.orTerm(p), Ava6ApiException);
  ASSERT_THROW(sum.orTerm(zero), Ava6ApiException);
  ASSERT_THROW(sum.orTerm(f_x), Ava6ApiException);
  ASSERT_THROW(sum.orTerm(sum), Ava6ApiException);
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.orTerm(b));
  ASSERT_THROW(p_0.orTerm(x), Ava6ApiException);
  ASSERT_THROW(p_0.orTerm(f), Ava6ApiException);
  ASSERT_THROW(p_0.orTerm(p), Ava6ApiException);
  ASSERT_THROW(p_0.orTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_0.orTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_0.orTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_0.orTerm(p_0));
  Term p_f_x = d_tm.mkTerm(Kind::APPLY_UF, {p, f_x});
  ASSERT_NO_THROW(p_f_x.orTerm(b));
  ASSERT_THROW(p_f_x.orTerm(x), Ava6ApiException);
  ASSERT_THROW(p_f_x.orTerm(f), Ava6ApiException);
  ASSERT_THROW(p_f_x.orTerm(p), Ava6ApiException);
  ASSERT_THROW(p_f_x.orTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_f_x.orTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_f_x.orTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_f_x.orTerm(p_0));
  ASSERT_NO_THROW(p_f_x.orTerm(p_f_x));
}

TEST_F(TestApiBlackTerm, xorTerm)
{
  Sort bvSort = d_tm.mkBitVectorSort(8);
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({bvSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  Term b = d_tm.mkTrue();
  ASSERT_THROW(Term().xorTerm(b), Ava6ApiException);
  ASSERT_THROW(b.xorTerm(Term()), Ava6ApiException);
  ASSERT_NO_THROW(b.xorTerm(b));
  Term x = d_tm.mkVar(d_tm.mkBitVectorSort(8), "x");
  ASSERT_THROW(x.xorTerm(b), Ava6ApiException);
  ASSERT_THROW(x.xorTerm(x), Ava6ApiException);
  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_THROW(f.xorTerm(b), Ava6ApiException);
  ASSERT_THROW(f.xorTerm(x), Ava6ApiException);
  ASSERT_THROW(f.xorTerm(f), Ava6ApiException);
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_THROW(p.xorTerm(b), Ava6ApiException);
  ASSERT_THROW(p.xorTerm(x), Ava6ApiException);
  ASSERT_THROW(p.xorTerm(f), Ava6ApiException);
  ASSERT_THROW(p.xorTerm(p), Ava6ApiException);
  Term zero = d_tm.mkInteger(0);
  ASSERT_THROW(zero.xorTerm(b), Ava6ApiException);
  ASSERT_THROW(zero.xorTerm(x), Ava6ApiException);
  ASSERT_THROW(zero.xorTerm(f), Ava6ApiException);
  ASSERT_THROW(zero.xorTerm(p), Ava6ApiException);
  ASSERT_THROW(zero.xorTerm(zero), Ava6ApiException);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_THROW(f_x.xorTerm(b), Ava6ApiException);
  ASSERT_THROW(f_x.xorTerm(x), Ava6ApiException);
  ASSERT_THROW(f_x.xorTerm(f), Ava6ApiException);
  ASSERT_THROW(f_x.xorTerm(p), Ava6ApiException);
  ASSERT_THROW(f_x.xorTerm(zero), Ava6ApiException);
  ASSERT_THROW(f_x.xorTerm(f_x), Ava6ApiException);
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_x});
  ASSERT_THROW(sum.xorTerm(b), Ava6ApiException);
  ASSERT_THROW(sum.xorTerm(x), Ava6ApiException);
  ASSERT_THROW(sum.xorTerm(f), Ava6ApiException);
  ASSERT_THROW(sum.xorTerm(p), Ava6ApiException);
  ASSERT_THROW(sum.xorTerm(zero), Ava6ApiException);
  ASSERT_THROW(sum.xorTerm(f_x), Ava6ApiException);
  ASSERT_THROW(sum.xorTerm(sum), Ava6ApiException);
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.xorTerm(b));
  ASSERT_THROW(p_0.xorTerm(x), Ava6ApiException);
  ASSERT_THROW(p_0.xorTerm(f), Ava6ApiException);
  ASSERT_THROW(p_0.xorTerm(p), Ava6ApiException);
  ASSERT_THROW(p_0.xorTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_0.xorTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_0.xorTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_0.xorTerm(p_0));
  Term p_f_x = d_tm.mkTerm(Kind::APPLY_UF, {p, f_x});
  ASSERT_NO_THROW(p_f_x.xorTerm(b));
  ASSERT_THROW(p_f_x.xorTerm(x), Ava6ApiException);
  ASSERT_THROW(p_f_x.xorTerm(f), Ava6ApiException);
  ASSERT_THROW(p_f_x.xorTerm(p), Ava6ApiException);
  ASSERT_THROW(p_f_x.xorTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_f_x.xorTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_f_x.xorTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_f_x.xorTerm(p_0));
  ASSERT_NO_THROW(p_f_x.xorTerm(p_f_x));
}

TEST_F(TestApiBlackTerm, eqTerm)
{
  Sort bvSort = d_tm.mkBitVectorSort(8);
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({bvSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  Term b = d_tm.mkTrue();
  ASSERT_THROW(Term().eqTerm(b), Ava6ApiException);
  ASSERT_THROW(b.eqTerm(Term()), Ava6ApiException);
  ASSERT_NO_THROW(b.eqTerm(b));
  Term x = d_tm.mkVar(d_tm.mkBitVectorSort(8), "x");
  ASSERT_THROW(x.eqTerm(b), Ava6ApiException);
  ASSERT_NO_THROW(x.eqTerm(x));
  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_THROW(f.eqTerm(b), Ava6ApiException);
  ASSERT_THROW(f.eqTerm(x), Ava6ApiException);
  ASSERT_NO_THROW(f.eqTerm(f));
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_THROW(p.eqTerm(b), Ava6ApiException);
  ASSERT_THROW(p.eqTerm(x), Ava6ApiException);
  ASSERT_THROW(p.eqTerm(f), Ava6ApiException);
  ASSERT_NO_THROW(p.eqTerm(p));
  Term zero = d_tm.mkInteger(0);
  ASSERT_THROW(zero.eqTerm(b), Ava6ApiException);
  ASSERT_THROW(zero.eqTerm(x), Ava6ApiException);
  ASSERT_THROW(zero.eqTerm(f), Ava6ApiException);
  ASSERT_THROW(zero.eqTerm(p), Ava6ApiException);
  ASSERT_NO_THROW(zero.eqTerm(zero));
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_THROW(f_x.eqTerm(b), Ava6ApiException);
  ASSERT_THROW(f_x.eqTerm(x), Ava6ApiException);
  ASSERT_THROW(f_x.eqTerm(f), Ava6ApiException);
  ASSERT_THROW(f_x.eqTerm(p), Ava6ApiException);
  ASSERT_NO_THROW(f_x.eqTerm(zero));
  ASSERT_NO_THROW(f_x.eqTerm(f_x));
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_x});
  ASSERT_THROW(sum.eqTerm(b), Ava6ApiException);
  ASSERT_THROW(sum.eqTerm(x), Ava6ApiException);
  ASSERT_THROW(sum.eqTerm(f), Ava6ApiException);
  ASSERT_THROW(sum.eqTerm(p), Ava6ApiException);
  ASSERT_NO_THROW(sum.eqTerm(zero));
  ASSERT_NO_THROW(sum.eqTerm(f_x));
  ASSERT_NO_THROW(sum.eqTerm(sum));
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.eqTerm(b));
  ASSERT_THROW(p_0.eqTerm(x), Ava6ApiException);
  ASSERT_THROW(p_0.eqTerm(f), Ava6ApiException);
  ASSERT_THROW(p_0.eqTerm(p), Ava6ApiException);
  ASSERT_THROW(p_0.eqTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_0.eqTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_0.eqTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_0.eqTerm(p_0));
  Term p_f_x = d_tm.mkTerm(Kind::APPLY_UF, {p, f_x});
  ASSERT_NO_THROW(p_f_x.eqTerm(b));
  ASSERT_THROW(p_f_x.eqTerm(x), Ava6ApiException);
  ASSERT_THROW(p_f_x.eqTerm(f), Ava6ApiException);
  ASSERT_THROW(p_f_x.eqTerm(p), Ava6ApiException);
  ASSERT_THROW(p_f_x.eqTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_f_x.eqTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_f_x.eqTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_f_x.eqTerm(p_0));
  ASSERT_NO_THROW(p_f_x.eqTerm(p_f_x));
}

TEST_F(TestApiBlackTerm, impTerm)
{
  Sort bvSort = d_tm.mkBitVectorSort(8);
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({bvSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  Term b = d_tm.mkTrue();
  ASSERT_THROW(Term().impTerm(b), Ava6ApiException);
  ASSERT_THROW(b.impTerm(Term()), Ava6ApiException);
  ASSERT_NO_THROW(b.impTerm(b));
  Term x = d_tm.mkVar(d_tm.mkBitVectorSort(8), "x");
  ASSERT_THROW(x.impTerm(b), Ava6ApiException);
  ASSERT_THROW(x.impTerm(x), Ava6ApiException);
  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_THROW(f.impTerm(b), Ava6ApiException);
  ASSERT_THROW(f.impTerm(x), Ava6ApiException);
  ASSERT_THROW(f.impTerm(f), Ava6ApiException);
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_THROW(p.impTerm(b), Ava6ApiException);
  ASSERT_THROW(p.impTerm(x), Ava6ApiException);
  ASSERT_THROW(p.impTerm(f), Ava6ApiException);
  ASSERT_THROW(p.impTerm(p), Ava6ApiException);
  Term zero = d_tm.mkInteger(0);
  ASSERT_THROW(zero.impTerm(b), Ava6ApiException);
  ASSERT_THROW(zero.impTerm(x), Ava6ApiException);
  ASSERT_THROW(zero.impTerm(f), Ava6ApiException);
  ASSERT_THROW(zero.impTerm(p), Ava6ApiException);
  ASSERT_THROW(zero.impTerm(zero), Ava6ApiException);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_THROW(f_x.impTerm(b), Ava6ApiException);
  ASSERT_THROW(f_x.impTerm(x), Ava6ApiException);
  ASSERT_THROW(f_x.impTerm(f), Ava6ApiException);
  ASSERT_THROW(f_x.impTerm(p), Ava6ApiException);
  ASSERT_THROW(f_x.impTerm(zero), Ava6ApiException);
  ASSERT_THROW(f_x.impTerm(f_x), Ava6ApiException);
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_x});
  ASSERT_THROW(sum.impTerm(b), Ava6ApiException);
  ASSERT_THROW(sum.impTerm(x), Ava6ApiException);
  ASSERT_THROW(sum.impTerm(f), Ava6ApiException);
  ASSERT_THROW(sum.impTerm(p), Ava6ApiException);
  ASSERT_THROW(sum.impTerm(zero), Ava6ApiException);
  ASSERT_THROW(sum.impTerm(f_x), Ava6ApiException);
  ASSERT_THROW(sum.impTerm(sum), Ava6ApiException);
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.impTerm(b));
  ASSERT_THROW(p_0.impTerm(x), Ava6ApiException);
  ASSERT_THROW(p_0.impTerm(f), Ava6ApiException);
  ASSERT_THROW(p_0.impTerm(p), Ava6ApiException);
  ASSERT_THROW(p_0.impTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_0.impTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_0.impTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_0.impTerm(p_0));
  Term p_f_x = d_tm.mkTerm(Kind::APPLY_UF, {p, f_x});
  ASSERT_NO_THROW(p_f_x.impTerm(b));
  ASSERT_THROW(p_f_x.impTerm(x), Ava6ApiException);
  ASSERT_THROW(p_f_x.impTerm(f), Ava6ApiException);
  ASSERT_THROW(p_f_x.impTerm(p), Ava6ApiException);
  ASSERT_THROW(p_f_x.impTerm(zero), Ava6ApiException);
  ASSERT_THROW(p_f_x.impTerm(f_x), Ava6ApiException);
  ASSERT_THROW(p_f_x.impTerm(sum), Ava6ApiException);
  ASSERT_NO_THROW(p_f_x.impTerm(p_0));
  ASSERT_NO_THROW(p_f_x.impTerm(p_f_x));
}

TEST_F(TestApiBlackTerm, iteTerm)
{
  Sort bvSort = d_tm.mkBitVectorSort(8);
  Sort intSort = d_tm.getIntegerSort();
  Sort boolSort = d_tm.getBooleanSort();
  Sort funSort1 = d_tm.mkFunctionSort({bvSort}, intSort);
  Sort funSort2 = d_tm.mkFunctionSort({intSort}, boolSort);

  Term b = d_tm.mkTrue();
  ASSERT_THROW(Term().iteTerm(b, b), Ava6ApiException);
  ASSERT_THROW(b.iteTerm(Term(), b), Ava6ApiException);
  ASSERT_THROW(b.iteTerm(b, Term()), Ava6ApiException);
  ASSERT_NO_THROW(b.iteTerm(b, b));
  Term x = d_tm.mkVar(d_tm.mkBitVectorSort(8), "x");
  ASSERT_NO_THROW(b.iteTerm(x, x));
  ASSERT_NO_THROW(b.iteTerm(b, b));
  ASSERT_THROW(b.iteTerm(x, b), Ava6ApiException);
  ASSERT_THROW(x.iteTerm(x, x), Ava6ApiException);
  ASSERT_THROW(x.iteTerm(x, b), Ava6ApiException);
  Term f = d_tm.mkVar(funSort1, "f");
  ASSERT_THROW(f.iteTerm(b, b), Ava6ApiException);
  ASSERT_THROW(x.iteTerm(b, x), Ava6ApiException);
  Term p = d_tm.mkVar(funSort2, "p");
  ASSERT_THROW(p.iteTerm(b, b), Ava6ApiException);
  ASSERT_THROW(p.iteTerm(x, b), Ava6ApiException);
  Term zero = d_tm.mkInteger(0);
  ASSERT_THROW(zero.iteTerm(x, x), Ava6ApiException);
  ASSERT_THROW(zero.iteTerm(x, b), Ava6ApiException);
  Term f_x = d_tm.mkTerm(Kind::APPLY_UF, {f, x});
  ASSERT_THROW(f_x.iteTerm(b, b), Ava6ApiException);
  ASSERT_THROW(f_x.iteTerm(b, x), Ava6ApiException);
  Term sum = d_tm.mkTerm(Kind::ADD, {f_x, f_x});
  ASSERT_THROW(sum.iteTerm(x, x), Ava6ApiException);
  ASSERT_THROW(sum.iteTerm(b, x), Ava6ApiException);
  Term p_0 = d_tm.mkTerm(Kind::APPLY_UF, {p, zero});
  ASSERT_NO_THROW(p_0.iteTerm(b, b));
  ASSERT_NO_THROW(p_0.iteTerm(x, x));
  ASSERT_THROW(p_0.iteTerm(x, b), Ava6ApiException);
  Term p_f_x = d_tm.mkTerm(Kind::APPLY_UF, {p, f_x});
  ASSERT_NO_THROW(p_f_x.iteTerm(b, b));
  ASSERT_NO_THROW(p_f_x.iteTerm(x, x));
  ASSERT_THROW(p_f_x.iteTerm(x, b), Ava6ApiException);
}

TEST_F(TestApiBlackTerm, termAssignment)
{
  Term t1 = d_tm.mkInteger(1);
  Term t2 = t1;
  t2 = d_tm.mkInteger(2);
  ASSERT_EQ(t1, d_tm.mkInteger(1));
}

TEST_F(TestApiBlackTerm, termCompare)
{
  Term t1 = d_tm.mkInteger(1);
  Term t2 = d_tm.mkTerm(Kind::ADD, {d_tm.mkInteger(2), d_tm.mkInteger(2)});
  Term t3 = d_tm.mkTerm(Kind::ADD, {d_tm.mkInteger(2), d_tm.mkInteger(2)});
  ASSERT_TRUE(t2 >= t3);
  ASSERT_TRUE(t2 <= t3);
  ASSERT_TRUE((t1 > t2) != (t1 < t2));
  ASSERT_TRUE((t1 > t2 || t1 == t2) == (t1 >= t2));
}

TEST_F(TestApiBlackTerm, termChildren)
{
  // simple term 2+3
  Term two = d_tm.mkInteger(2);
  Term t1 = d_tm.mkTerm(Kind::ADD, {two, d_tm.mkInteger(3)});
  ASSERT_EQ(t1[0], two);
  ASSERT_EQ(t1.getNumChildren(), 2);
  Term tnull;
  ASSERT_THROW(tnull.getNumChildren(), Ava6ApiException);

  Term::const_iterator it;
  it = t1.begin();
  ASSERT_TRUE((*it).isIntegerValue());
  it++;
  ASSERT_TRUE((*it).isIntegerValue());
  ++it;
  ASSERT_EQ(it, t1.end());

  // apply term f(2)
  Sort intSort = d_tm.getIntegerSort();
  Sort fsort = d_tm.mkFunctionSort({intSort}, intSort);
  Term f = d_tm.mkConst(fsort, "f");
  Term t2 = d_tm.mkTerm(Kind::APPLY_UF, {f, two});
  // due to our higher-order view of terms, we treat f as a child of APPLY_UF
  ASSERT_EQ(t2.getNumChildren(), 2);
  ASSERT_EQ(t2[0], f);
  ASSERT_EQ(t2[1], two);
  ASSERT_THROW(tnull[0], Ava6ApiException);
}

TEST_F(TestApiBlackTerm, getInteger)
{
  Term int1 = d_tm.mkInteger("-18446744073709551616");
  Term int2 = d_tm.mkInteger("-18446744073709551615");
  Term int3 = d_tm.mkInteger("-4294967296");
  Term int4 = d_tm.mkInteger("-4294967295");
  Term int5 = d_tm.mkInteger("-10");
  Term int6 = d_tm.mkInteger("0");
  Term int7 = d_tm.mkInteger("10");
  Term int8 = d_tm.mkInteger("4294967295");
  Term int9 = d_tm.mkInteger("4294967296");
  Term int10 = d_tm.mkInteger("18446744073709551615");
  Term int11 = d_tm.mkInteger("18446744073709551616");
  Term int12 = d_tm.mkInteger("-0");

  ASSERT_THROW(d_tm.mkInteger(""), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("-"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("-1-"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("0.0"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("-0.1"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("012"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("0000"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("-01"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("-00"), Ava6ApiException);

  ASSERT_TRUE(!int1.isInt32Value() && !int1.isUInt32Value()
              && !int1.isInt64Value() && !int1.isUInt64Value()
              && int1.isIntegerValue());
  ASSERT_EQ(int1.getIntegerValue(), "-18446744073709551616");
  ASSERT_TRUE(int1.getRealOrIntegerValueSign() == -1);
  ASSERT_TRUE(!int2.isInt32Value() && !int2.isUInt32Value()
              && !int2.isInt64Value() && !int2.isUInt64Value()
              && int2.isIntegerValue());
  ASSERT_EQ(int2.getIntegerValue(), "-18446744073709551615");
  ASSERT_TRUE(!int3.isInt32Value() && !int3.isUInt32Value()
              && int3.isInt64Value() && !int3.isUInt64Value()
              && int3.isIntegerValue());
  ASSERT_EQ(int3.getInt64Value(), -4294967296);
  ASSERT_EQ(int3.getIntegerValue(), "-4294967296");
  ASSERT_TRUE(!int4.isInt32Value() && !int4.isUInt32Value()
              && int4.isInt64Value() && !int4.isUInt64Value()
              && int4.isIntegerValue());
  ASSERT_EQ(int4.getInt64Value(), -4294967295);
  ASSERT_EQ(int4.getIntegerValue(), "-4294967295");
  ASSERT_TRUE(int5.isInt32Value() && !int5.isUInt32Value()
              && int5.isInt64Value() && !int5.isUInt64Value()
              && int5.isIntegerValue());
  ASSERT_EQ(int5.getInt32Value(), -10);
  ASSERT_EQ(int5.getInt64Value(), -10);
  ASSERT_EQ(int5.getIntegerValue(), "-10");
  ASSERT_TRUE(int6.isInt32Value() && int6.isUInt32Value() && int6.isInt64Value()
              && int6.isUInt64Value() && int6.isIntegerValue());
  ASSERT_EQ(int6.getInt32Value(), 0);
  ASSERT_EQ(int6.getUInt32Value(), 0);
  ASSERT_EQ(int6.getInt64Value(), 0);
  ASSERT_EQ(int6.getUInt64Value(), 0);
  ASSERT_EQ(int6.getIntegerValue(), "0");
  ASSERT_TRUE(int6.getRealOrIntegerValueSign() == 0);
  ASSERT_TRUE(int7.isInt32Value() && int7.isUInt32Value() && int7.isInt64Value()
              && int7.isUInt64Value() && int7.isIntegerValue());
  ASSERT_EQ(int7.getInt32Value(), 10);
  ASSERT_EQ(int7.getUInt32Value(), 10);
  ASSERT_EQ(int7.getInt64Value(), 10);
  ASSERT_EQ(int7.getUInt64Value(), 10);
  ASSERT_EQ(int7.getIntegerValue(), "10");
  ASSERT_TRUE(int7.getRealOrIntegerValueSign() == 1);
  ASSERT_TRUE(!int8.isInt32Value() && int8.isUInt32Value()
              && int8.isInt64Value() && int8.isUInt64Value()
              && int8.isIntegerValue());
  ASSERT_EQ(int8.getUInt32Value(), 4294967295);
  ASSERT_EQ(int8.getInt64Value(), 4294967295);
  ASSERT_EQ(int8.getUInt64Value(), 4294967295);
  ASSERT_EQ(int8.getIntegerValue(), "4294967295");
  ASSERT_TRUE(!int9.isInt32Value() && !int9.isUInt32Value()
              && int9.isInt64Value() && int9.isUInt64Value()
              && int9.isIntegerValue());
  ASSERT_EQ(int9.getInt64Value(), 4294967296);
  ASSERT_EQ(int9.getUInt64Value(), 4294967296);
  ASSERT_EQ(int9.getIntegerValue(), "4294967296");
  ASSERT_TRUE(!int10.isInt32Value() && !int10.isUInt32Value()
              && !int10.isInt64Value() && int10.isUInt64Value()
              && int10.isIntegerValue());
  ASSERT_EQ(int10.getUInt64Value(), 18446744073709551615ull);
  ASSERT_EQ(int10.getIntegerValue(), "18446744073709551615");
  ASSERT_TRUE(!int11.isInt32Value() && !int11.isUInt32Value()
              && !int11.isInt64Value() && !int11.isUInt64Value()
              && int11.isIntegerValue());
  ASSERT_EQ(int11.getIntegerValue(), "18446744073709551616");
}

TEST_F(TestApiBlackTerm, getString)
{
  Term s1 = d_tm.mkString("abcde");
  ASSERT_TRUE(s1.isStringValue());
  ASSERT_EQ(s1.getU32StringValue(), U"abcde");
}

TEST_F(TestApiBlackTerm, getReal)
{
  Term real1 = d_tm.mkReal("0");
  Term real2 = d_tm.mkReal(".0");
  Term real3 = d_tm.mkReal("-17");
  Term real4 = d_tm.mkReal("-3/5");
  Term real5 = d_tm.mkReal("12.7");
  Term real6 = d_tm.mkReal("1/4294967297");
  Term real7 = d_tm.mkReal("4294967297");
  Term real8 = d_tm.mkReal("1/18446744073709551617");
  Term real9 = d_tm.mkReal("18446744073709551617");
  Term real10 = d_tm.mkReal("2343.2343");

  ASSERT_TRUE(real1.isRealValue() && real1.isReal64Value()
              && real1.isReal32Value());
  ASSERT_TRUE(real2.isRealValue() && real2.isReal64Value()
              && real2.isReal32Value());
  ASSERT_TRUE(real3.isRealValue() && real3.isReal64Value()
              && real3.isReal32Value());
  ASSERT_TRUE(real4.isRealValue() && real4.isReal64Value()
              && real4.isReal32Value());
  ASSERT_TRUE(real5.isRealValue() && real5.isReal64Value()
              && real5.isReal32Value());
  ASSERT_TRUE(real6.isRealValue() && real6.isReal64Value());
  ASSERT_TRUE(real7.isRealValue() && real7.isReal64Value());
  ASSERT_TRUE(real8.isRealValue());
  ASSERT_TRUE(real9.isRealValue());
  ASSERT_TRUE(real10.isRealValue());

  ASSERT_EQ((std::pair<int32_t, uint32_t>(0, 1)), real1.getReal32Value());
  ASSERT_EQ((std::pair<int64_t, uint64_t>(0, 1)), real1.getReal64Value());
  ASSERT_EQ("0/1", real1.getRealValue());

  ASSERT_EQ((std::pair<int32_t, uint32_t>(0, 1)), real2.getReal32Value());
  ASSERT_EQ((std::pair<int64_t, uint64_t>(0, 1)), real2.getReal64Value());
  ASSERT_EQ("0/1", real2.getRealValue());

  ASSERT_EQ((std::pair<int32_t, uint32_t>(-17, 1)), real3.getReal32Value());
  ASSERT_EQ((std::pair<int64_t, uint64_t>(-17, 1)), real3.getReal64Value());
  ASSERT_EQ("-17/1", real3.getRealValue());

  ASSERT_EQ((std::pair<int32_t, uint32_t>(-3, 5)), real4.getReal32Value());
  ASSERT_EQ((std::pair<int64_t, uint64_t>(-3, 5)), real4.getReal64Value());
  ASSERT_EQ("-3/5", real4.getRealValue());

  ASSERT_EQ((std::pair<int32_t, uint32_t>(127, 10)), real5.getReal32Value());
  ASSERT_EQ((std::pair<int64_t, uint64_t>(127, 10)), real5.getReal64Value());
  ASSERT_EQ("127/10", real5.getRealValue());

  ASSERT_EQ((std::pair<int64_t, uint64_t>(1, 4294967297)),
            real6.getReal64Value());
  ASSERT_EQ("1/4294967297", real6.getRealValue());

  ASSERT_EQ((std::pair<int64_t, uint64_t>(4294967297, 1)),
            real7.getReal64Value());
  ASSERT_EQ("4294967297/1", real7.getRealValue());

  ASSERT_EQ("1/18446744073709551617", real8.getRealValue());

  ASSERT_EQ("18446744073709551617/1", real9.getRealValue());

  ASSERT_EQ("23432343/10000", real10.getRealValue());

  ASSERT_THROW(d_tm.mkReal("1/0"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkReal("2/0000"), Ava6ApiException);
}



TEST_F(TestApiBlackTerm, getBoolean)
{
  Term b1 = d_tm.mkBoolean(true);
  Term b2 = d_tm.mkBoolean(false);

  ASSERT_TRUE(b1.isBooleanValue());
  ASSERT_TRUE(b2.isBooleanValue());
  ASSERT_TRUE(b1.getBooleanValue());
  ASSERT_FALSE(b2.getBooleanValue());
}

TEST_F(TestApiBlackTerm, getBitVector)
{
  Term b1 = d_tm.mkBitVector(8, 15);
  Term b2 = d_tm.mkBitVector(8, "00001111", 2);
  Term b3 = d_tm.mkBitVector(8, "15", 10);
  Term b4 = d_tm.mkBitVector(8, "0f", 16);
  Term b5 = d_tm.mkBitVector(9, "00001111", 2);
  Term b6 = d_tm.mkBitVector(9, "15", 10);
  Term b7 = d_tm.mkBitVector(9, "0f", 16);

  ASSERT_TRUE(b1.isBitVectorValue());
  ASSERT_TRUE(b2.isBitVectorValue());
  ASSERT_TRUE(b3.isBitVectorValue());
  ASSERT_TRUE(b4.isBitVectorValue());
  ASSERT_TRUE(b5.isBitVectorValue());
  ASSERT_TRUE(b6.isBitVectorValue());
  ASSERT_TRUE(b7.isBitVectorValue());

  ASSERT_EQ("00001111", b1.getBitVectorValue(2));
  ASSERT_EQ("15", b1.getBitVectorValue(10));
  ASSERT_EQ("f", b1.getBitVectorValue(16));
  ASSERT_EQ("00001111", b2.getBitVectorValue(2));
  ASSERT_EQ("15", b2.getBitVectorValue(10));
  ASSERT_EQ("f", b2.getBitVectorValue(16));
  ASSERT_EQ("00001111", b3.getBitVectorValue(2));
  ASSERT_EQ("15", b3.getBitVectorValue(10));
  ASSERT_EQ("f", b3.getBitVectorValue(16));
  ASSERT_EQ("00001111", b4.getBitVectorValue(2));
  ASSERT_EQ("15", b4.getBitVectorValue(10));
  ASSERT_EQ("f", b4.getBitVectorValue(16));
  ASSERT_EQ("000001111", b5.getBitVectorValue(2));
  ASSERT_EQ("15", b5.getBitVectorValue(10));
  ASSERT_EQ("f", b5.getBitVectorValue(16));
  ASSERT_EQ("000001111", b6.getBitVectorValue(2));
  ASSERT_EQ("15", b6.getBitVectorValue(10));
  ASSERT_EQ("f", b6.getBitVectorValue(16));
  ASSERT_EQ("000001111", b7.getBitVectorValue(2));
  ASSERT_EQ("15", b7.getBitVectorValue(10));
  ASSERT_EQ("f", b7.getBitVectorValue(16));
}





TEST_F(TestApiBlackTerm, getUninterpretedSortValue)
{
  d_solver->setOption("produce-models", "true");
  Sort uSort = d_tm.mkUninterpretedSort("u");
  Term x = d_tm.mkConst(uSort, "x");
  Term y = d_tm.mkConst(uSort, "y");
  d_solver->assertFormula(d_tm.mkTerm(Kind::EQUAL, {x, y}));
  ASSERT_TRUE(d_solver->checkSat().isSat());
  Term vx = d_solver->getValue(x);
  Term vy = d_solver->getValue(y);
  ASSERT_TRUE(vx.isUninterpretedSortValue());
  ASSERT_TRUE(vy.isUninterpretedSortValue());
  ASSERT_EQ(vx.getUninterpretedSortValue(), vy.getUninterpretedSortValue());
}





TEST_F(TestApiBlackTerm, getTuple)
{
  Term t1 = d_tm.mkInteger(15);
  Term t2 = d_tm.mkReal(17, 25);
  Term t3 = d_tm.mkString("abc");

  Term tup = d_tm.mkTuple({t1, t2, t3});

  ASSERT_TRUE(tup.isTupleValue());
  ASSERT_EQ(std::vector<Term>({t1, t2, t3}), tup.getTupleValue());
}



TEST_F(TestApiBlackTerm, getSet)
{
  Sort s = d_tm.mkSetSort(d_tm.getIntegerSort());

  Term i1 = d_tm.mkInteger(5);
  Term i2 = d_tm.mkInteger(7);

  Term s1 = d_tm.mkEmptySet(s);
  Term s2 = d_tm.mkTerm(Kind::SET_SINGLETON, {i1});
  Term s3 = d_tm.mkTerm(Kind::SET_SINGLETON, {i1});
  Term s4 = d_tm.mkTerm(Kind::SET_SINGLETON, {i2});
  Term s5 = d_tm.mkTerm(Kind::SET_UNION,
                        {s2, d_tm.mkTerm(Kind::SET_UNION, {s3, s4})});

  ASSERT_TRUE(s1.isSetValue());
  ASSERT_TRUE(s2.isSetValue());
  ASSERT_TRUE(s3.isSetValue());
  ASSERT_TRUE(s4.isSetValue());
  ASSERT_FALSE(s5.isSetValue());
  s5 = d_solver->simplify(s5);
  ASSERT_TRUE(s5.isSetValue());

  ASSERT_EQ(std::set<Term>({}), s1.getSetValue());
  ASSERT_EQ(std::set<Term>({i1}), s2.getSetValue());
  ASSERT_EQ(std::set<Term>({i1}), s3.getSetValue());
  ASSERT_EQ(std::set<Term>({i2}), s4.getSetValue());
  ASSERT_EQ(std::set<Term>({i1, i2}), s5.getSetValue());
}

TEST_F(TestApiBlackTerm, getSequence)
{
  Sort s = d_tm.mkSequenceSort(d_tm.getIntegerSort());

  Term i1 = d_tm.mkInteger(5);
  Term i2 = d_tm.mkInteger(7);

  Term s1 = d_tm.mkEmptySequence(s);
  Term s2 = d_tm.mkTerm(Kind::SEQ_UNIT, {i1});
  Term s3 = d_tm.mkTerm(Kind::SEQ_UNIT, {i1});
  Term s4 = d_tm.mkTerm(Kind::SEQ_UNIT, {i2});
  Term s5 = d_tm.mkTerm(Kind::SEQ_CONCAT,
                        {s2, d_tm.mkTerm(Kind::SEQ_CONCAT, {s3, s4})});

  ASSERT_TRUE(s1.isSequenceValue());
  ASSERT_TRUE(!s2.isSequenceValue());
  ASSERT_TRUE(!s3.isSequenceValue());
  ASSERT_TRUE(!s4.isSequenceValue());
  ASSERT_TRUE(!s5.isSequenceValue());

  s2 = d_solver->simplify(s2);
  s3 = d_solver->simplify(s3);
  s4 = d_solver->simplify(s4);
  s5 = d_solver->simplify(s5);

  ASSERT_EQ(std::vector<Term>({}), s1.getSequenceValue());
  ASSERT_EQ(std::vector<Term>({i1}), s2.getSequenceValue());
  ASSERT_EQ(std::vector<Term>({i1}), s3.getSequenceValue());
  ASSERT_EQ(std::vector<Term>({i2}), s4.getSequenceValue());
  ASSERT_EQ(std::vector<Term>({i1, i1, i2}), s5.getSequenceValue());
}

TEST_F(TestApiBlackTerm, substitute)
{
  Term x = d_tm.mkConst(d_tm.getIntegerSort(), "x");
  Term one = d_tm.mkInteger(1);
  Term ttrue = d_tm.mkTrue();
  Term xpx = d_tm.mkTerm(Kind::ADD, {x, x});
  Term onepone = d_tm.mkTerm(Kind::ADD, {one, one});

  ASSERT_EQ(xpx.substitute(x, one), onepone);
  ASSERT_EQ(onepone.substitute(one, x), xpx);
  // incorrect due to type
  ASSERT_THROW(xpx.substitute(one, ttrue), Ava6ApiException);

  // simultaneous substitution
  Term y = d_tm.mkConst(d_tm.getIntegerSort(), "y");
  Term xpy = d_tm.mkTerm(Kind::ADD, {x, y});
  Term xpone = d_tm.mkTerm(Kind::ADD, {y, one});
  std::vector<Term> es = {x, y};
  std::vector<Term> rs = {y, one};
  ASSERT_EQ(xpy.substitute(es, rs), xpone);

  // incorrect substitution due to arity
  rs.pop_back();
  ASSERT_THROW(xpy.substitute(es, rs), Ava6ApiException);

  // incorrect substitution due to types
  rs.push_back(ttrue);
  ASSERT_THROW(xpy.substitute(es, rs), Ava6ApiException);

  // null cannot substitute
  Term tnull;
  ASSERT_THROW(tnull.substitute(one, x), Ava6ApiException);
  ASSERT_THROW(xpx.substitute(tnull, x), Ava6ApiException);
  ASSERT_THROW(xpx.substitute(x, tnull), Ava6ApiException);
  rs.pop_back();
  rs.push_back(tnull);
  ASSERT_THROW(xpy.substitute(es, rs), Ava6ApiException);
  es.clear();
  rs.clear();
  es.push_back(x);
  rs.push_back(y);
  ASSERT_THROW(tnull.substitute(es, rs), Ava6ApiException);
  es.push_back(tnull);
  rs.push_back(one);
  ASSERT_THROW(xpx.substitute(es, rs), Ava6ApiException);
}



TEST_F(TestApiBlackTerm, getSequenceValue)
{
  Sort realsort = d_tm.getRealSort();
  Sort seqsort = d_tm.mkSequenceSort(realsort);
  Term s = d_tm.mkEmptySequence(seqsort);

  ASSERT_EQ(s.getKind(), Kind::CONST_SEQUENCE);
  // empty sequence has zero elements
  std::vector<Term> cs = s.getSequenceValue();
  ASSERT_TRUE(cs.empty());

  // A seq.unit app is not a constant sequence (regardless of whether it is
  // applied to a constant).
  Term su = d_tm.mkTerm(Kind::SEQ_UNIT, {d_tm.mkReal(1)});
  ASSERT_THROW(su.getSequenceValue(), Ava6ApiException);
}





TEST_F(TestApiBlackTerm, getSkolem)
{
  // ordinary variables are not skolems
  Term x = d_tm.mkConst(d_tm.getIntegerSort(), "x");
  ASSERT_FALSE(x.isSkolem());
  ASSERT_THROW(x.getSkolemId(), Ava6ApiException);
  ASSERT_THROW(x.getSkolemIndices(), Ava6ApiException);
}

TEST_F(TestApiBlackTerm, termScopedToString)
{
  Sort intsort = d_tm.getIntegerSort();
  Term x = d_tm.mkConst(intsort, "x");
  ASSERT_EQ(x.toString(), "x");
  ASSERT_EQ(x.toString(), "x");
}

TEST_F(TestApiBlackTerm, toString)
{
  ASSERT_NO_THROW(Term().toString());

  Sort intsort = d_tm.getIntegerSort();
  Term x = d_tm.mkConst(intsort, "x");
  std::stringstream ss;

  ss << std::vector<Term>{x, x};
  ss << std::set<Term>{x, x};
  ss << std::unordered_set<Term>{x, x};
}
}  // namespace test
}  // namespace ava6::internal
