/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of the TermManager class of the  C++ API.
 */

#include <gtest/gtest.h>

#include <cmath>

#include "test_api.h"

namespace ava6::internal::test {

class TestApiBlackTermManager : public TestApi
{
};

TEST_F(TestApiBlackTermManager, getBooleanSort)
{
  ASSERT_NO_THROW(d_tm.getBooleanSort());
}

TEST_F(TestApiBlackTermManager, getIntegerSort)
{
  ASSERT_NO_THROW(d_tm.getIntegerSort());
}

TEST_F(TestApiBlackTermManager, getRealSort)
{
  ASSERT_NO_THROW(d_tm.getRealSort());
}

TEST_F(TestApiBlackTermManager, getRegExpSort)
{
  ASSERT_NO_THROW(d_tm.getRegExpSort());
}

TEST_F(TestApiBlackTermManager, getStringSort)
{
  ASSERT_NO_THROW(d_tm.getStringSort());
}


TEST_F(TestApiBlackTermManager, mkBitVectorSort)
{
  ASSERT_NO_THROW(d_tm.mkBitVectorSort(32));
  ASSERT_THROW(d_tm.mkBitVectorSort(0), Ava6ApiException);
}


TEST_F(TestApiBlackTermManager, mkDatatypeSort)
{
  {
    DatatypeDecl dtypeSpec = d_tm.mkDatatypeDecl("list");
    DatatypeConstructorDecl cons = d_tm.mkDatatypeConstructorDecl("cons");
    cons.addSelector("head", d_tm.getIntegerSort());
    dtypeSpec.addConstructor(cons);
    DatatypeConstructorDecl nil = d_tm.mkDatatypeConstructorDecl("nil");
    dtypeSpec.addConstructor(nil);
    ASSERT_NO_THROW(d_tm.mkDatatypeSort(dtypeSpec));

    ASSERT_THROW(d_tm.mkDatatypeSort(dtypeSpec), Ava6ApiException);
    ASSERT_THROW(d_tm.mkDatatypeSort(dtypeSpec), Ava6ApiException);
  }

  DatatypeDecl throwsDtypeSpec = d_tm.mkDatatypeDecl("list");
  ASSERT_THROW(d_tm.mkDatatypeSort(throwsDtypeSpec), Ava6ApiException);

  {
    TermManager tm;
    DatatypeDecl dtypeSpec = tm.mkDatatypeDecl("list");
    DatatypeConstructorDecl cons = tm.mkDatatypeConstructorDecl("cons");
    cons.addSelector("head", tm.getIntegerSort());
    dtypeSpec.addConstructor(cons);
    DatatypeConstructorDecl nil = tm.mkDatatypeConstructorDecl("nil");
    dtypeSpec.addConstructor(nil);
    ASSERT_THROW(d_tm.mkDatatypeSort(dtypeSpec), Ava6ApiException);
  }
}

TEST_F(TestApiBlackTermManager, mkDatatypeSorts)
{
  {
    DatatypeDecl dtypeSpec1 = d_tm.mkDatatypeDecl("list1");
    DatatypeConstructorDecl cons1 = d_tm.mkDatatypeConstructorDecl("cons1");
    cons1.addSelector("head1", d_tm.getIntegerSort());
    dtypeSpec1.addConstructor(cons1);
    DatatypeConstructorDecl nil1 = d_tm.mkDatatypeConstructorDecl("nil1");
    dtypeSpec1.addConstructor(nil1);
    DatatypeDecl dtypeSpec2 = d_tm.mkDatatypeDecl("list2");
    DatatypeConstructorDecl cons2 = d_tm.mkDatatypeConstructorDecl("cons2");
    cons2.addSelector("head2", d_tm.getIntegerSort());
    dtypeSpec2.addConstructor(cons2);
    DatatypeConstructorDecl nil2 = d_tm.mkDatatypeConstructorDecl("nil2");
    dtypeSpec2.addConstructor(nil2);
    std::vector<DatatypeDecl> decls = {dtypeSpec1, dtypeSpec2};
    ASSERT_NO_THROW(d_tm.mkDatatypeSorts(decls));

    ASSERT_THROW(d_tm.mkDatatypeSorts(decls), Ava6ApiException);
    ASSERT_THROW(d_tm.mkDatatypeSorts(decls), Ava6ApiException);
  }

  DatatypeDecl throwsDtypeSpec = d_tm.mkDatatypeDecl("list");
  std::vector<DatatypeDecl> throwsDecls = {throwsDtypeSpec};
  ASSERT_THROW(d_tm.mkDatatypeSorts(throwsDecls), Ava6ApiException);

  /* with unresolved sorts */
  Sort unresList = d_tm.mkUnresolvedDatatypeSort("ulist");
  DatatypeDecl ulist = d_tm.mkDatatypeDecl("ulist");
  DatatypeConstructorDecl ucons = d_tm.mkDatatypeConstructorDecl("ucons");
  ucons.addSelector("car", unresList);
  ucons.addSelector("cdr", unresList);
  ulist.addConstructor(ucons);
  DatatypeConstructorDecl unil = d_tm.mkDatatypeConstructorDecl("unil");
  ulist.addConstructor(unil);
  std::vector<DatatypeDecl> udecls = {ulist};
  ASSERT_NO_THROW(d_tm.mkDatatypeSorts(udecls));

  ASSERT_THROW(d_tm.mkDatatypeSorts(udecls), Ava6ApiException);
  ASSERT_THROW(d_tm.mkDatatypeSorts(udecls), Ava6ApiException);

  /* mutually recursive with unresolved parameterized sorts */
  Sort p0 = d_tm.mkParamSort("p0");
  Sort p1 = d_tm.mkParamSort("p1");
  Sort u0 = d_tm.mkUnresolvedDatatypeSort("dt0", 1);
  Sort u1 = d_tm.mkUnresolvedDatatypeSort("dt1", 1);
  DatatypeDecl dtdecl0 = d_tm.mkDatatypeDecl("dt0", {p0});
  DatatypeDecl dtdecl1 = d_tm.mkDatatypeDecl("dt1", {p1});
  DatatypeConstructorDecl ctordecl0 = d_tm.mkDatatypeConstructorDecl("c0");
  ctordecl0.addSelector("s0", u1.instantiate({p0}));
  DatatypeConstructorDecl ctordecl1 = d_tm.mkDatatypeConstructorDecl("c1");
  ctordecl1.addSelector("s1", u0.instantiate({p1}));
  dtdecl0.addConstructor(ctordecl0);
  dtdecl1.addConstructor(ctordecl1);
  dtdecl1.addConstructor(d_tm.mkDatatypeConstructorDecl("nil"));
  std::vector<Sort> dt_sorts = d_tm.mkDatatypeSorts({dtdecl0, dtdecl1});
  Sort isort1 = dt_sorts[1].instantiate({d_tm.getBooleanSort()});
  Term t1 = d_tm.mkConst(isort1, "t");
  Term t0 =
      d_tm.mkTerm(Kind::APPLY_SELECTOR,
                  {t1.getSort().getDatatype().getSelector("s1").getTerm(), t1});
  ASSERT_EQ(dt_sorts[0].instantiate({d_tm.getBooleanSort()}), t0.getSort());

  {
    TermManager tm;
    DatatypeDecl dtypeSpec1 = tm.mkDatatypeDecl("list1");
    DatatypeConstructorDecl cons1 = tm.mkDatatypeConstructorDecl("cons1");
    cons1.addSelector("head1", tm.getIntegerSort());
    dtypeSpec1.addConstructor(cons1);
    DatatypeConstructorDecl nil1 = tm.mkDatatypeConstructorDecl("nil1");
    dtypeSpec1.addConstructor(nil1);
    DatatypeDecl dtypeSpec2 = tm.mkDatatypeDecl("list2");
    DatatypeConstructorDecl cons2 = tm.mkDatatypeConstructorDecl("cons2");
    cons2.addSelector("head2", tm.getIntegerSort());
    dtypeSpec2.addConstructor(cons2);
    DatatypeConstructorDecl nil2 = tm.mkDatatypeConstructorDecl("nil2");
    dtypeSpec2.addConstructor(nil2);
    std::vector<DatatypeDecl> decls = {dtypeSpec1, dtypeSpec2};
    ASSERT_THROW(d_tm.mkDatatypeSorts(decls), Ava6ApiException);
  }

  /* Note: More tests are in datatype_api_black. */
}

TEST_F(TestApiBlackTermManager, mkFunctionSort)
{
  ASSERT_NO_THROW(d_tm.mkFunctionSort({d_tm.mkUninterpretedSort("u")},
                                      d_tm.getIntegerSort()));
  Sort funSort = d_tm.mkFunctionSort({d_tm.mkUninterpretedSort("u")},
                                     d_tm.getIntegerSort());
  // Function-valued arguments and results are excluded.
  ASSERT_THROW(d_tm.mkFunctionSort({funSort}, d_tm.getIntegerSort()),
               Ava6ApiException);
  ASSERT_THROW(d_tm.mkFunctionSort({d_tm.getIntegerSort()}, funSort),
               Ava6ApiException);
  ASSERT_NO_THROW(d_tm.mkFunctionSort(
      {d_tm.mkUninterpretedSort("u"), d_tm.getIntegerSort()},
      d_tm.getIntegerSort()));
  Sort funSort2 = d_tm.mkFunctionSort({d_tm.mkUninterpretedSort("u")},
                                      d_tm.getIntegerSort());
  // Function-valued arguments and results are excluded.
  ASSERT_NO_THROW(d_tm.mkFunctionSort({funSort2, d_tm.mkUninterpretedSort("u")},
                                      d_tm.getIntegerSort()));
  ASSERT_THROW(
      d_tm.mkFunctionSort(
          {d_tm.getIntegerSort(), d_tm.mkUninterpretedSort("u")}, funSort2),
      Ava6ApiException);

  std::vector<Sort> sorts1 = {
      d_tm.getBooleanSort(), d_tm.getIntegerSort(), d_tm.getIntegerSort()};
  std::vector<Sort> sorts2 = {d_tm.getBooleanSort(), d_tm.getIntegerSort()};
  ASSERT_NO_THROW(d_tm.mkFunctionSort(sorts2, d_tm.getIntegerSort()));
  ASSERT_NO_THROW(d_tm.mkFunctionSort(sorts1, d_tm.getIntegerSort()));

  TermManager tm;
  ASSERT_THROW(tm.mkFunctionSort(sorts2, tm.getIntegerSort()),
               Ava6ApiException);
  ASSERT_THROW(tm.mkFunctionSort({tm.getBooleanSort(), tm.getIntegerSort()},
                                 d_tm.getIntegerSort()),
               Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkParamSort)
{
  ASSERT_NO_THROW(d_tm.mkParamSort("T"));
  ASSERT_NO_THROW(d_tm.mkParamSort(""));
}

TEST_F(TestApiBlackTermManager, mkPredicateSort)
{
  ASSERT_NO_THROW(d_tm.mkPredicateSort({d_tm.getIntegerSort()}));
  ASSERT_THROW(d_tm.mkPredicateSort({}), Ava6ApiException);
  Sort funSort = d_tm.mkFunctionSort({d_tm.mkUninterpretedSort("u")},
                                     d_tm.getIntegerSort());
  // functions as arguments are allowed
  ASSERT_NO_THROW(d_tm.mkPredicateSort({d_tm.getIntegerSort(), funSort}));

  ASSERT_NO_THROW(d_tm.mkPredicateSort({d_tm.getIntegerSort()}));

  TermManager tm;
  ASSERT_THROW(tm.mkPredicateSort({d_tm.getIntegerSort()}), Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkRecordSort)
{
  std::vector<std::pair<std::string, Sort>> fields = {
      std::make_pair("b", d_tm.getBooleanSort()),
      std::make_pair("bv", d_tm.mkBitVectorSort(8)),
      std::make_pair("i", d_tm.getIntegerSort())};
  std::vector<std::pair<std::string, Sort>> empty;
  ASSERT_NO_THROW(d_tm.mkRecordSort(fields));
  ASSERT_NO_THROW(d_tm.mkRecordSort(empty));
  Sort recSort = d_tm.mkRecordSort(fields);
  ASSERT_NO_THROW(recSort.getDatatype());
  ASSERT_NO_THROW(d_tm.mkRecordSort(fields));

  TermManager tm;
  ASSERT_THROW(tm.mkRecordSort({{"b", tm.getBooleanSort()},
                                {"bv", d_tm.mkBitVectorSort(8)},
                                {"i", tm.getIntegerSort()}}),
               Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkSetSort)
{
  ASSERT_NO_THROW(d_tm.mkSetSort(d_tm.getBooleanSort()));
  ASSERT_NO_THROW(d_tm.mkSetSort(d_tm.getIntegerSort()));
  ASSERT_NO_THROW(d_tm.mkSetSort(d_tm.mkBitVectorSort(4)));
  ASSERT_NO_THROW(d_tm.mkSetSort(d_tm.mkBitVectorSort(4)));
  TermManager tm;
  ASSERT_THROW(d_tm.mkSetSort(tm.getBooleanSort()), Ava6ApiException);
}


TEST_F(TestApiBlackTermManager, mkSequenceSort)
{
  ASSERT_NO_THROW(d_tm.mkSequenceSort(d_tm.getBooleanSort()));
  ASSERT_NO_THROW(
      d_tm.mkSequenceSort(d_tm.mkSequenceSort(d_tm.getIntegerSort())));
  ASSERT_NO_THROW(d_tm.mkSequenceSort(d_tm.getIntegerSort()));
  TermManager tm;
  ASSERT_THROW(d_tm.mkSequenceSort(tm.getBooleanSort()), Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkAbstractSort)
{
  ASSERT_NO_THROW(d_tm.mkAbstractSort(SortKind::ARRAY_SORT));
  ASSERT_NO_THROW(d_tm.mkAbstractSort(SortKind::BITVECTOR_SORT));
  ASSERT_NO_THROW(d_tm.mkAbstractSort(SortKind::TUPLE_SORT));
  ASSERT_NO_THROW(d_tm.mkAbstractSort(SortKind::SET_SORT));
  ASSERT_THROW(d_tm.mkAbstractSort(SortKind::BOOLEAN_SORT), Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkUninterpretedSort)
{
  ASSERT_NO_THROW(d_tm.mkUninterpretedSort("u"));
  ASSERT_NO_THROW(d_tm.mkUninterpretedSort(""));
}

TEST_F(TestApiBlackTermManager, mkUnresolvedDatatypeSort)
{
  ASSERT_NO_THROW(d_tm.mkUnresolvedDatatypeSort("u"));
  ASSERT_NO_THROW(d_tm.mkUnresolvedDatatypeSort("u", 1));
  ASSERT_NO_THROW(d_tm.mkUnresolvedDatatypeSort(""));
  ASSERT_NO_THROW(d_tm.mkUnresolvedDatatypeSort("", 1));
}

TEST_F(TestApiBlackTermManager, mkUninterpretedSortConstructorSort)
{
  ASSERT_NO_THROW(d_tm.mkUninterpretedSortConstructorSort(2, "s"));
  ASSERT_NO_THROW(d_tm.mkUninterpretedSortConstructorSort(2, ""));
  ASSERT_THROW(d_tm.mkUninterpretedSortConstructorSort(0), Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkTupleSort)
{
  ASSERT_NO_THROW(d_tm.mkTupleSort({d_tm.getIntegerSort()}));
  Sort funSort = d_tm.mkFunctionSort({d_tm.mkUninterpretedSort("u")},
                                     d_tm.getIntegerSort());
  ASSERT_NO_THROW(d_tm.mkTupleSort({d_tm.getIntegerSort(), funSort}));

  ASSERT_NO_THROW(d_tm.mkTupleSort({d_tm.getIntegerSort()}));
  TermManager tm;
  ASSERT_THROW(d_tm.mkTupleSort({tm.getBooleanSort()}), Ava6ApiException);
}


TEST_F(TestApiBlackTermManager, mkBitVector)
{
  ASSERT_NO_THROW(d_tm.mkBitVector(8, 2));
  ASSERT_NO_THROW(d_tm.mkBitVector(32, 2));
  ASSERT_NO_THROW(d_tm.mkBitVector(8, "-1111111", 2));
  ASSERT_NO_THROW(d_tm.mkBitVector(8, "0101", 2));
  ASSERT_NO_THROW(d_tm.mkBitVector(8, "00000101", 2));
  ASSERT_NO_THROW(d_tm.mkBitVector(8, "-127", 10));
  ASSERT_NO_THROW(d_tm.mkBitVector(8, "255", 10));
  ASSERT_NO_THROW(d_tm.mkBitVector(8, "-7f", 16));
  ASSERT_NO_THROW(d_tm.mkBitVector(8, "a0", 16));

  ASSERT_THROW(d_tm.mkBitVector(0, 2), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(0, "-127", 10), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(0, "a0", 16), Ava6ApiException);

  ASSERT_THROW(d_tm.mkBitVector(8, "", 2), Ava6ApiException);

  ASSERT_THROW(d_tm.mkBitVector(8, "101", 5), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "128", 11), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "a0", 21), Ava6ApiException);

  ASSERT_THROW(d_tm.mkBitVector(8, "-11111111", 2), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "101010101", 2), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "-256", 10), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "257", 10), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "-a0", 16), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "fffff", 16), Ava6ApiException);

