/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Check macros for the ava6 C++ API.
 *
 * These macros implement guards for the ava6 C++ API functions.
 */

#include "ava6_private.h"

#ifndef AVA6__API__CHECKS_H
#define AVA6__API__CHECKS_H

#include <ava6/ava6.h>

#include <sstream>

#include "base/modal_exception.h"
#include "options/option_exception.h"

namespace ava6 {

#define AVA6_API_TRY_CATCH_BEGIN \
  try                            \
  {
#define AVA6_API_TRY_CATCH_END                         \
  }                                                    \
  catch (const internal::OptionException& e)           \
  {                                                    \
    throw Ava6ApiOptionException(e.getMessage());      \
  }                                                    \
  catch (const internal::RecoverableModalException& e) \
  {                                                    \
    throw Ava6ApiRecoverableException(e.getMessage()); \
  }                                                    \
  catch (const internal::Exception& e)                 \
  {                                                    \
    throw Ava6ApiException(e.getMessage());            \
  }                                                    \
  catch (const std::invalid_argument& e) { throw Ava6ApiException(e.what()); }

/* -------------------------------------------------------------------------- */
/* API guard helpers                                                          */
/* -------------------------------------------------------------------------- */

class Ava6ApiExceptionStream
{
 public:
  Ava6ApiExceptionStream() {}
  /* Note: This needs to be explicitly set to 'noexcept(false)' since it is
   * a destructor that throws an exception and in C++11 all destructors
   * default to noexcept(true) (else this triggers a call to std::terminate). */
  ~Ava6ApiExceptionStream() noexcept(false)
  {
    if (std::uncaught_exceptions() == 0)
    {
      throw Ava6ApiException(d_stream.str());
    }
  }

  std::ostream& ostream() { return d_stream; }

 private:
  std::stringstream d_stream;
};

class Ava6ApiRecoverableExceptionStream
{
 public:
  Ava6ApiRecoverableExceptionStream() {}
  /* Note: This needs to be explicitly set to 'noexcept(false)' since it is
   * a destructor that throws an exception and in C++11 all destructors
   * default to noexcept(true) (else this triggers a call to std::terminate). */
  ~Ava6ApiRecoverableExceptionStream() noexcept(false)
  {
    if (std::uncaught_exceptions() == 0)
    {
      throw Ava6ApiRecoverableException(d_stream.str());
    }
  }

  std::ostream& ostream() { return d_stream; }

 private:
  std::stringstream d_stream;
};

class Ava6ApiUnsupportedExceptionStream
{
 public:
  Ava6ApiUnsupportedExceptionStream() {}
  /* Note: This needs to be explicitly set to 'noexcept(false)' since it is
   * a destructor that throws an exception and in C++11 all destructors
   * default to noexcept(true) (else this triggers a call to std::terminate). */
  ~Ava6ApiUnsupportedExceptionStream() noexcept(false)
  {
    if (std::uncaught_exceptions() == 0)
    {
      throw Ava6ApiUnsupportedException(d_stream.str());
    }
  }

  std::ostream& ostream() { return d_stream; }

