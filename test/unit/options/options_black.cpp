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

#include <algorithm>
#include <limits>

#include "options/option_exception.h"
#include "options/options_public.h"
#include "test_api.h"

namespace ava6::internal {

namespace test {

template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

class TestBlackOptions : public TestApi
{
 public:
  /**
   * Tests setting options for option "name", including error values.
   */
  void testSetOption(const std::string& name)
  {
    auto info = d_solver->getOptionInfo(name);

    try
    {
      std::visit(
          overloaded{
              [this, &name](const OptionInfo::VoidInfo&) {
                d_solver->setOption(name, "");
              },
              [this, &name](const OptionInfo::ValueInfo<bool>&) {
                d_solver->setOption(name, "false");
                d_solver->setOption(name, "true");
              },
              [this, &name](const OptionInfo::ValueInfo<std::string>&) {
                d_solver->setOption(name, "foo");
              },
              [this, &name](const OptionInfo::NumberInfo<int64_t>& v) {
                std::pair<int64_t, int64_t> range{
                    std::numeric_limits<int64_t>::min(),
                    std::numeric_limits<int64_t>::max()};
                if (v.minimum)
                {
                  EXPECT_THROW(
                      d_solver->setOption(name, std::to_string(*v.minimum - 1)),
                      Ava6ApiOptionException);
                  EXPECT_NO_THROW(
                      d_solver->setOption(name, std::to_string(*v.minimum)));
                  range.first = *v.minimum;
                }
                if (v.maximum)
                {
                  EXPECT_THROW(
                      d_solver->setOption(name, std::to_string(*v.maximum + 1)),
                      Ava6ApiOptionException);
                  EXPECT_NO_THROW(
                      d_solver->setOption(name, std::to_string(*v.maximum)));
                  range.second = *v.maximum;
                }
                // Compute the midpoint without overflowing. Note that neither
                // (first + second) nor the span (second - first) is
                // representable in general, e.g., for [-1, INT64_MAX].
                EXPECT_NO_THROW(d_solver->setOption(
                    name,
                    std::to_string(range.first / 2 + range.second / 2
                                   + (range.first % 2 + range.second % 2)
                                         / 2)));
                EXPECT_THROW(d_solver->setOption(name, "0123abc"),
                             Ava6ApiOptionException);
              },
              [this, &name](const OptionInfo::NumberInfo<uint64_t>& v) {
                std::pair<uint64_t, uint64_t> range{
                    std::numeric_limits<uint64_t>::min(),
                    std::numeric_limits<uint64_t>::max()};
                EXPECT_THROW(d_solver->setOption(name, "-1"),
                             Ava6ApiOptionException);
                if (v.minimum)
                {
                  EXPECT_THROW(
                      d_solver->setOption(name, std::to_string(*v.minimum - 1)),
                      Ava6ApiOptionException);
                  EXPECT_NO_THROW(
                      d_solver->setOption(name, std::to_string(*v.minimum)));
                  range.first = *v.minimum;
                }
                if (v.maximum)
                {
                  EXPECT_THROW(
                      d_solver->setOption(name, std::to_string(*v.maximum + 1)),
                      Ava6ApiOptionException);
                  EXPECT_NO_THROW(
                      d_solver->setOption(name, std::to_string(*v.maximum)));
                  range.second = *v.maximum;
                }
                // Compute the midpoint without overflowing: range.second is
                // UINT64_MAX unless the option declares a maximum.
                EXPECT_NO_THROW(d_solver->setOption(
                    name,
                    std::to_string(range.first
                                   + (range.second - range.first) / 2)));
                EXPECT_THROW(d_solver->setOption(name, "0123abc"),
                             Ava6ApiOptionException);
              },
              [this, &name](const OptionInfo::NumberInfo<double>& v) {
                std::pair<double, double> range{
                    std::numeric_limits<double>::min(),
                    std::numeric_limits<double>::max()};
                if (v.minimum)
                {
                  EXPECT_THROW(
                      d_solver->setOption(name, std::to_string(*v.minimum - 1)),
                      Ava6ApiOptionException);
                  EXPECT_NO_THROW(
                      d_solver->setOption(name, std::to_string(*v.minimum)));
                  range.first = *v.minimum;
                }
                if (v.maximum)
                {
                  EXPECT_THROW(
                      d_solver->setOption(name, std::to_string(*v.maximum + 1)),
                      Ava6ApiOptionException);
                  EXPECT_NO_THROW(
                      d_solver->setOption(name, std::to_string(*v.maximum)));
                  range.second = *v.maximum;
                }
                EXPECT_NO_THROW(d_solver->setOption(
                    name, std::to_string((range.first + range.second) / 2)));
              },
              [this, &name](const OptionInfo::ModeInfo& v) {
                EXPECT_THROW(d_solver->setOption(name, "foobarbaz"),
                             Ava6ApiOptionException);
                for (const auto& m : v.modes)
                {
                  d_solver->setOption(name, m);
                  EXPECT_EQ(d_solver->getOption(name), m);
                }
                EXPECT_DEATH(d_solver->setOption(name, "help"), "");
              },
          },
          info.valueInfo);
    }
    catch (const Ava6ApiOptionException&)
    {
    }
  }
  /**
   * Sets a single valid option for option "name".
   */
  void testSetOptionOnce(const std::string& name)
  {
    auto info = d_solver->getOptionInfo(name);

    try
    {
      std::visit(
          overloaded{
              [this, &name](const OptionInfo::VoidInfo&) {
                d_solver->setOption(name, "");
              },
              [this, &name](const OptionInfo::ValueInfo<bool>&) {
                d_solver->setOption(name, "false");
              },
              [this, &name](const OptionInfo::ValueInfo<std::string>&) {
                d_solver->setOption(name, "foo");
              },
              [this, &name](const OptionInfo::NumberInfo<int64_t>&) {
                std::pair<int64_t, int64_t> range{
                    std::numeric_limits<int64_t>::min(),
                    std::numeric_limits<int64_t>::max()};
                d_solver->setOption(
                    name, std::to_string((range.first + range.second) / 2));
              },
              [this, &name](const OptionInfo::NumberInfo<uint64_t>&) {
                std::pair<uint64_t, uint64_t> range{
                    std::numeric_limits<uint64_t>::min(),
                    std::numeric_limits<uint64_t>::max()};
                d_solver->setOption(
                    name, std::to_string((range.first + range.second) / 2));
              },
              [this, &name](const OptionInfo::NumberInfo<double>&) {
                std::pair<double, double> range{
                    std::numeric_limits<double>::min(),
                    std::numeric_limits<double>::max()};
                d_solver->setOption(
                    name, std::to_string((range.first + range.second) / 2));
              },
              [this, &name](const OptionInfo::ModeInfo& v) {
                if (!v.modes.empty())
                {
                  d_solver->setOption(name, v.modes[0]);
                }
              },
          },
          info.valueInfo);
    }
    catch (const Ava6ApiOptionException&)
    {
    }
  }
};

TEST_F(TestBlackOptions, set)
{
  const std::set<std::string> muted{"copyright",
                                    "help",
                                    "show-config",
                                    "show-debug-tags",
                                    "show-trace-tags",
                                    "version"};
  for (const auto& name : options::getNames())
  {

    if (muted.count(name))
    {
      testing::internal::CaptureStdout();
    }
    testSetOption(name);
    if (muted.count(name))
    {
      testing::internal::GetCapturedStdout();
    }
  }
}



TEST_F(TestBlackOptions, getOptionInfoBenchmark)
{
  auto names = options::getNames();
  std::unordered_set<std::string> ignore = {
      "output",
      "quiet",
      "rweight",
      "trace",
      "verbose",
  };
  auto end = std::remove_if(names.begin(), names.end(), [&](const auto& i) {
    return ignore.count(i);
  });
  names.erase(end, names.end());
  size_t ct = 0;
  for (size_t i = 0; i < 1000; ++i)
  {
    for (const auto& name : names)
    {
      ct += d_solver->getOption(name).size();
    }
  }
  std::cout << ct << std::endl;
}

}  // namespace test
}  // namespace ava6::internal
