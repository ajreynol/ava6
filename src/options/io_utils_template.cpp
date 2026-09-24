/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * IO manipulation classes.
 */

#include <iomanip>
#include <iostream>

#include "options/io_utils.h"

namespace ava6::internal::options::ioutils {
namespace {

// There is no good way to figure out whether the value behind iword() was
// explicitly set. The default value is zero; we shift by some random constant
// such that zero is never a valid value, and we can still use both negative
// and positive values.
static constexpr long value_offset = 1024;

template <typename T>
void setData(std::ios_base& ios, int iosIndex, T value)
{
  ios.iword(iosIndex) = static_cast<long>(value) + value_offset;
}
template <typename T>
T getData(std::ios_base& ios, int iosIndex, T defaultValue)
{
  long& l = ios.iword(iosIndex);
  if (l == 0)
  {
    return defaultValue;
  }
  return static_cast<T>(l - value_offset);
}

}  // namespace

static const int s_cpcFormat = std::ios_base::xalloc();
void applyCpcFormat(std::ios_base& ios, bool enabled)
{
  setData(ios, s_cpcFormat, enabled);
}
bool getCpcFormat(std::ios_base& ios)
{
  return getData(ios, s_cpcFormat, false);
}

// clang-format off
${ioimpls}$
    // clang-format on

    Scope::Scope(std::ios_base& ios)
    : d_ios(ios),
      d_cpcFormat(getCpcFormat(ios)),
      // clang-format off
${ioscope_memberinit}$
// clang-format on
{
}

Scope::~Scope()
{
  applyCpcFormat(d_ios, d_cpcFormat);
  // clang-format off
${ioscope_restore}$
  // clang-format on
}

}  // namespace ava6::internal::options::ioutils