 private:
  std::stringstream d_stream;
};

/* -------------------------------------------------------------------------- */
/* Basic check macros.                                                        */
/* -------------------------------------------------------------------------- */

/**
 * The base check macro.
 * Throws a Ava6ApiException if 'cond' is false.
 */
#define AVA6_API_CHECK(cond) \
  AVA6_PREDICT_TRUE(cond)    \
  ? (void)0                  \
  : ava6::internal::OstreamVoider() & ava6::Ava6ApiExceptionStream().ostream()

/**
 * The base check macro for throwing recoverable exceptions.
 * Throws a Ava6ApiRecoverableException if 'cond' is false.
 */
#define AVA6_API_RECOVERABLE_CHECK(cond) \
  AVA6_PREDICT_TRUE(cond)                \
  ? (void)0                              \
  : ava6::internal::OstreamVoider()      \
          & ava6::Ava6ApiRecoverableExceptionStream().ostream()

/**
 * The base check macro for throwing unsupported exceptions.
 * Throws a Ava6ApiUnsupportedException if 'cond' is false.
 */
#define AVA6_API_UNSUPPORTED_CHECK(cond) \
  AVA6_PREDICT_TRUE(cond)                \
  ? (void)0                              \
  : ava6::internal::OstreamVoider()      \
          & ava6::Ava6ApiUnsupportedExceptionStream().ostream()

/* -------------------------------------------------------------------------- */
/* Not null checks.                                                           */
/* -------------------------------------------------------------------------- */

/** Check it 'this' is not a null object. */
#define AVA6_API_CHECK_NOT_NULL                     \
  AVA6_API_CHECK(!isNullHelper())                   \
      << "invalid call to '" << __PRETTY_FUNCTION__ \
      << "', expected non-null object"

/** Check if given argument is not a null object. */
#define AVA6_API_ARG_CHECK_NOT_NULL(arg) \
  AVA6_API_CHECK(!arg.isNull()) << "invalid null argument for '" << #arg << "'";

/** Check if given argument is not a null pointer. */
#define AVA6_API_ARG_CHECK_NOT_NULLPTR(arg) \
  AVA6_API_CHECK(arg != nullptr) << "invalid null argument for '" << #arg << "'"
/**
 * Check if given argument at given index in container 'args' is not a null
 * object.
 */
#define AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL(what, arg, args, idx)      \
  AVA6_API_CHECK(!arg.isNull()) << "invalid null " << (what) << " in '" \
                                << #args << "' at index " << (idx)

/* -------------------------------------------------------------------------- */
/* Kind checks.                                                               */
/* -------------------------------------------------------------------------- */

/** Check if given kind is a valid kind. */
#define AVA6_API_KIND_CHECK(kind)     \
  AVA6_API_CHECK(isDefinedKind(kind)) \
      << "invalid kind '" << std::to_string(kind) << "'"

/**
 * Check if given kind is a valid kind.
 * Creates a stream to provide a message that identifies what kind was expected
 * if given kind is invalid.
 */
#define AVA6_API_KIND_CHECK_EXPECTED(cond, kind) \
  AVA6_PREDICT_TRUE(cond)                        \
  ? (void)0                                      \
  : ava6::internal::OstreamVoider()              \
          & Ava6ApiExceptionStream().ostream()   \
                << "invalid kind '" << std::to_string(kind) << "', expected "

/* -------------------------------------------------------------------------- */
/* Argument checks.                                                           */
/* -------------------------------------------------------------------------- */

/**
 * Check condition 'cond' for given argument 'arg'.
 * Creates a stream to provide a message that identifies what was expected to
 * hold if condition is false and throws a non-recoverable exception.
 */
#define AVA6_API_ARG_CHECK_EXPECTED(cond, arg)                      \
  AVA6_PREDICT_TRUE(cond)                                           \
  ? (void)0                                                         \
  : ava6::internal::OstreamVoider()                                 \
          & Ava6ApiExceptionStream().ostream()                      \
                << "invalid argument '" << arg << "' for '" << #arg \
                << "', expected "

/**
 * Check condition 'cond' for given argument 'arg'.
 * Creates a stream to provide a message that identifies what was expected to
 * hold if condition is false and throws a recoverable exception.
 */
#define AVA6_API_RECOVERABLE_ARG_CHECK_EXPECTED(cond, arg)          \
  AVA6_PREDICT_TRUE(cond)                                           \
  ? (void)0                                                         \
  : ava6::internal::OstreamVoider()                                 \
          & Ava6ApiRecoverableExceptionStream().ostream()           \
                << "invalid argument '" << arg << "' for '" << #arg \
                << "', expected "

/**
 * Check condition 'cond' for given argument 'arg'.
 * Provides a more specific error message than AVA6_API_ARG_CHECK_EXPECTED,
 * it identifies that this check is a size check.
 * Creates a stream to provide a message that identifies what was expected to
 * hold if condition is false and throws a recoverable exception.
 */
#define AVA6_API_ARG_SIZE_CHECK_EXPECTED(cond, arg) \
  AVA6_PREDICT_TRUE(cond)                           \
  ? (void)0                                         \
  : ava6::internal::OstreamVoider()                 \
          & Ava6ApiExceptionStream().ostream()      \
                << "invalid size of argument '" << #arg << "', expected "

/**
 * Check condition 'cond' for the argument at given index in container 'args'.
 * Argument 'what' identifies what is being checked (e.g., "term").
 * Creates a stream to provide a message that identifies what was expected to
 * hold if condition is false.
 * Usage:
 *   AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(
 *     <condition>, "what", <container>, <idx>) << "message";
 */
#define AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(cond, what, args, idx)          \
  AVA6_PREDICT_TRUE(cond)                                                    \
  ? (void)0                                                                  \
  : ava6::internal::OstreamVoider()                                          \
          & Ava6ApiExceptionStream().ostream()                               \
                << "invalid " << (what) << " in '" << #args << "' at index " \
                << (idx) << ", expected "

/**
 * Check condition 'cond' for given operator index `index` in list of indices
 * `args`. Creates a stream to provide a message that identifies what was
 * expected to hold if condition is false and throws a non-recoverable
 * exception.
 */
#define AVA6_API_CHECK_OP_INDEX(cond, args, index)                            \
  AVA6_PREDICT_TRUE(cond)                                                     \
  ? (void)0                                                                   \
  : ava6::internal::OstreamVoider()                                           \
          & Ava6ApiExceptionStream().ostream()                                \
                << "invalid value '" << args[index] << "' at index " << index \
                << " for operator, expected "

/* -------------------------------------------------------------------------- */
/* Term manager check.                                                        */
/* -------------------------------------------------------------------------- */

/**
 * Term manager check for member functions of classes other than class Solver.
 * Check if given term manager matches the term manager this solver object is
 * associated with.
 */
#define AVA6_API_ARG_CHECK_TM(what, arg)                  \
  AVA6_API_CHECK(d_nm == arg.d_nm)                        \
      << "Given " << (what)                               \
      << " is not associated with the term manager this " \
      << "object is associated with"

/* -------------------------------------------------------------------------- */
/* Sort checks.                                                               */
/* -------------------------------------------------------------------------- */

/**
 * Sort check for member functions of classes other than class Solver.
 * Check if given sort is not null and associated with the term manager this
 * object is associated with.
 */
#define AVA6_API_CHECK_SORT(sort)        \
  do                                     \
  {                                      \
    AVA6_API_ARG_CHECK_NOT_NULL(sort);   \
    AVA6_API_ARG_CHECK_TM("sort", sort); \
  } while (0)

/**
 * Sort check for member functions of classes other than class Solver.
 * Check if each sort in the given container of sorts is not null and
 * associated with the term manager this object is associated with.
 */
#define AVA6_API_CHECK_SORTS(sorts)                                           \
  do                                                                          \
  {                                                                           \
    size_t i = 0;                                                             \
    for (const auto& s : sorts)                                               \
    {                                                                         \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("sort", s, sorts, i);              \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(d_nm == s.d_nm, "sort", sorts, i)  \
          << "a sort associated with term manager this object is associated " \
             "with";                                                          \
      i += 1;                                                                 \
    }                                                                         \
  } while (0)

/**
 * Sort check for member functions of classes other than class Solver.
 * Check if each sort in the given container of sorts is not null, is
 * associated with the term manager this object is associated with, and is a
 * first-class sort.
 */
#define AVA6_API_CHECK_DOMAIN_SORTS(sorts)                                   \
  do                                                                         \
  {                                                                          \
    size_t i = 0;                                                            \
    for (const auto& s : sorts)                                              \
    {                                                                        \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("sort", s, sorts, i);             \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(d_nm == s.d_nm, "sort", sorts, i) \
          << "a sort associated with the term manager this object is "       \
             "associated "                                                   \
             "with";                                                         \
      AVA6_API_ARG_CHECK_EXPECTED(s.getTypeNode().isFirstClass() && !s.isFunction(), s)         \
          << "first-class sort as domain sort";                              \
      i += 1;                                                                \
    }                                                                        \
  } while (0)

/* -------------------------------------------------------------------------- */
/* Term checks.                                                               */
/* -------------------------------------------------------------------------- */

/**
 * Term check for member functions of classes other than class Solver.
 * Check if given term is not null and associated with the term manager this
 * object is associated with.
 */
#define AVA6_API_CHECK_TERM(term)        \
  do                                     \
  {                                      \
    AVA6_API_ARG_CHECK_NOT_NULL(term);   \
    AVA6_API_ARG_CHECK_TM("term", term); \
  } while (0)

/**
 * Term check for member functions of classes other than class Solver.
 * Check if each term in the given container of terms is not null and
 * associated with the term manager this object is associated with.
 */
#define AVA6_API_CHECK_TERMS(terms)                                          \
  do                                                                         \
  {                                                                          \
    size_t i = 0;                                                            \
    for (const auto& s : terms)                                              \
    {                                                                        \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("term", s, terms, i);             \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(d_nm == s.d_nm, "term", terms, i) \
          << "a term associated with the term manager this object is "       \
             "associated "                                                   \
             "with";                                                         \
      i += 1;                                                                \
    }                                                                        \
  } while (0)

/**
 * Term check for member functions of classes other than class Solver.
 * Check if each term and sort in the given map (which maps terms to sorts) is
 * not null and associated with the term manager this object is associated
 * with.
 */
#define AVA6_API_CHECK_TERMS_MAP(map)                                  \
  do                                                                   \
  {                                                                    \
    size_t i = 0;                                                      \
    for (const auto& p : map)                                          \
    {                                                                  \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("term", p.first, map, i);   \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                            \
          d_nm == p.first.d_nm, "term", map, i)                        \
          << "a term associated with the term manager this object is " \
             "associated "                                             \
             "with";                                                   \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("sort", p.second, map, i);  \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                            \
          d_nm == p.second.d_nm, "sort", map, i)                       \
          << "a sort associated with the term manager this object is " \
             "associated "                                             \
             "with";                                                   \
      i += 1;                                                          \
    }                                                                  \
  } while (0)

