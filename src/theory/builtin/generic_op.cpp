/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * A class for representing abstract types.
 */

#include "theory/builtin/generic_op.h"

#include <iostream>

#include "expr/dtype.h"
#include "expr/dtype_cons.h"
#include "theory/datatypes/project_op.h"
#include "theory/datatypes/theory_datatypes_utils.h"
#include "theory/evaluator.h"
#include "util/bitvector.h"
#include "util/divisible.h"
#include "util/iand.h"
#include "util/rational.h"
#include "util/regexp.h"

using namespace ava6::internal::kind;

namespace ava6::internal {

std::ostream& operator<<(std::ostream& out, const GenericOp& op)
{
  return out << "(GenericOp " << op.getKind() << ')';
}

size_t GenericOpHashFunction::operator()(const GenericOp& op) const
{
  return kind::KindHashFunction()(op.getKind());
}

GenericOp::GenericOp(Kind k) : d_kind(k) {}

GenericOp::GenericOp(const GenericOp& op) : d_kind(op.getKind()) {}

Kind GenericOp::getKind() const { return d_kind; }

bool GenericOp::operator==(const GenericOp& op) const
{
  return getKind() == op.getKind();
}

bool GenericOp::isNumeralIndexedOperatorKind(Kind k)
{
  return k == Kind::DIVISIBLE || k == Kind::REGEXP_LOOP
         || k == Kind::REGEXP_REPEAT || k == Kind::BITVECTOR_EXTRACT
         || k == Kind::BITVECTOR_REPEAT || k == Kind::BITVECTOR_ZERO_EXTEND
         || k == Kind::BITVECTOR_SIGN_EXTEND || k == Kind::BITVECTOR_ROTATE_LEFT
         || k == Kind::BITVECTOR_ROTATE_RIGHT || k == Kind::INT_TO_BITVECTOR
         || k == Kind::BITVECTOR_BIT || k == Kind::IAND;
}

bool GenericOp::isIndexedOperatorKind(Kind k)
{
  return isNumeralIndexedOperatorKind(k) || k == Kind::APPLY_UPDATER
         || k == Kind::APPLY_TESTER;
}

std::vector<Node> GenericOp::getIndicesForOperator(Kind k, Node n)
{
  NodeManager* nm = n.getNodeManager();
  std::vector<Node> indices;
  switch (k)
  {
    case Kind::DIVISIBLE:
    {
      const Divisible& op = n.getConst<Divisible>();
      indices.push_back(nm->mkConstInt(Rational(op.k)));
      break;
    }
    case Kind::REGEXP_REPEAT:
    {
      const RegExpRepeat& op = n.getConst<RegExpRepeat>();
      indices.push_back(nm->mkConstInt(Rational(op.d_repeatAmount)));
      break;
    }
    case Kind::REGEXP_LOOP:
    {
      const RegExpLoop& op = n.getConst<RegExpLoop>();
      indices.push_back(nm->mkConstInt(Rational(op.d_loopMinOcc)));
      indices.push_back(nm->mkConstInt(Rational(op.d_loopMaxOcc)));
      break;
    }
    case Kind::BITVECTOR_EXTRACT:
    {
      const BitVectorExtract& p = n.getConst<BitVectorExtract>();
      indices.push_back(nm->mkConstInt(Rational(p.d_high)));
      indices.push_back(nm->mkConstInt(Rational(p.d_low)));
      break;
    }
    case Kind::BITVECTOR_REPEAT:
      indices.push_back(nm->mkConstInt(
          Rational(n.getConst<BitVectorRepeat>().d_repeatAmount)));
      break;
    case Kind::BITVECTOR_ZERO_EXTEND:
      indices.push_back(nm->mkConstInt(
          Rational(n.getConst<BitVectorZeroExtend>().d_zeroExtendAmount)));
      break;
    case Kind::BITVECTOR_SIGN_EXTEND:
      indices.push_back(nm->mkConstInt(
          Rational(n.getConst<BitVectorSignExtend>().d_signExtendAmount)));
      break;
    case Kind::BITVECTOR_ROTATE_LEFT:
      indices.push_back(nm->mkConstInt(
          Rational(n.getConst<BitVectorRotateLeft>().d_rotateLeftAmount)));
      break;
    case Kind::BITVECTOR_ROTATE_RIGHT:
      indices.push_back(nm->mkConstInt(
          Rational(n.getConst<BitVectorRotateRight>().d_rotateRightAmount)));
      break;
    case Kind::BITVECTOR_BIT:
      indices.push_back(
          nm->mkConstInt(Rational(n.getConst<BitVectorBit>().d_bitIndex)));
      break;
    case Kind::INT_TO_BITVECTOR:
      indices.push_back(
          nm->mkConstInt(Rational(n.getConst<IntToBitVector>().d_size)));
      break;
    case Kind::IAND:
      indices.push_back(nm->mkConstInt(Rational(n.getConst<IntAnd>().d_size)));
      break;


    case Kind::APPLY_TESTER:
    {
      unsigned index = DType::indexOf(n);
      const DType& dt = DType::datatypeOf(n);
      indices.push_back(dt[index].getConstructor());
    }
    break;
    case Kind::APPLY_UPDATER:
    {
      unsigned index = DType::indexOf(n);
      const DType& dt = DType::datatypeOf(n);
      unsigned cindex = DType::cindexOf(n);
      indices.push_back(dt[cindex][index].getSelector());
    }
    break;
    default:
      Unhandled() << "GenericOp::getOperatorIndices: unhandled kind " << k;
      break;
  }
  return indices;
}

/** Converts a list of constant integer terms to a list of unsigned integers */
bool convertToNumeralList(const std::vector<Node>& indices,
                          std::vector<uint32_t>& numerals)
{
  for (const Node& i : indices)
  {
    Node in = i;
    if (in.getKind() != Kind::CONST_INTEGER)
    {
      // If trivially evaluatable, take that as the numeral.
      // This implies that we can concretize applications of
      // APPLY_INDEXED_SYMBOLIC whose indices can evaluate. This in turn
      // implies that e.g. (extract (+ 2 2) 2 x) concretizes via getConcreteApp
      // to ((_ extract 4 2) x), which implies it has type (BitVec 3) based
      // on the type rule for APPLY_INDEXED_SYMBOLIC.
      theory::Evaluator e(nullptr);
      in = e.eval(in, {}, {});
      if (in.isNull() || in.getKind() != Kind::CONST_INTEGER)
      {
        return false;
      }
    }
    const Integer& ii = in.getConst<Rational>().getNumerator();
    if (!ii.fitsUnsignedInt())
    {
      return false;
    }
    numerals.push_back(ii.toUnsignedInt());
  }
  return true;
}

Node GenericOp::getOperatorForIndices(NodeManager* nm,
                                      Kind k,
                                      const std::vector<Node>& indices)
{
  // all indices should be constant!
  Assert(isIndexedOperatorKind(k));
  if (isNumeralIndexedOperatorKind(k))
  {
    std::vector<uint32_t> numerals;
    if (!convertToNumeralList(indices, numerals))
    {
      // failed to convert due to non-constant, or overflow on index
      return Node::null();
    }
    switch (k)
    {
      case Kind::DIVISIBLE:
        Assert(numerals.size() == 1);
        return nm->mkConst(Divisible(numerals[0]));
      case Kind::REGEXP_REPEAT:
        Assert(numerals.size() == 1);
        return nm->mkConst(RegExpRepeat(numerals[0]));
      case Kind::REGEXP_LOOP:
        Assert(numerals.size() == 2);
        return nm->mkConst(RegExpLoop(numerals[0], numerals[1]));
      case Kind::BITVECTOR_EXTRACT:
        Assert(numerals.size() == 2);
        return nm->mkConst(BitVectorExtract(numerals[0], numerals[1]));
      case Kind::BITVECTOR_REPEAT:
        Assert(numerals.size() == 1);
        return nm->mkConst(BitVectorRepeat(numerals[0]));
      case Kind::BITVECTOR_ZERO_EXTEND:
        Assert(numerals.size() == 1);
        return nm->mkConst(BitVectorZeroExtend(numerals[0]));
      case Kind::BITVECTOR_SIGN_EXTEND:
        Assert(numerals.size() == 1);
        return nm->mkConst(BitVectorSignExtend(numerals[0]));
      case Kind::BITVECTOR_ROTATE_LEFT:
        Assert(numerals.size() == 1);
        return nm->mkConst(BitVectorRotateLeft(numerals[0]));
      case Kind::BITVECTOR_ROTATE_RIGHT:
        Assert(numerals.size() == 1);
        return nm->mkConst(BitVectorRotateRight(numerals[0]));
      case Kind::BITVECTOR_BIT:
        Assert(numerals.size() == 1);
        return nm->mkConst(BitVectorBit(numerals[0]));
      case Kind::INT_TO_BITVECTOR:
        Assert(numerals.size() == 1);
        return nm->mkConst(IntToBitVector(numerals[0]));
      case Kind::IAND:
        Assert(numerals.size() == 1);
        return nm->mkConst(IntAnd(numerals[0]));
      default:
        Unhandled() << "GenericOp::getOperatorForIndices: unhandled kind " << k;
        break;
    }
  }
  else
  {
    switch (k)
    {
      case Kind::APPLY_TESTER:
      {
        Assert(indices.size() == 1);
        unsigned index = DType::indexOf(indices[0]);
        const DType& dt = DType::datatypeOf(indices[0]);
        return dt[index].getTester();
      }
      break;
      case Kind::APPLY_UPDATER:
      {
        Assert(indices.size() == 1);
        unsigned index = DType::indexOf(indices[0]);
        const DType& dt = DType::datatypeOf(indices[0]);
        unsigned cindex = DType::cindexOf(indices[0]);
        return dt[cindex][index].getUpdater();
      }
      break;
      default:
        Unhandled() << "GenericOp::getOperatorForIndices: unhandled kind" << k;
        break;
    }
  }
  return Node::null();
}

Node GenericOp::getConcreteApp(const Node& app)
{
  Trace("generic-op") << "getConcreteApp " << app << std::endl;
  Assert(app.getKind() == Kind::APPLY_INDEXED_SYMBOLIC);
  Kind okind = app.getOperator().getConst<GenericOp>().getKind();
  // determine how many arguments should be passed to the end function. This is
  // usually one, but we handle cases where it is >1.
  size_t nargs = metakind::getMinArityForKind(okind);
  std::vector<Node> indices(app.begin(), app.end() - nargs);
  NodeManager* nm = app.getNodeManager();
  Node op = getOperatorForIndices(nm, okind, indices);
  // could have a bad index, in which case we don't rewrite
  if (op.isNull())
  {
    return app;
  }
  std::vector<Node> args;
  args.push_back(op);
  args.insert(args.end(), app.end() - nargs, app.end());
  Node ret = nm->mkNode(okind, args);
  // could be ill typed, in which case we don't rewrite
  if (ret.getTypeOrNull(true).isNull())
  {
    return app;
  }
  return ret;
}

}  // namespace ava6::internal
