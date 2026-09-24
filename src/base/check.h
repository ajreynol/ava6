/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Assertion utility classes, functions and macros.
 *
 * Assertion macros assert a condition and aborts() the process if the
 * condition is not satisfied. These macro leave a hanging ostream for the user
 * to specify additional information about the failure.
 *
 * Example usage:
 *   AlwaysAssert(x >= 0) << "x must be positive.";
 *
 * Assert is an AlwaysAssert that is only enabled in debug builds.
 *   Assert(pointer != nullptr);
 *
 * AVA6_FATAL() can be used to indicate unreachable code.
 *
 * Note: The AlwaysAssert and Assert macros are not safe for use in
 *       signal-handling code.
 */

#include "ava6_private_library.h"

#ifndef AVA6__CHECK_H
#define AVA6__CHECK_H

#include <ava6/ava6_export.h>

#include <cstdarg>
#include <ostream>

#include "base/exception.h"

namespace ava6::internal {

// Implementation notes:
// To understand FatalStream and OStreamVoider, it is useful to understand
// how a AlwaysAssert is structured. AlwaysAssert(cond) is roughly the following
// pattern:
//  cond ? (void)0 : OstreamVoider() & FatalStream().stream()
// This is a carefully crafted message to achieve a hanging ostream using
// operator precedence. The line `AlwaysAssert(cond) << foo << bar;` will bind
// as follows:
//  `cond ? ((void)0) : (OSV() & ((FS().stream() << foo) << bar));`
// Once the expression is evaluated, the destructor ~FatalStream() of the
// temporary object is then run, which abort()'s the process. The role of the
// OStreamVoider() is to match the void type of the true branch.

// Class that provides an ostream and whose destructor aborts! Direct usage of
// this class is discouraged.
class AVA6_EXPORT FatalStream
{
 public:
  FatalStream(const char* function, const char* file, int line);
  [[noreturn]] ~FatalStream();

  std::ostream& stream();