/**
 * Term check for member functions of classes other than class Solver.
 * Check if each term in the given container is not null, associated with the
 * term manager object this object is associated with, and of the given sort.
 */
#define AVA6_API_CHECK_TERMS_WITH_SORT(terms, sort)                            \
  do                                                                           \
  {                                                                            \
    size_t i = 0;                                                              \
    for (const auto& t : terms)                                                \
    {                                                                          \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("term", t, terms, i);               \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(d_nm == t.d_nm, "term", terms, i)   \
          << "a term associated with the term manager this object is "         \
             "associated "                                                     \
             "with";                                                           \
      AVA6_API_CHECK(AVA6_EQUAL(t.getSort(), sort))                            \
          << "Expected term with sort " << sort << " at index " << i << " in " \
          << #terms;                                                           \
      i += 1;                                                                  \
    }                                                                          \
  } while (0)

/**
 * Term check for member functions of classes other than class Solver.
 * Check if each term in both the given container is not null, associated with
 * the term manager this object is associated with, and their sorts are
 * pairwise equal.
 */
#define AVA6_API_TERM_CHECK_TERMS_WITH_TERMS_SORT_EQUAL_TO(terms1, terms2)     \
  do                                                                           \
  {                                                                            \
    size_t i = 0;                                                              \
    for (const auto& t1 : terms1)                                              \
    {                                                                          \
      const auto& t2 = terms2[i];                                              \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("term", t1, terms1, i);             \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(d_nm == t1.d_nm, "term", terms1, i) \
          << "a term associated with the term manager this object is "         \
             "associated "                                                     \
             "with";                                                           \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("term", t2, terms2, i);             \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(d_nm == t2.d_nm, "term", terms2, i) \
          << "a term associated with the term manager this object is "         \
             "associated "                                                     \
             "with";                                                           \
      AVA6_API_CHECK(AVA6_EQUAL(t1.getSort(), t2.getSort()))                   \
          << "expecting terms of the same sort at index " << i;                \
      i += 1;                                                                  \
    }                                                                          \
  } while (0)