  ASSERT_THROW(d_tm.mkBitVector(8, "10201010", 2), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "-25x", 10), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "2x7", 10), Ava6ApiException);
  ASSERT_THROW(d_tm.mkBitVector(8, "fzff", 16), Ava6ApiException);

  ASSERT_EQ(d_tm.mkBitVector(8, "0101", 2), d_tm.mkBitVector(8, "00000101", 2));
  ASSERT_EQ(d_tm.mkBitVector(4, "-1", 2), d_tm.mkBitVector(4, "1111", 2));
  ASSERT_EQ(d_tm.mkBitVector(4, "-1", 16), d_tm.mkBitVector(4, "1111", 2));
  ASSERT_EQ(d_tm.mkBitVector(4, "-1", 10), d_tm.mkBitVector(4, "1111", 2));
  ASSERT_EQ(d_tm.mkBitVector(8, "01010101", 2).toString(), "#b01010101");
  ASSERT_EQ(d_tm.mkBitVector(8, "F", 16).toString(), "#b00001111");
  ASSERT_EQ(d_tm.mkBitVector(8, "-1", 10), d_tm.mkBitVector(8, "FF", 16));
}


TEST_F(TestApiBlackTermManager, mkVar)
{
  Sort boolSort = d_tm.getBooleanSort();
  Sort intSort = d_tm.getIntegerSort();
  Sort funSort = d_tm.mkFunctionSort({intSort}, boolSort);
  ASSERT_NO_THROW(d_tm.mkVar(boolSort));
  ASSERT_NO_THROW(d_tm.mkVar(funSort));
  ASSERT_NO_THROW(d_tm.mkVar(boolSort, std::string("b")));
  ASSERT_NO_THROW(d_tm.mkVar(funSort, ""));
  ASSERT_THROW(d_tm.mkVar(Sort()), Ava6ApiException);
  ASSERT_THROW(d_tm.mkVar(Sort(), "a"), Ava6ApiException);
  ASSERT_NO_THROW(d_tm.mkVar(boolSort, "x"));
  TermManager tm;
  ASSERT_THROW(tm.mkVar(boolSort, "c"), Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkBoolean)
{
  ASSERT_NO_THROW(d_tm.mkBoolean(true));
  ASSERT_NO_THROW(d_tm.mkBoolean(false));
}


TEST_F(TestApiBlackTermManager, mkEmptySet)
{
  Sort s = d_tm.mkSetSort(d_tm.getBooleanSort());
  ASSERT_THROW(d_tm.mkEmptySet(Sort()), Ava6ApiException);
  ASSERT_NO_THROW(d_tm.mkEmptySet(s));
  ASSERT_THROW(d_tm.mkEmptySet(d_tm.getBooleanSort()), Ava6ApiException);
  ASSERT_NO_THROW(d_tm.mkEmptySet(s));
  TermManager tm;
  ASSERT_THROW(tm.mkEmptySet(s), Ava6ApiException);
}


TEST_F(TestApiBlackTermManager, mkEmptySequence)
{
  Sort s = d_tm.mkSequenceSort(d_tm.getBooleanSort());
  ASSERT_NO_THROW(d_tm.mkEmptySequence(s));
  ASSERT_NO_THROW(d_tm.mkEmptySequence(d_tm.getBooleanSort()));
  ASSERT_NO_THROW(d_tm.mkEmptySequence(s));
  TermManager tm;
  ASSERT_THROW(tm.mkEmptySequence(s), Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkFalse)
{
  ASSERT_NO_THROW(d_tm.mkFalse());
  ASSERT_NO_THROW(d_tm.mkFalse());
}


TEST_F(TestApiBlackTermManager, mkOp)
{
  // mkOp(Kind kind, const std::string& arg)
  ASSERT_NO_THROW(d_tm.mkOp(Kind::DIVISIBLE, "2147483648"));
  ASSERT_THROW(d_tm.mkOp(Kind::BITVECTOR_EXTRACT, "asdf"), Ava6ApiException);

  // mkOp(Kind kind, std::vector<uint32_t> args)
  ASSERT_NO_THROW(d_tm.mkOp(Kind::DIVISIBLE, {1}));
  ASSERT_NO_THROW(d_tm.mkOp(Kind::BITVECTOR_ROTATE_LEFT, {1}));
  ASSERT_NO_THROW(d_tm.mkOp(Kind::BITVECTOR_ROTATE_RIGHT, {1}));
  ASSERT_THROW(d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {1}), Ava6ApiException);

  ASSERT_NO_THROW(d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {1, 1}));
  ASSERT_THROW(d_tm.mkOp(Kind::DIVISIBLE, {1, 2}), Ava6ApiException);

  ASSERT_NO_THROW(d_tm.mkOp(Kind::TUPLE_PROJECT, {1, 2, 2}));
}


