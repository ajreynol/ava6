/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of ava6::Configuration.
 */

#include "base/configuration.h"
#include "test.h"

namespace ava6::internal {
namespace test {

class TestUtilBlackConfiguration : public TestInternal
{
};

TEST_F(TestUtilBlackConfiguration, static_flags)
{
  const bool debug =
#ifdef AVA6_DEBUG
      true;
#else  /* AVA6_DEBUG */
      false;
#endif /* AVA6_DEBUG */

  const bool tracing =
#ifdef AVA6_TRACING
      true;
#else  /* AVA6_TRACING */
      false;
#endif /* AVA6_TRACING */

  const bool muzzled =
#ifdef AVA6_MUZZLE
      true;
#else  /* AVA6_MUZZLE */
      false;
#endif /* AVA6_MUZZLE */

  const bool assertions =
#ifdef AVA6_ASSERTIONS
      true;
#else  /* AVA6_ASSERTIONS */
      false;
#endif /* AVA6_ASSERTIONS */

  const bool coverage =
#ifdef AVA6_COVERAGE
      true;
#else  /* AVA6_COVERAGE */
      false;
#endif /* AVA6_COVERAGE */

  const bool profiling =
#ifdef AVA6_PROFILING
      true;
#else  /* AVA6_PROFILING */
      false;
#endif /* AVA6_PROFILING */

  ASSERT_EQ(Configuration::isDebugBuild(), debug);
  ASSERT_EQ(Configuration::isTracingBuild(), tracing);
  ASSERT_EQ(Configuration::isMuzzledBuild(), muzzled);
  ASSERT_EQ(Configuration::isAssertionBuild(), assertions);
  ASSERT_EQ(Configuration::isCoverageBuild(), coverage);
  ASSERT_EQ(Configuration::isProfilingBuild(), profiling);
}

TEST_F(TestUtilBlackConfiguration, package_name)
{
  ASSERT_EQ(Configuration::getPackageName(), "ava6");
}

TEST_F(TestUtilBlackConfiguration, versions)
{
  // just test that the functions exist
  Configuration::getVersionString();
}

TEST_F(TestUtilBlackConfiguration, about)
{
  // just test that the functions exists
  Configuration::about();
  Configuration::copyright();
  Configuration::aboutAndCopyright();
}
}  // namespace test
}  // namespace ava6::internal