/* -------------------------------------------------------------------------- */
/* DatatypeDecl checks.                                                       */
/* -------------------------------------------------------------------------- */

/**
 * DatatypeDecl check for member functions of classes other than class Solver.
 * Check if given datatype declaration is not null and associated with the
 * term manager this DatatypeDecl object is associated with.
 */
#define AVA6_API_CHECK_DTDECL(decl)                                      \
  do                                                                     \
  {                                                                      \
    AVA6_API_ARG_CHECK_NOT_NULL(decl);                                   \
    AVA6_API_CHECK(d_nm == decl.d_nm)                                    \
        << "Given datatype declaration is not associated with the term " \
           "manager this "                                               \
        << "object is associated with";                                  \
  } while (0)

/* -------------------------------------------------------------------------- */
/* Checks for class TermManager.                                              */
/* -------------------------------------------------------------------------- */

/**
 * Term manager check for member functions of class TermManager.
 * Check if given term manager matches the term manager this term manager.
 */
#define AVA6_API_ARG_TM_CHECK_TM(what, arg) \
  AVA6_API_CHECK(d_nm == arg.d_nm)          \
      << "Given " << (what) << " is not associated with this term manager"
/**
 * Sort check for member functions of class TermManager.
 * Check if given sort is not null and associated with this term manager.
 */
