/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Common ava6 types. These types are used internally as well as externally and
 * the language bindings are generated automatically.
 */

#include <ava6/ava6.h>
#include <ava6/ava6_types.h>

#include <iostream>
#include <sstream>

#include "base/check.h"

namespace ava6 {
std::ostream& operator<<(std::ostream& out, RoundingMode rm)
{
  switch (rm)
  {
    case RoundingMode::ROUND_NEAREST_TIES_TO_EVEN: out << "RNE"; break;
    case RoundingMode::ROUND_TOWARD_POSITIVE: out << "RTP"; break;
    case RoundingMode::ROUND_TOWARD_NEGATIVE: out << "RTN"; break;
    case RoundingMode::ROUND_TOWARD_ZERO: out << "RTZ"; break;
    case RoundingMode::ROUND_NEAREST_TIES_TO_AWAY: out << "RNA"; break;
    default:
      throw Ava6ApiException("unhandled enum value '"
                             + std::to_string(static_cast<int32_t>(rm))
                             + "' encountered");
  }
  return out;
}
}  // namespace ava6

namespace std {
std::string to_string(ava6::RoundingMode rm)
{
  std::stringstream ss;
  ss << rm;
  return ss.str();
}
}  // namespace std

namespace ava6 {
std::ostream& operator<<(std::ostream& out, UnknownExplanation e)
{
  switch (e)
  {
    case UnknownExplanation::REQUIRES_FULL_CHECK:
      out << "REQUIRES_FULL_CHECK";
      break;
    case UnknownExplanation::INCOMPLETE: out << "INCOMPLETE"; break;
    case UnknownExplanation::TIMEOUT: out << "TIMEOUT"; break;
    case UnknownExplanation::RESOURCEOUT: out << "RESOURCEOUT"; break;
    case UnknownExplanation::MEMOUT: out << "MEMOUT"; break;
    case UnknownExplanation::INTERRUPTED: out << "INTERRUPTED"; break;
    case UnknownExplanation::UNSUPPORTED: out << "UNSUPPORTED"; break;
    case UnknownExplanation::OTHER: out << "OTHER"; break;
    case UnknownExplanation::REQUIRES_CHECK_AGAIN:
      out << "REQUIRES_CHECK_AGAIN";
      break;
    case UnknownExplanation::UNKNOWN_REASON: out << "UNKNOWN_REASON"; break;
    default:
      throw Ava6ApiException("unhandled enum value '"
                             + std::to_string(static_cast<int32_t>(e))
                             + "' encountered");
  }
  return out;
}
}  // namespace ava6

namespace std {
std::string to_string(ava6::UnknownExplanation exp)
{
  std::stringstream ss;
  ss << exp;
  return ss.str();
}
}  // namespace std

namespace ava6::modes {
std::ostream& operator<<(std::ostream& out, ProofComponent pc)
{
  switch (pc)
  {
    case ProofComponent::RAW_PREPROCESS: out << "raw_preprocess"; break;
    case ProofComponent::PREPROCESS: out << "preprocess"; break;
    case ProofComponent::SAT: out << "sat"; break;
    case ProofComponent::THEORY_LEMMAS: out << "theory_lemmas"; break;
    case ProofComponent::FULL: out << "full"; break;
    default: out << "?";
  }
  return out;
}
}  // namespace ava6::modes

namespace std {
std::string to_string(ava6::modes::ProofComponent pc)
{
  std::stringstream ss;
  ss << pc;
  return ss.str();
}
}  // namespace std

namespace ava6::modes {
std::ostream& operator<<(std::ostream& out, ProofFormat)
{
  return out << "cpc";
}
}  // namespace ava6::modes

namespace std {
std::string to_string(ava6::modes::ProofFormat format)
{
  std::stringstream ss;
  ss << format;
  return ss.str();
}
}  // namespace std

namespace ava6::modes {
std::ostream& operator<<(std::ostream& out, OptionCategory cat)
{
  switch (cat)
  {
    case OptionCategory::REGULAR: out << "regular"; break;
    case OptionCategory::EXPERT: out << "expert"; break;
    case OptionCategory::COMMON: out << "common"; break;
    case OptionCategory::UNDOCUMENTED: out << "undocumented"; break;
    default: out << "?";
  }
  return out;
}
}  // namespace ava6::modes

namespace std {
std::string to_string(ava6::modes::OptionCategory category)
{
  std::stringstream ss;
  ss << category;
  return ss.str();
}
}  // namespace std

namespace ava6::modes {
std::ostream& operator<<(std::ostream& out, InputLanguage lang)
{
  switch (lang)
  {
    case InputLanguage::SMT_LIB_2_6: out << "smt_lib_2_6"; break;
    case InputLanguage::UNKNOWN: out << "unknown"; break;
    default: out << "?";
  }
  return out;
}
}  // namespace ava6::modes

namespace std {
std::string to_string(ava6::modes::InputLanguage lang)
{
  std::stringstream ss;
  ss << lang;
  return ss.str();
}
}  // namespace std