TEST_F(TestApiBlackTermManager, mkInteger)
{
  ASSERT_NO_THROW(d_tm.mkInteger("123"));
  ASSERT_THROW(d_tm.mkInteger("1.23"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("1/23"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("12/3"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger(".2"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("2."), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger(""), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("asdf"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("1.2/3"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("."), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("/"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("2/"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkInteger("/2"), Ava6ApiException);

  ASSERT_NO_THROW(d_tm.mkInteger(1));
  ASSERT_NO_THROW(d_tm.mkInteger(-1));
}

TEST_F(TestApiBlackTermManager, mkReal)
{
  ASSERT_NO_THROW(d_tm.mkReal("123"));
  ASSERT_NO_THROW(d_tm.mkReal("1.23"));
  ASSERT_NO_THROW(d_tm.mkReal("1/23"));
  ASSERT_NO_THROW(d_tm.mkReal("12/3"));
  ASSERT_NO_THROW(d_tm.mkReal(".2"));
  ASSERT_NO_THROW(d_tm.mkReal("2."));
  ASSERT_THROW(d_tm.mkReal(""), Ava6ApiException);
  ASSERT_THROW(d_tm.mkReal("asdf"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkReal("1.2/3"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkReal("."), Ava6ApiException);
  ASSERT_THROW(d_tm.mkReal("/"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkReal("2/"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkReal("/2"), Ava6ApiException);

  int32_t val1 = 1;
  int64_t val2 = -1;
  uint32_t val3 = 1;
  uint64_t val4 = -1;
  ASSERT_NO_THROW(d_tm.mkReal(val1));
  ASSERT_NO_THROW(d_tm.mkReal(val2));
  ASSERT_NO_THROW(d_tm.mkReal(val3));
  ASSERT_NO_THROW(d_tm.mkReal(val4));
  ASSERT_NO_THROW(d_tm.mkReal(val4));
  ASSERT_NO_THROW(d_tm.mkReal(val1, val1));
  ASSERT_NO_THROW(d_tm.mkReal(val2, val2));
  ASSERT_NO_THROW(d_tm.mkReal(val3, val3));
  ASSERT_NO_THROW(d_tm.mkReal(val4, val4));
  ASSERT_NO_THROW(d_tm.mkReal("-1/-1"));
  ASSERT_NO_THROW(d_tm.mkReal("1/-1"));
  ASSERT_NO_THROW(d_tm.mkReal("-1/1"));
  ASSERT_NO_THROW(d_tm.mkReal("1/1"));
  ASSERT_THROW(d_tm.mkReal("/-5"), Ava6ApiException);
  ASSERT_THROW(d_tm.mkReal(1, 0), Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkRegexpAll)
{
  Sort strSort = d_tm.getStringSort();
  Term s = d_tm.mkConst(strSort, "s");
  ASSERT_NO_THROW(d_tm.mkTerm(Kind::STRING_IN_REGEXP, {s, d_tm.mkRegexpAll()}));
}

TEST_F(TestApiBlackTermManager, mkRegexpAllchar)
{
  Sort strSort = d_tm.getStringSort();
  Term s = d_tm.mkConst(strSort, "s");
  ASSERT_NO_THROW(
      d_tm.mkTerm(Kind::STRING_IN_REGEXP, {s, d_tm.mkRegexpAllchar()}));
}

TEST_F(TestApiBlackTermManager, mkRegexpNone)
{
  Sort strSort = d_tm.getStringSort();
  Term s = d_tm.mkConst(strSort, "s");
  ASSERT_NO_THROW(
      d_tm.mkTerm(Kind::STRING_IN_REGEXP, {s, d_tm.mkRegexpNone()}));
}


TEST_F(TestApiBlackTermManager, mkString)
{
  ASSERT_NO_THROW(d_tm.mkString(""));
  ASSERT_NO_THROW(d_tm.mkString("asdfasdf"));
  ASSERT_EQ(d_tm.mkString("asdf\\nasdf").toString(), "\"asdf\\u{5c}nasdf\"");
  ASSERT_EQ(d_tm.mkString("asdf\\u{005c}nasdf", true).toString(),
            "\"asdf\\u{5c}nasdf\"");
  std::u32string s;
  ASSERT_EQ(d_tm.mkString(s).getU32StringValue(), s);
}


TEST_F(TestApiBlackTermManager, mkTermFromOp)
{
  Sort bv32 = d_tm.mkBitVectorSort(32);
  Term a = d_tm.mkConst(bv32, "a");
  Term b = d_tm.mkConst(bv32, "b");
  std::vector<Term> v1 = {d_tm.mkInteger(1), d_tm.mkInteger(2)};
  std::vector<Term> v2 = {d_tm.mkInteger(1), Term()};
  std::vector<Term> v3 = {};
  std::vector<Term> v4 = {d_tm.mkInteger(5)};

  // simple operator terms
  Op opterm1 = d_tm.mkOp(Kind::BITVECTOR_EXTRACT, {2, 1});
  Op opterm2 = d_tm.mkOp(Kind::DIVISIBLE, {1});

  // list datatype
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
  Term consTerm = list.getConstructor("cons").getTerm();
  Term nilTerm = list.getConstructor("nil").getTerm();
  Term headTerm = list["cons"].getSelector("head").getTerm();
  Term tailTerm = list["cons"]["tail"].getTerm();

  // mkTerm(Op op, const std::vector<Term>& children) const
  ASSERT_NO_THROW(d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR, {nilTerm}));
  ASSERT_THROW(d_tm.mkTerm(Kind::APPLY_SELECTOR, {nilTerm}), Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(Kind::APPLY_SELECTOR, {consTerm}), Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR, {consTerm}),
               Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(opterm1), Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(Kind::APPLY_SELECTOR, {headTerm}), Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(opterm1), Ava6ApiException);
  ASSERT_NO_THROW(d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR, {nilTerm}));

  ASSERT_NO_THROW(d_tm.mkTerm(opterm1, {a}));
  ASSERT_NO_THROW(d_tm.mkTerm(opterm2, {d_tm.mkInteger(1)}));
  ASSERT_NO_THROW(d_tm.mkTerm(Kind::APPLY_SELECTOR, {headTerm, c}));
  ASSERT_NO_THROW(d_tm.mkTerm(Kind::APPLY_SELECTOR, {tailTerm, c}));
  ASSERT_THROW(d_tm.mkTerm(opterm2, {a}), Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(opterm1, {Term()}), Ava6ApiException);
  ASSERT_THROW(
      d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR, {consTerm, d_tm.mkInteger(0)}),
      Ava6ApiException);
  ASSERT_NO_THROW(d_tm.mkTerm(opterm1, {a}));

  ASSERT_NO_THROW(
      d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR,
                  {consTerm,
                   d_tm.mkInteger(0),
                   d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR, {nilTerm})}));
  ASSERT_THROW(d_tm.mkTerm(opterm2, {d_tm.mkInteger(1), d_tm.mkInteger(2)}),
               Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(opterm1, {a, b}), Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(opterm2, {d_tm.mkInteger(1), Term()}),
               Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(opterm2, {Term(), d_tm.mkInteger(1)}),
               Ava6ApiException);
  ASSERT_NO_THROW(
      d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR,
                  {consTerm,
                   d_tm.mkInteger(0),
                   d_tm.mkTerm(Kind::APPLY_CONSTRUCTOR, {nilTerm})}));

  ASSERT_THROW(d_tm.mkTerm(opterm1, {a, b, a}), Ava6ApiException);
  ASSERT_THROW(
      d_tm.mkTerm(opterm2, {d_tm.mkInteger(1), d_tm.mkInteger(1), Term()}),
      Ava6ApiException);

  ASSERT_NO_THROW(d_tm.mkTerm(opterm2, {v4}));
  ASSERT_THROW(d_tm.mkTerm(opterm2, {v1}), Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(opterm2, {v2}), Ava6ApiException);
  ASSERT_THROW(d_tm.mkTerm(opterm2, {v3}), Ava6ApiException);
  ASSERT_NO_THROW(d_tm.mkTerm(opterm2, {v4}));

  TermManager tm;
  ASSERT_THROW(tm.mkTerm(opterm2, {tm.mkInteger(1)}), Ava6ApiException);
  ASSERT_THROW(tm.mkTerm(tm.mkOp(Kind::DIVISIBLE, {1}), {d_tm.mkInteger(1)}),
               Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkTrue)
{
  ASSERT_NO_THROW(d_tm.mkTrue());
  ASSERT_NO_THROW(d_tm.mkTrue());
}

TEST_F(TestApiBlackTermManager, mkTuple)
{
  ASSERT_NO_THROW(d_tm.mkTuple({d_tm.mkBitVector(3, "101", 2)}));
  ASSERT_NO_THROW(d_tm.mkTuple({d_tm.mkInteger("5")}));
  ASSERT_NO_THROW(d_tm.mkTuple({d_tm.mkReal("5.3")}));
  ASSERT_NO_THROW(d_tm.mkTuple({d_tm.mkBitVector(3, "101", 2)}));
  ASSERT_NO_THROW(d_tm.mkTuple({d_tm.mkBitVector(3, "101", 2)}));

  TermManager tm;
  ASSERT_THROW(tm.mkTuple({d_tm.mkBitVector(3, "101", 2)}), Ava6ApiException);
}


TEST_F(TestApiBlackTermManager, mkConst)
{
  Sort boolSort = d_tm.getBooleanSort();
  Sort intSort = d_tm.getIntegerSort();
  Sort funSort = d_tm.mkFunctionSort({intSort}, boolSort);
  ASSERT_NO_THROW(d_tm.mkConst(boolSort));
  ASSERT_NO_THROW(d_tm.mkConst(funSort));
  ASSERT_NO_THROW(d_tm.mkConst(boolSort, std::string("b")));
  ASSERT_NO_THROW(d_tm.mkConst(intSort, std::string("i")));
  ASSERT_NO_THROW(d_tm.mkConst(funSort, "f"));
  ASSERT_NO_THROW(d_tm.mkConst(funSort, ""));
  ASSERT_THROW(d_tm.mkConst(Sort()), Ava6ApiException);
  ASSERT_THROW(d_tm.mkConst(Sort(), "a"), Ava6ApiException);
  ASSERT_NO_THROW(d_tm.mkConst(boolSort));

  TermManager tm;
  ASSERT_THROW(tm.mkConst(boolSort), Ava6ApiException);
}

TEST_F(TestApiBlackTermManager, mkSkolem)
{
  Sort integer = d_tm.getIntegerSort();
  Sort arraySort = d_tm.mkArraySort(integer, integer);

  Term a = d_tm.mkConst(arraySort, "a");
  Term b = d_tm.mkConst(arraySort, "b");

  Term sk = d_tm.mkSkolem(SkolemId::ARRAY_DEQ_DIFF, {a, b});
  Term sk2 = d_tm.mkSkolem(SkolemId::ARRAY_DEQ_DIFF, {b, a});

  ASSERT_THROW(d_tm.mkSkolem(SkolemId::ARRAY_DEQ_DIFF, {a}), Ava6ApiException);

  ASSERT_TRUE(sk.isSkolem());
  ASSERT_EQ(sk.getSkolemId(), SkolemId::ARRAY_DEQ_DIFF);
  ASSERT_EQ(sk.getSkolemIndices(), std::vector<Term>({a, b}));
  // ARRAY_DEQ_DIFF is commutative, so the order of the indices is sorted.
  ASSERT_EQ(sk2.getSkolemIndices(), std::vector<Term>({a, b}));
}

TEST_F(TestApiBlackTermManager, getNumIndicesForSkolemId)
{
  ASSERT_EQ(d_tm.getNumIndicesForSkolemId(SkolemId::ARITH_VTS_DELTA), 0);
  ASSERT_EQ(d_tm.getNumIndicesForSkolemId(SkolemId::ARITH_VTS_DELTA_FREE), 0);
  ASSERT_EQ(d_tm.getNumIndicesForSkolemId(SkolemId::ARITH_VTS_INFINITY), 1);
  ASSERT_EQ(d_tm.getNumIndicesForSkolemId(SkolemId::ARITH_VTS_INFINITY_FREE),
            1);
  ASSERT_EQ(d_tm.getNumIndicesForSkolemId(SkolemId::WITNESS_INV_CONDITION), 1);
  ASSERT_EQ(d_tm.getNumIndicesForSkolemId(SkolemId::WITNESS_STRING_LENGTH), 3);
  ASSERT_EQ(d_tm.getNumIndicesForSkolemId(SkolemId::STRINGS_REPLACE_ALL_RESULT),
            3);
  ASSERT_EQ(
      d_tm.getNumIndicesForSkolemId(SkolemId::STRINGS_REPLACE_RE_ALL_RESULT),
      3);
}

TEST_F(TestApiBlackTermManager, uFIteration)
{
  Sort intSort = d_tm.getIntegerSort();
  Sort funSort = d_tm.mkFunctionSort({intSort, intSort}, intSort);
  Term x = d_tm.mkConst(intSort, "x");
  Term y = d_tm.mkConst(intSort, "y");
  Term f = d_tm.mkConst(funSort, "f");
  Term fxy = d_tm.mkTerm(Kind::APPLY_UF, {f, x, y});

  // Expecting the uninterpreted function to be one of the children
  Term expected_children[3] = {f, x, y};
  uint32_t idx = 0;
  for (auto c : fxy)
  {
    ASSERT_LT(idx, 3);
    ASSERT_EQ(c, expected_children[idx]);
    idx++;
  }
}

TEST_F(TestApiBlackTermManager, getStatistics)
{
  ASSERT_NO_THROW(ava6::Stat());
  // do some array reasoning to make sure we have statistics
  {
    Sort s1 = d_tm.getIntegerSort();
    Sort s2 = d_tm.mkArraySort(s1, s1);
    Term t1 = d_tm.mkConst(s1, "i");
    Term t2 = d_tm.mkConst(s2, "a");
    Term t3 = d_tm.mkTerm(Kind::SELECT, {t2, t1});
    d_solver->assertFormula(t3.eqTerm(t1));
    d_solver->checkSat();
  }
  ava6::Statistics stats = d_tm.getStatistics();
  {
    std::stringstream ss;
    ss << stats;
  }
  for (const auto& s : stats)
  {
    ASSERT_FALSE(s.first.empty());
  }
  for (auto it = stats.begin(true, true); it != stats.end(); ++it)
  {
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
    if (s.first == "ava6::CONSTANT")
    {
      ASSERT_FALSE(s.second.isInternal());
      ASSERT_FALSE(s.second.isDefault());
      ASSERT_TRUE(s.second.isHistogram());
      auto hist = s.second.getHistogram();
      ASSERT_FALSE(hist.empty());
      std::stringstream ss;
      ss << s.second;
      ASSERT_EQ(ss.str(), "{ UNKNOWN_TYPE_CONSTANT: 1, integer type: 1 }");
    }
    else if (s.first == "theory::arrays::avgIndexListLength")
    {
      ASSERT_TRUE(s.second.isInternal());
      ASSERT_TRUE(s.second.isDouble());
      ASSERT_TRUE(std::isnan(s.second.getDouble()));
    }
  }
}

TEST_F(TestApiBlackTermManager, printStatisticsSafe)
{
  // do some array reasoning to make sure we have statistics
  {
    Sort s1 = d_tm.getIntegerSort();
    Sort s2 = d_tm.mkArraySort(s1, s1);
    Term t1 = d_tm.mkConst(s1, "i");
    Term t2 = d_tm.mkConst(s2, "a");
    Term t3 = d_tm.mkTerm(Kind::SELECT, {t2, t1});
    d_solver->assertFormula(t3.eqTerm(t1));
    d_solver->checkSat();
  }
  testing::internal::CaptureStdout();
  d_tm.printStatisticsSafe(STDOUT_FILENO);
  std::string out = testing::internal::GetCapturedStdout();
  std::stringstream expected;
  expected << "ava6::CONSTANT = { integer type: 1, UNKNOWN_TYPE_CONSTANT: 1 }"
           << std::endl
           << "ava6::TERM = { <unsupported>: 1 }" << std::endl;
  ASSERT_EQ(out, expected.str());
}

}  // namespace ava6::internal::test