#define AVA6_API_TM_CHECK_SORT(sort)        \
  do                                        \
  {                                         \
    AVA6_API_ARG_CHECK_NOT_NULL(sort);      \
    AVA6_API_ARG_TM_CHECK_TM("sort", sort); \
  } while (0)

/**
 * Sort check for member functions of class TermManager.
 * Check if given sort at given index of given sorts is not null and associated
 * with this term manager.
 */
#define AVA6_API_TM_CHECK_SORT_AT_INDEX(sort, sorts, index)           \
  do                                                                  \
  {                                                                   \
    AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("sort", sort, sorts, index); \
    AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                             \
        d_nm == sort.d_nm, "sort", sorts, index)                      \
        << "a sort associated with this term manager";                \
  } while (0)

/**
 * Sort checks for member functions of class TermManager.
 * Check if each sort in the given container of sorts is not null and
 * associated with the term manager of this solver.
 */
#define AVA6_API_TM_CHECK_SORTS(sorts)              \
  do                                                \
  {                                                 \
    size_t i = 0;                                   \
    for (const auto& s : sorts)                     \
    {                                               \
      AVA6_API_TM_CHECK_SORT_AT_INDEX(s, sorts, i); \
      i += 1;                                       \
    }                                               \
  } while (0)

/**
 * Domain sort checks for member functions of class TermManager.
 * Check if each domain sort in the given container of sorts is not null,
 * associated with this term manager, and a first-class sort.
 */
#define AVA6_API_TM_CHECK_DOMAIN_SORTS(sorts)                           \
  do                                                                    \
  {                                                                     \
    size_t i = 0;                                                       \
    for (const auto& s : sorts)                                         \
    {                                                                   \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("domain sort", s, sorts, i); \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                             \
          d_nm == s.d_nm, "domain sort", sorts, i)                      \
          << "a sort associated with this term manager";                \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                             \
          s.getTypeNode().isFirstClass() && !s.isFunction(), "domain sort", sorts, i)      \
          << "first-class sort as domain sort";                         \
      i += 1;                                                           \
    }                                                                   \
  } while (0)

/**
 * Domain sort check for member functions of class TermManager.
 * Check if domain sort is not null, associated with this term manager, and a
 * first-class sort.
 */
#define AVA6_API_TM_CHECK_DOMAIN_SORT(sort)                                   \
  do                                                                          \
  {                                                                           \
    AVA6_API_ARG_CHECK_NOT_NULL(sort);                                        \
    AVA6_API_CHECK(d_nm == sort.d_nm) << "Given sort is not associated with " \
                                         "this term manager";                 \
    AVA6_API_ARG_CHECK_EXPECTED(sort.getTypeNode().isFirstClass() && !sort.isFunction(), sort)      \
        << "first-class sort as domain sort";                                 \
  } while (0)

/**
 * Codomain sort check for member functions of class TermManager.
 * Check if codomain sort is not null, associated with this term manager, and a
 * first-class, non-function sort.
 */
#define AVA6_API_TM_CHECK_CODOMAIN_SORT(sort)                                 \
  do                                                                          \
  {                                                                           \
    AVA6_API_ARG_CHECK_NOT_NULL(sort);                                        \
    AVA6_API_CHECK(d_nm == sort.d_nm) << "Given sort is not associated with " \
                                         "this term manager";                 \
    AVA6_API_ARG_CHECK_EXPECTED(!sort.isFunction(), sort)                     \
        << "non-function sort as codomain sort";                              \
  } while (0)

/**
 * Op checks for member functions of class TermManager.
 * Check if given operator is not null and associated with this term manager.
 */
#define AVA6_API_TM_CHECK_OP(op)                                           \
  do                                                                       \
  {                                                                        \
    AVA6_API_ARG_CHECK_NOT_NULL(op);                                       \
    AVA6_API_CHECK(d_nm == op.d_nm) << "Given operator is not associated " \
                                       "with this term manager";           \
  } while (0)

/**
 * Term check for member functions of class TermManager.
 * Check if given term is not null and associated with this term manager.
 */
