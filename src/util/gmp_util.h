/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * GMP utilities.
 */

#include "ava6_public.h"

#ifndef AVA6__GMP_UTIL_H
#define AVA6__GMP_UTIL_H

#include <gmpxx.h>

// the following includes are only required if AVA6_NEED_INT64_T_OVERLOADS is
// defined.
#ifdef AVA6_NEED_INT64_T_OVERLOADS
#include <string>
#include <type_traits>
#endif /* AVA6_NEED_INT64_T_OVERLOADS */

namespace ava6::internal {

/** Hashes the gmp integer primitive in a word by word fashion. */
inline size_t gmpz_hash(const mpz_t toHash)
{
  size_t hash = 0;
  for (int i = 0, n = mpz_size(toHash); i < n; ++i)
  {
    mp_limb_t limb = mpz_getlimbn(toHash, i);
    hash = hash * 2;
    hash = hash ^ limb;
  }
  return hash;
} /* gmpz_hash() */

#ifdef AVA6_NEED_INT64_T_OVERLOADS
/*
 * Before converting an int64_t/uint64_t to a GMP value, we need to check if
 * the input value will fit inside of a (un)signed long, which is the largest
 * overload supported by GMP's C++ API.
 *
 * If the input value is *larger* than (un)signed long, then we use GMP's
 * string-based API to construct the value.
 *
 * The following function looks-up the largest matching type against the input
 * type, checks the range, and then decides to use the string-based overloads
 * if the value does not fit.
 */
template <typename T>
mpz_class construct_mpz(T z)
{
  /* only use this with int64_t or uint64_t */
  static_assert(
      std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value,
      "construct_mpz only supports int64_t or uint64_t");

  /* what's the largest GMP-supported type? */
  using gmp_type = typename std::
      conditional<std::is_signed<T>::value, signed long, unsigned long>::type;

  /* result value */
  mpz_class result;

  /* if our input fits inside of the range of the largest GMP-support type, we
   * construct result based on the value */
  if (std::numeric_limits<gmp_type>::min() <= z
      && z <= std::numeric_limits<gmp_type>::max())
  {
    result = static_cast<gmp_type>(z);
  }
  else
  {
    /* otherwise, use the string-based methods */
    result = std::to_string(z);
  }
  return result;
}
#endif /* AVA6_NEED_INT64_T_OVERLOADS */
}  // namespace ava6::internal

#endif /* AVA6__GMP_UTIL_H */
