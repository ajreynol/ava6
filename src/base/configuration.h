/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Interface to a public class that provides compile-time information
 * about the ava6 library.
 *
 * Eventually, the configuration methods will all be migrated to the
 * ava6::internal::configuration namespace below. This is cleaner and avoids a
 * gcc/10.1.0 bug. See https://github.com/cvc5/cvc5/pull/7898 for details.
 */

#include "ava6_public.h"

#ifndef AVA6__CONFIGURATION_H
#define AVA6__CONFIGURATION_H

#include <ava6/ava6_export.h>

#include <string>
#include <vector>

namespace ava6::internal {

namespace configuration {
static constexpr bool isStatisticsBuild()
{
#ifdef AVA6_STATISTICS_ON
  return true;
#else
  return false;
#endif
}
}  // namespace configuration

/**
 * Represents the (static) configuration of ava6.
 */
class AVA6_EXPORT Configuration final
{
 public:
  /** Delete default ctor: Disallow construction of this class. */
  Configuration() = delete;

 private:
  // these constants are filled in by the build system
  static const bool GIT_BUILD;
  static const bool AVA6_IS_RELEASE;
  static const char* const AVA6_VERSION;
  static const char* const AVA6_FULL_VERSION;
  static const char* const AVA6_GIT_INFO;

 public:
  static std::string getName();



  static bool isDebugBuild();

  static bool isTracingBuild();

  static bool isMuzzledBuild();

  static bool isAssertionBuild();

  static bool isCoverageBuild();

  static bool isProfilingBuild();

  static bool isAsanBuild();

  static bool isUbsanBuild();

  static bool isTsanBuild();

  static bool isCompetitionBuild();

  static bool isStaticBuild();

  static std::string getPackageName();

  static std::string getVersionString();

  static std::string copyright();

  static std::string about();

  static std::string aboutAndCopyright();

  static bool licenseIsGpl();

  static bool isBuiltWithGmp();

  static bool isBuiltWithCln();

  static bool isBuiltWithGlpk();

  static bool isBuiltWithCryptominisat();

  static bool isBuiltWithKissat();

  static bool isBuiltWithPoly();

  static bool isBuiltWithCoCoA();

  static bool isBuiltWithNormaliz();


  /* Return a sorted array of the trace tags name */
  static const std::vector<std::string>& getTraceTags();
  /* Test if the given argument is a known trace tag name */
  static bool isTraceTag(const std::string& tag);

  static bool isGitBuild();
  static std::string getGitInfo();

  static std::string getCompiler();
  static std::string getCompiledDateTime();

  static std::string getBuildType();

}; /* class Configuration */

}  // namespace ava6::internal

#endif /* AVA6__CONFIGURATION_H */