#define AVA6_API_TM_CHECK_TERM(term)        \
  do                                        \
  {                                         \
    AVA6_API_ARG_CHECK_NOT_NULL(term);      \
    AVA6_API_ARG_TM_CHECK_TM("term", term); \
  } while (0)

/**
 * Term checks for member functions of class TermManager.
 * Check if given term 't' (which is stored at index 'idx' of 'terms') is not
 * null and associated with this term manager.
 */
#define AVA6_API_TM_CHECK_TERM_AT_INDEX(t, terms, idx)                       \
  do                                                                         \
  {                                                                          \
    AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("term", t, terms, idx);             \
    AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(d_nm == t.d_nm, "term", terms, idx) \
        << "a term associated with this term manager";                       \
  } while (0)

/**
 * Term checks for member functions of class TermManager.
 * Check if each term in the given container of terms is not null and
 * associated with this term manager.
 */
#define AVA6_API_TM_CHECK_TERMS(terms)                   \
  for (size_t i = 0, size = terms.size(); i < size; ++i) \
  {                                                      \
    AVA6_API_TM_CHECK_TERM_AT_INDEX(terms[i], terms, i); \
  }

/**
 * DatatypeDecl checks for member functions of class TermManager.
 * Check if given datatype declaration is not null and associated with this
 * term manager.
 */
#define AVA6_API_TM_CHECK_DTDECL(decl)                                    \
  do                                                                      \
  {                                                                       \
    AVA6_API_ARG_CHECK_NOT_NULL(decl);                                    \
    AVA6_API_ARG_TM_CHECK_TM("datatype declaration", decl);               \
    AVA6_API_CHECK(!decl.isResolved())                                    \
        << "Given datatype declaration is already resolved (has already " \
        << "been used to create a datatype sort)";                        \
    AVA6_API_ARG_CHECK_EXPECTED(                                          \
        dtypedecl.getDatatype().getNumConstructors() > 0, dtypedecl)      \
        << "a datatype declaration with at least one constructor";        \
  } while (0)

/**
 * DatatypeDecl checks for member functions of class TermManager.
 * Check if each datatype declaration in the given container of declarations is
 * not null and associated with this term manager.
 */
#define AVA6_API_TM_CHECK_DTDECLS(decls)                                 \
  do                                                                     \
  {                                                                      \
    size_t i = 0;                                                        \
    for (const auto& d : decls)                                          \
    {                                                                    \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL(                              \
          "datatype declaration", d, decls, i);                          \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                              \
          d_nm == d.d_nm, "datatype declaration", decls, i)              \
          << "a datatype declaration associated with this term manager"; \
      AVA6_API_CHECK(!d.isResolved())                                    \
          << "Given datatype declaration is already resolved (has "      \
          << "already been used to create a datatype sort)";             \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                              \
          d.getDatatype().getNumConstructors() > 0,                      \
          "datatype declaration",                                        \
          decls,                                                         \
          i)                                                             \
          << "a datatype declaration with at least one constructor";     \
      i += 1;                                                            \
    }                                                                    \
  } while (0)

/* -------------------------------------------------------------------------- */
/* Checks for class Solver.                                                   */
/* -------------------------------------------------------------------------- */

/* Sort checks. ------------------------------------------------------------- */

/**
 * Sort checks for member functions of class Solver.
 * Check if given sort is not null and associated with the term manager of this
 * solver.
 */
#define AVA6_API_SOLVER_CHECK_SORT(sort)        \
  do                                            \
  {                                             \
    AVA6_API_ARG_CHECK_NOT_NULL(sort);          \
    AVA6_API_CHECK(d_tm.d_nm == sort.d_nm)      \
        << "Given sort is not associated with " \
           "the term manager of this solver";   \
  } while (0)

/**
 * Sort checks for member functions of class Solver.
 * Check if each sort in the given container of sorts is not null and
 * associated with the term manager of this solver.
 */
#define AVA6_API_SOLVER_CHECK_SORTS(sorts)                             \
  do                                                                   \
  {                                                                    \
    size_t i = 0;                                                      \
    for (const auto& s : sorts)                                        \
    {                                                                  \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("sorts", s, sorts, i);      \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                            \
          d_tm.d_nm == s.d_nm, "sort", sorts, i)                       \
          << "a sort associated with the term manager of this solver"; \
      i += 1;                                                          \
    }                                                                  \
  } while (0)

