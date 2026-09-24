/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of RealAlgebraicNumber based on libpoly.
 */

#include "base/ava6config.h"
#include "util/real_algebraic_number.h"


#include <limits>
#include <sstream>

#include "base/check.h"

#define RAN_UNREACHABLE \
  Unreachable() << "RealAlgebraicNumber is not available without libpoly."

namespace ava6::internal {

RealAlgebraicNumber::RealAlgebraicNumber()
    :
      d_rat()
{
}


RealAlgebraicNumber::RealAlgebraicNumber(const Integer& i)
    :
      d_rat(i)
{
}

RealAlgebraicNumber::RealAlgebraicNumber(const Rational& r)
    :
      d_rat(r)
{
}

RealAlgebraicNumber::RealAlgebraicNumber(
    AVA6_UNUSED const std::vector<long>& coefficients,
    AVA6_UNUSED long lower,
    AVA6_UNUSED long upper)
{
#ifdef AVA6_ASSERTIONS
  for (long c : coefficients)
  {
    Assert(std::numeric_limits<int32_t>::min() <= c
           && c <= std::numeric_limits<int32_t>::max())
        << "Coefficients need to fit within 32 bit integers. Please use the "
           "constructor based on Integer instead.";
  }
#endif
  RAN_UNREACHABLE;
}

RealAlgebraicNumber::RealAlgebraicNumber(
    AVA6_UNUSED const std::vector<Integer>& coefficients,
    AVA6_UNUSED const Rational& lower,
    AVA6_UNUSED const Rational& upper)
{
  RAN_UNREACHABLE;
}
RealAlgebraicNumber::RealAlgebraicNumber(
    AVA6_UNUSED const std::vector<Rational>& coefficients,
    AVA6_UNUSED const Rational& lower,
    AVA6_UNUSED const Rational& upper)
{
  RAN_UNREACHABLE;
}

bool RealAlgebraicNumber::isRational() const
{
  return true;
}
Rational RealAlgebraicNumber::toRational() const
{
  return getRationalValue();
}

std::string RealAlgebraicNumber::toString() const
{
  std::stringstream ss;
  ss << getRationalValue();
  return ss.str();
}


bool RealAlgebraicNumber::operator==(const RealAlgebraicNumber& rhs) const
{
  return getRationalValue() == rhs.getRationalValue();
}
bool RealAlgebraicNumber::operator!=(const RealAlgebraicNumber& rhs) const
{
  return !(*this == rhs);
}
bool RealAlgebraicNumber::operator<(const RealAlgebraicNumber& rhs) const
{
  return getRationalValue() < rhs.getRationalValue();
}
bool RealAlgebraicNumber::operator<=(const RealAlgebraicNumber& rhs) const
{
  return getRationalValue() <= rhs.getRationalValue();
}
bool RealAlgebraicNumber::operator>(const RealAlgebraicNumber& rhs) const
{
  return rhs < *this;
}
bool RealAlgebraicNumber::operator>=(const RealAlgebraicNumber& rhs) const
{
  return rhs <= *this;
  ;
}

RealAlgebraicNumber RealAlgebraicNumber::operator+(
    const RealAlgebraicNumber& rhs) const
{
  return getRationalValue() + rhs.getRationalValue();
}
RealAlgebraicNumber RealAlgebraicNumber::operator-(
    const RealAlgebraicNumber& rhs) const
{
  return getRationalValue() - rhs.getRationalValue();
}
RealAlgebraicNumber RealAlgebraicNumber::operator-() const
{
  return -getRationalValue();
}
RealAlgebraicNumber RealAlgebraicNumber::operator*(
    const RealAlgebraicNumber& rhs) const
{
  return getRationalValue() * rhs.getRationalValue();
}
RealAlgebraicNumber RealAlgebraicNumber::operator/(
    const RealAlgebraicNumber& rhs) const
{
  Assert(!rhs.isZero()) << "Can not divide by zero";
  return getRationalValue() / rhs.getRationalValue();
}

RealAlgebraicNumber& RealAlgebraicNumber::operator+=(
    const RealAlgebraicNumber& rhs)
{
  getRationalValue() = getRationalValue() + rhs.getRationalValue();
  return *this;
}
RealAlgebraicNumber& RealAlgebraicNumber::operator-=(
    const RealAlgebraicNumber& rhs)
{
  getRationalValue() = getRationalValue() - rhs.getRationalValue();
  return *this;
}
RealAlgebraicNumber& RealAlgebraicNumber::operator*=(
    const RealAlgebraicNumber& rhs)
{
  getRationalValue() = getRationalValue() * rhs.getRationalValue();
  return *this;
}

int RealAlgebraicNumber::sgn() const
{
  return getRationalValue().sgn();
}
bool RealAlgebraicNumber::isZero() const
{
  return getRationalValue().isZero();
}
bool RealAlgebraicNumber::isOne() const
{
  return getRationalValue().isOne();
}
RealAlgebraicNumber RealAlgebraicNumber::inverse() const
{
  Assert(!isZero()) << "Can not invert zero";
  return getRationalValue().inverse();
}

size_t RealAlgebraicNumber::hash() const
{
  return getRationalValue().hash();
}

std::ostream& operator<<(std::ostream& os, const RealAlgebraicNumber& ran)
{
  return os << ran.toString();
}

}  // namespace ava6::internal

namespace std {
size_t hash<ava6::internal::RealAlgebraicNumber>::operator()(
    const ava6::internal::RealAlgebraicNumber& ran) const
{
  return ran.hash();
}

}  // namespace std
