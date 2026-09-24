/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Output utility classes and functions.
 */

#include "ava6_private_library.h"

#ifndef AVA6__OUTPUT_H
#define AVA6__OUTPUT_H

#include <ava6/ava6_export.h>

#include <algorithm>
#include <cstdio>
#include <ios>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ava6::internal {

template <class T, class U>
std::ostream& operator<<(std::ostream& out, const std::pair<T, U>& p)
{
  return out << "[" << p.first << "," << p.second << "]";
}

/**
 * A utility class to provide (essentially) a "/dev/null" streambuf.
 * If debugging support is compiled in, but debugging for
 * e.g. "parser" is off, then Trace("parser") returns a stream
 * attached to a null_streambuf instance so that output is directed to
 * the bit bucket.
 */
class null_streambuf : public std::streambuf
{
 public:
  /* Overriding overflow() just ensures that EOF isn't returned on the
   * stream.  Perhaps this is not so critical, but recommended; this
   * way the output stream looks like it's functioning, in a non-error
   * state. */
  int overflow(int c) override { return c; }
}; /* class null_streambuf */

/** A null stream-buffer singleton */
extern null_streambuf null_sb;
/** A null output stream singleton */
extern std::ostream null_os AVA6_EXPORT;

class Ava6ostream
{
  static const std::string s_tab AVA6_EXPORT;
  static const int s_indentIosIndex AVA6_EXPORT;

  /** The underlying ostream */
  std::ostream* d_os;
  /** Are we in the first column? */
  bool d_firstColumn;

  /** The endl manipulator (why do we need to keep this?) */
  std::ostream& (*const d_endl)(std::ostream&);

  Ava6ostream(const Ava6ostream&) = delete;
  Ava6ostream& operator=(const Ava6ostream&) = delete;

 public:
  Ava6ostream() : d_os(nullptr), d_firstColumn(false), d_endl(&std::endl) {}
  explicit Ava6ostream(std::ostream* os)
      : d_os(os), d_firstColumn(true), d_endl(&std::endl)
  {
  }

  void reset(std::ostream* os)
  {
    d_os = os;
    d_firstColumn = (os != nullptr);
  }

  void pushIndent()
  {
    if (d_os != nullptr)
    {
      ++d_os->iword(s_indentIosIndex);
    }
  }
  void popIndent()
  {
    if (d_os != nullptr)
    {
      long& indent = d_os->iword(s_indentIosIndex);
      if (indent > 0)
      {
        --indent;
      }
    }
  }

  Ava6ostream& flush()
  {
    if (d_os != nullptr)
    {
      d_os->flush();
    }
    return *this;
  }

  bool isConnected() const { return d_os != nullptr; }
  operator std::ostream&() const { return isConnected() ? *d_os : null_os; }

  std::ostream* getStreamPointer() const { return d_os; }

  template <class T>
  Ava6ostream& operator<<(T const& t)
  {
    if (d_os != nullptr)
    {
      if (d_firstColumn)
      {
        d_firstColumn = false;
        long indent = d_os->iword(s_indentIosIndex);
        for (long i = 0; i < indent; ++i)
        {
          d_os = &(*d_os << s_tab);
        }
      }
      *d_os << t;
    }
    return *this;
  }

  // support manipulators, endl, etc..
  Ava6ostream& operator<<(std::ostream& (*pf)(std::ostream&))
  {
    if (d_os != nullptr)
    {
      d_os = &(*d_os << pf);

      if (pf == d_endl)
      {
        d_firstColumn = true;
      }
    }
    return *this;
  }
  Ava6ostream& operator<<(std::ios& (*pf)(std::ios&))
  {
    if (d_os != nullptr)
    {
      d_os = &(*d_os << pf);
    }
    return *this;
  }
  Ava6ostream& operator<<(std::ios_base& (*pf)(std::ios_base&))
  {
    if (d_os != nullptr)
    {
      d_os = &(*d_os << pf);
    }
    return *this;
  }
  Ava6ostream& operator<<(Ava6ostream& (*pf)(Ava6ostream&))
  {
    return pf(*this);
  }
}; /* class Ava6ostream */

inline Ava6ostream& push(Ava6ostream& stream)
{
  stream.pushIndent();
  return stream;
}

inline Ava6ostream& pop(Ava6ostream& stream)
{
  stream.popIndent();
  return stream;
}

/**
 * Does nothing; designed for compilation of non-debug/non-trace
 * builds.  None of these should ever be called in such builds, but we
 * offer this to the compiler so it doesn't complain.
 */
class NullC
{
  mutable Ava6ostream d_null;

 public:
  NullC() : d_null(nullptr) {}
  operator bool() const { return false; }
  operator Ava6ostream&() const { return d_null; }
  operator std::ostream&() const { return null_os; }
}; /* class NullC */

extern NullC nullStream AVA6_EXPORT;

/** The warning output class */
class WarningC
{
  std::set<std::pair<std::string, size_t> > d_alreadyWarned;
  std::ostream* d_os;

 public:
  explicit WarningC(std::ostream* os) : d_os(os) {}

  Ava6ostream operator()() const { return Ava6ostream(d_os); }