/**
 * Domain sort checks for member functions of class Solver.
 * Check if each domain sort in the given container of sorts is not null,
 * associated with the term manager of this solver, and a first-class sort.
 */
#define AVA6_API_SOLVER_CHECK_DOMAIN_SORTS(sorts)                             \
  do                                                                          \
  {                                                                           \
    size_t i = 0;                                                             \
    for (const auto& s : sorts)                                               \
    {                                                                         \
      AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("domain sort", s, sorts, i);       \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                                   \
          d_tm.d_nm == s.d_nm, "domain sort", sorts, i)                       \
          << "a sort associated with the term manager of this solver object"; \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                                   \
          s.getTypeNode().isFirstClass() && !s.isFunction(), "domain sort", sorts, i)            \
          << "first-class sort as domain sort";                               \
      i += 1;                                                                 \
    }                                                                         \
  } while (0)

/**
 * Codomain sort check for member functions of class Solver.
 * Check if codomain sort is not null, associated with the term manager of this
 * solver, and a first-class, non-function sort.
 */
#define AVA6_API_SOLVER_CHECK_CODOMAIN_SORT(sort)         \
  do                                                      \
  {                                                       \
    AVA6_API_ARG_CHECK_NOT_NULL(sort);                    \
    AVA6_API_CHECK(d_tm.d_nm == sort.d_nm)                \
        << "Given sort is not associated with "           \
           "the term manager of this solver";             \
    AVA6_API_ARG_CHECK_EXPECTED(!sort.isFunction(), sort) \
        << "non-function sort as codomain sort";          \
  } while (0)

/* Term checks. ------------------------------------------------------------- */

/**
 * Term checks for member functions of class Solver.
 * Check if given term is not null and associated with the term manager of this
 * solver.
 */
#define AVA6_API_SOLVER_CHECK_TERM(term)        \
  do                                            \
  {                                             \
    AVA6_API_ARG_CHECK_NOT_NULL(term);          \
    AVA6_API_CHECK(d_tm.d_nm == term.d_nm)      \
        << "Given term is not associated with " \
           "the term manager of this solver";   \
  } while (0)

/**
 * Term checks for member functions of class Solver.
 * Check if given term 't' (which is stored at index 'idx' of 'terms') is not
 * null and associated with the term manager of this solver.
 */
#define AVA6_API_SOLVER_CHECK_TERM_AT_INDEX(t, terms, idx)           \
  do                                                                 \
  {                                                                  \
    AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL("term", t, terms, idx);     \
    AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                            \
        d_tm.d_nm == t.d_nm, "term", terms, idx)                     \
        << "a term associated with the term manager of this solver"; \
  } while (0)

/**
 * Term checks for member functions of class Solver.
 * Check if each term in the given container of terms is not null and
 * associated with the term manager of this solver.
 */
#define AVA6_API_SOLVER_CHECK_TERMS(terms)              \
  do                                                    \
  {                                                     \
    size_t i = 0;                                       \
    for (const auto& t : terms)                         \
    {                                                   \
      AVA6_API_SOLVER_CHECK_TERM_AT_INDEX(t, terms, i); \
      i += 1;                                           \
    }                                                   \
  } while (0)

/**
 * Term checks for member functions of class Solver.
 * Check if given term is not null, associated with the term manager of this
 * solver, and of given sort.
 */
#define AVA6_API_SOLVER_CHECK_TERM_WITH_SORT(term, sort) \
  do                                                     \
  {                                                      \
    AVA6_API_SOLVER_CHECK_TERM(term);                    \
    AVA6_API_CHECK(AVA6_EQUAL(term.getSort(), sort))     \
        << "Expected term with sort " << sort;           \
  } while (0)

/**
 * Term checks for member functions of class Solver.
 * Check if each term in the given container is not null, associated with the
 * term manager of this solver, and of the given sort.
 */
#define AVA6_API_SOLVER_CHECK_TERMS_WITH_SORT(terms, sort)                   \
  for (size_t i = 0, size = terms.size(); i < size; ++i)                     \
  {                                                                          \
    AVA6_API_SOLVER_CHECK_TERM_AT_INDEX(terms[i], terms, i);                 \
    AVA6_API_CHECK(AVA6_EQUAL(terms[i].getSort(), sort))                     \
        << "Expected term with sort " << sort << " at index " << i << " in " \
        << #terms;                                                           \
  }