 private:
  void Flush();
};

// Helper class that changes the type of an std::ostream& into a void. See
// "Implementation notes" for more information.
class OstreamVoider
{
 public:
  OstreamVoider() {}
  // The operator precedence between operator& and operator<< is critical here.
  void operator&(std::ostream&) {}
};

// AVA6_FATAL() always aborts a function and provides a convenient way of
// formatting error messages. This can be used instead of a return type.
//
// Example function that returns a type Foo:
//   Foo bar(T t) {
//     switch(t.type()) {
//     ...
//     default:
//       AVA6_FATAL() << "Unknown T type " << t.enum();
//     }
//   }
#define AVA6_FATAL() \
  internal::FatalStream(__PRETTY_FUNCTION__, __FILE__, __LINE__).stream()

/* GCC <= 9.2 ignores AVA6_NO_RETURN of ~FatalStream() if
 * used in template classes (e.g., CDHashMap::save()).  As a workaround we
 * explicitly call abort() to let the compiler know that the
 * corresponding function call will not return. */
#define SuppressWrongNoReturnWarning abort()

// If `cond` is true, log an error message and abort the process.
// Otherwise, does nothing. This leaves a hanging std::ostream& that can be
// inserted into.
#define AVA6_FATAL_IF(cond, function, file, line) \
  AVA6_PREDICT_FALSE(!(cond))                     \
  ? (void)0                                       \
  : ava6::internal::OstreamVoider()               \
          & ava6::internal::FatalStream(function, file, line).stream()

// If `cond` is false, log an error message and abort()'s the process.
// Otherwise, does nothing. This leaves a hanging std::ostream& that can be
// inserted into using operator<<. Example usages:
//   AlwaysAssert(x >= 0);
//   AlwaysAssert(x >= 0) << "x must be positive";
//   AlwaysAssert(x >= 0) << "expected a positive value. Got " << x << "
//   instead";
#define AlwaysAssert(cond)                                        \
  AVA6_FATAL_IF(!(cond), __PRETTY_FUNCTION__, __FILE__, __LINE__) \
      << "Check failure\n\n " << #cond << "\n"

// Assert is a variant of AlwaysAssert() that is only checked when
// AVA6_ASSERTIONS is defined. We rely on the optimizer to remove the deadcode.
#ifdef AVA6_ASSERTIONS
#define Assert(cond) AlwaysAssert(cond)
#else
#define Assert(cond) \
  AVA6_FATAL_IF(false, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

// AVA6_EQUAL(lhs, rhs) checks if two expressions are equal while forcing
// a left-to-right evaluation order. This is essential when expressions
// have side effects (e.g., generating new node IDs) and a predictable,
// deterministic evaluation order is required.
#define AVA6_EQUAL(lhs, rhs) \
  ([&] {                     \
    auto _l = (lhs);         \
    auto _r = (rhs);         \
    return _l == _r;         \
  }())

// AssertEqual(lhs, rhs) asserts that two expressions are equal, forcing
// a left-to-right evaluation order.
// Refer to AVA6_EQUAL for specific use cases involving side effects.
#define AssertEqual(lhs, rhs) Assert(AVA6_EQUAL(lhs, rhs))

// DebugUnhandled() triggers an assertion failure (when AVA6_ASSERTIONS is
// enabled) to flag potential unhandled code paths. When running under
// the Clang Static Analyzer, it becomes a no-op so the analyzer can continue
// exploring the production control flow.
#if defined(__clang_analyzer__)
#define DebugUnhandled() Assert(true)
#else
#define DebugUnhandled() Assert(false)
#endif

class AssertArgumentException : public Exception
{
 protected:
  AssertArgumentException() : Exception() {}

  void construct(const char* header,
                 const char* extra,
                 const char* function,
                 const char* file,
                 unsigned line,
                 const char* fmt,
                 va_list args);

  void construct(const char* header,
                 const char* extra,
                 const char* function,
                 const char* file,
                 unsigned line);

 public:
  AssertArgumentException(const char* condStr,
                          const char* argDesc,
                          const char* function,
                          const char* file,
                          unsigned line,
                          const char* fmt,
                          ...);

  AssertArgumentException(const char* condStr,
                          const char* argDesc,
                          const char* function,
                          const char* file,
                          unsigned line);

}; /* class AssertArgumentException */

#define Unreachable() AVA6_FATAL() << "Unreachable code reached "

#define Unhandled() AVA6_FATAL() << "Unhandled case encountered "

#define Unimplemented() AVA6_FATAL() << "Unimplemented code encountered "

#define InternalError() AVA6_FATAL() << "Internal error detected "

#define IllegalArgument(arg, msg...)              \
  throw ava6::internal::IllegalArgumentException( \
      "",                                         \
      #arg,                                       \
      __PRETTY_FUNCTION__,                        \
      ava6::internal::IllegalArgumentException::formatVariadic(msg).c_str());
// This cannot use check argument directly as this forces
// CheckArgument to use a va_list. This is unsupported in Swig.
#define PrettyCheckArgument(cond, arg, msg...)                          \
  do                                                                    \
  {                                                                     \
    if (__builtin_expect((!(cond)), false))                             \
    {                                                                   \
      throw ava6::internal::IllegalArgumentException(                   \
          #cond,                                                        \
          #arg,                                                         \
          __PRETTY_FUNCTION__,                                          \
          ava6::internal::IllegalArgumentException::formatVariadic(msg) \
              .c_str());                                                \
    }                                                                   \
  } while (0)
#define AlwaysAssertArgument(cond, arg, msg...)                         \
  do                                                                    \
  {                                                                     \
    if (__builtin_expect((!(cond)), false))                             \
    {                                                                   \
      throw ava6::internal::AssertArgumentException(                    \
          #cond, #arg, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##msg); \
    }                                                                   \
  } while (0)

#ifdef AVA6_ASSERTIONS
#define AssertArgument(cond, arg, msg...) AlwaysAssertArgument(cond, arg, ##msg)
#define DebugCheckArgument(cond, arg, msg...) CheckArgument(cond, arg, ##msg)
#else                                     /* ! AVA6_ASSERTIONS */
#define AssertArgument(cond, arg, msg...) /*__builtin_expect( ( cond ), true \
                                             )*/
#define DebugCheckArgument( \
    cond, arg, msg...) /*__builtin_expect( ( cond ), true )*/
#endif                 /* AVA6_ASSERTIONS */

}  // namespace ava6::internal

#endif /* AVA6__CHECK_H */