  std::ostream& setStream(std::ostream* os)
  {
    d_os = os;
    return *d_os;
  }
  std::ostream& getStream() const { return *d_os; }
  std::ostream* getStreamPointer() const { return d_os; }

  bool isOn() const { return d_os != &null_os; }

  // This function supports the WarningOnce() macro, which allows you
  // to easily indicate that a warning should be emitted, but only
  // once for a given run of ava6.
  bool warnOnce(const std::string& file, size_t line)
  {
    std::pair<std::string, size_t> pr = std::make_pair(file, line);
    if (d_alreadyWarned.find(pr) != d_alreadyWarned.end())
    {
      // signal caller not to warn again
      return false;
    }

    // okay warn this time, but don't do it again
    d_alreadyWarned.insert(pr);
    return true;
  }

}; /* class WarningC */

/** The trace output class */
class TraceC
{
  std::ostream* d_os;
  std::vector<std::string> d_tags;
  mutable Ava6ostream d_stream;

 public:
  explicit TraceC(std::ostream* os) : d_os(os), d_stream(os) {}

  Ava6ostream& operator()() const
  {
    d_stream.reset(d_os);
    return d_stream;
  }
  Ava6ostream& operator()(const std::string& tag) const
  {
    if (isOn(tag))
    {
      d_stream.reset(d_os);
    }
    else
    {
      d_stream.reset(nullptr);
    }
    return d_stream;
  }

  bool on(const std::string& tag)
  {
    d_tags.emplace_back(tag);
    return true;
  }
  bool off(const std::string& tag)
  {
    auto it = std::find(d_tags.begin(), d_tags.end(), tag);
    if (it != d_tags.end())
    {
      *it = d_tags.back();
      d_tags.pop_back();
    }
    return false;
  }

  bool isOn(const std::string& tag) const
  {
    // This is faster than using std::set::find() or sorting the vector and
    // using std::lower_bound.
    return !d_tags.empty()
           && std::find(d_tags.begin(), d_tags.end(), tag) != d_tags.end();
  }

  std::ostream& setStream(std::ostream* os)
  {
    d_os = os;
    return *d_os;
  }
  std::ostream& getStream() const { return *d_os; }
  std::ostream* getStreamPointer() const { return d_os; }

}; /* class TraceC */

/** The warning output singleton */
extern WarningC WarningChannel AVA6_EXPORT;
/** The trace output singleton */
extern TraceC TraceChannel AVA6_EXPORT;

#ifdef AVA6_MUZZLE

#define Warning                                              \
  ava6::internal::__ava6_true() ? ava6::internal::nullStream \
                                : ava6::internal::WarningChannel
#define WarningOnce                                          \
  ava6::internal::__ava6_true() ? ava6::internal::nullStream \
                                : ava6::internal::WarningChannel
#define TraceIsOn \
  ava6::internal::__ava6_true() ? false : ava6::internal::TraceChannel.isOn
#define Trace(tag)                                           \
  ava6::internal::__ava6_true() ? ava6::internal::nullStream \
                                : ava6::internal::TraceChannel()

#else /* AVA6_MUZZLE */

#define Warning                                                         \
  (!ava6::internal::WarningChannel.isOn()) ? ava6::internal::nullStream \
                                           : ava6::internal::WarningChannel
#define WarningOnce                                                 \
  (!ava6::internal::WarningChannel.isOn()                           \
   || !ava6::internal::WarningChannel.warnOnce(__FILE__, __LINE__)) \
      ? ava6::internal::nullStream                                  \
      : ava6::internal::WarningChannel
#ifdef AVA6_TRACING
#define TraceIsOn ava6::internal::TraceChannel.isOn
#define Trace(tag)                                                     \
  !ava6::internal::TraceChannel.isOn(tag) ? ava6::internal::nullStream \
                                          : ava6::internal::TraceChannel()
#else /* AVA6_TRACING */
#define TraceIsOn \
  ava6::internal::__ava6_true() ? false : ava6::internal::TraceChannel.isOn
#define Trace(tag)                                           \
  ava6::internal::__ava6_true() ? ava6::internal::nullStream \
                                : ava6::internal::TraceChannel()
#endif /* AVA6_TRACING */

#endif /* AVA6_MUZZLE */

// Disallow e.g. !Trace("foo").isOn() forms
// because the ! will apply before the ? .
// If a compiler error has directed you here,
// just parenthesize it e.g. !(Trace("foo").isOn())
class __ava6_true
{
  AVA6_UNUSED void operator!();
  AVA6_UNUSED void operator~();
  AVA6_UNUSED void operator-();
  AVA6_UNUSED void operator+();

 public:
  inline operator bool() { return true; }
}; /* __ava6_true */

/**
 * Pushes an indentation level on construction, pop on destruction.
 * Useful for tracing recursive functions especially, but also can be
 * used for clearly separating different phases of an algorithm,
 * or iterations of a loop, or... etc.
 */
class IndentedScope
{
  Ava6ostream& d_out;

 public:
  inline IndentedScope(Ava6ostream& out) : d_out(out) { d_out << push; }
  inline ~IndentedScope() { d_out << pop; }
}; /* class IndentedScope */

}  // namespace ava6::internal

#endif /* AVA6__OUTPUT_H */