/**
 * Bound variable checks for member functions of class Solver.
 * Check if given term 'bv' (which is stored at index 'idx' of 'bound_vars') is
 * not null, associated with the term manager of this solver, and a bound
 * variable.
 */
#define AVA6_API_SOLVER_CHECK_BOUND_VAR_AT_INDEX(bv, bound_vars, idx) \
  do                                                                  \
  {                                                                   \
    AVA6_API_SOLVER_CHECK_TERM_AT_INDEX(bv, bound_vars, idx);         \
    AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                             \
        bv.d_node->getKind() == ava6::internal::Kind::BOUND_VARIABLE, \
        "bound variable",                                             \
        bound_vars,                                                   \
        idx)                                                          \
        << "a bound variable";                                        \
  } while (0)

/**
 * Bound variable checks for member functions of class Solver.
 * Check if each term in the given container is not null, associated with the
 * term manager of this solver, and a bound variable.
 */
#define AVA6_API_SOLVER_CHECK_BOUND_VARS(bound_vars)                        \
  for (size_t i = 0, size = bound_vars.size(); i < size; ++i)               \
  {                                                                         \
    AVA6_API_SOLVER_CHECK_BOUND_VAR_AT_INDEX(bound_vars[i], bound_vars, i); \
  }

/**
 * Additional bound variable checks for member functions of class Solver that
 * define functions.
 * Check if each term in the given container matches the corresponding sort in
 * 'domain_sorts', and is a first-class term.
 */
#define AVA6_API_SOLVER_CHECK_BOUND_VARS_DEF_FUN_SORTS(bound_vars,            \
                                                       domain_sorts)          \
  do                                                                          \
  {                                                                           \
    size_t size = bound_vars.size();                                          \
    AVA6_API_ARG_SIZE_CHECK_EXPECTED(size == domain_sorts.size(), bound_vars) \
        << "'" << domain_sorts.size() << "'";                                 \
    for (size_t i = 0; i < size; ++i)                                         \
    {                                                                         \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                                   \
          domain_sorts[i] == bound_vars[i].getSort(),                         \
          "sort of parameter",                                                \
          bound_vars,                                                         \
          i)                                                                  \
          << "sort '" << domain_sorts[i] << "'";                              \
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(                                   \
          domain_sorts[i].getTypeNode().isFirstClass() && !domain_sorts[i].isFunction(),                       \
          "domain sort",                                                      \
          domain_sorts,                                                       \
          i)                                                                  \
          << "first-class sort of parameter of defined function";             \
    }                                                                         \
  } while (0)

/* Datatype checks. --------------------------------------------------------- */

/**
 * DatatypeConstructorDecl checks for member functions of class Solver.
 * Check if a given datatype constructor declaration at the index in the given
 * container of declarations is not null and associated with the term manager of
 * this solver.
 */
#define AVA6_API_SOLVER_CHECK_DTCTORDECL_AT_INDEX(decl, decls, idx)          \
  do                                                                         \
  {                                                                          \
    AVA6_API_ARG_AT_INDEX_CHECK_NOT_NULL(                                    \
        "datatype constructor declaration", decl, decls, idx);               \
    AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(d_tm.d_nm == decl.d_nm,             \
                                         "datatype constructor declaration", \
                                         decls,                              \
                                         idx)                                \
        << "a datatype constructor declaration associated with the term "    \
           "manager of this solver "                                         \
           "object";                                                         \
  } while (0)

/**
 * DatatypeConstructorDecl checks for member functions of class Solver.
 * Check if each datatype constructor declaration in the given container of
 * declarations is not null and associated with the term manager of this solver.
 */
#define AVA6_API_SOLVER_CHECK_DTCTORDECLS(decls)                   \
  for (size_t i = 0, size = decls.size(); i < size; ++i)           \
  {                                                                \
    AVA6_API_SOLVER_CHECK_DTCTORDECL_AT_INDEX(decls[i], decls, i); \
  }

/**
 * Argument number checks for mkOp.
 */
#define AVA6_API_OP_CHECK_ARITY(nargs, expected, kind)                      \
  AVA6_API_CHECK(nargs == expected)                                         \
      << "invalid number of indices for operator " << kind << ", expected " \
      << expected << " but got " << nargs << "."

}  // namespace ava6
#endif
