/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Provide compile-time configuration information about the ava6 library.
 */

#include "ava6_private.h"

#ifndef AVA6__CONFIGURATION_PRIVATE_H
#define AVA6__CONFIGURATION_PRIVATE_H

#include <string>

#include "base/configuration.h"

namespace ava6::internal {

#ifdef AVA6_DEBUG
#define IS_DEBUG_BUILD true
#else /* AVA6_DEBUG */
#define IS_DEBUG_BUILD false
#endif /* AVA6_DEBUG */



#ifdef AVA6_TRACING
#define IS_TRACING_BUILD true
#else /* AVA6_TRACING */
#define IS_TRACING_BUILD false
#endif /* AVA6_TRACING */

#ifdef AVA6_MUZZLE
#define IS_MUZZLED_BUILD true
#else /* AVA6_MUZZLE */
#define IS_MUZZLED_BUILD false
#endif /* AVA6_MUZZLE */

#ifdef AVA6_ASSERTIONS
#define IS_ASSERTIONS_BUILD true
#else /* AVA6_ASSERTIONS */
#define IS_ASSERTIONS_BUILD false
#endif /* AVA6_ASSERTIONS */

#ifdef AVA6_COVERAGE
#define IS_COVERAGE_BUILD true
#else /* AVA6_COVERAGE */
#define IS_COVERAGE_BUILD false
#endif /* AVA6_COVERAGE */

#ifdef AVA6_PROFILING
#define IS_PROFILING_BUILD true
#else /* AVA6_PROFILING */
#define IS_PROFILING_BUILD false
#endif /* AVA6_PROFILING */

#ifdef AVA6_COMPETITION_MODE
#define IS_COMPETITION_BUILD true
#else /* AVA6_COMPETITION_MODE */
#define IS_COMPETITION_BUILD false
#endif /* AVA6_COMPETITION_MODE */

#ifdef AVA6_GMP_IMP
#define IS_GMP_BUILD true
#else /* AVA6_GMP_IMP */
#define IS_GMP_BUILD false
#endif /* AVA6_GMP_IMP */

#ifdef AVA6_CLN_IMP
#define IS_CLN_BUILD true
#else /* AVA6_CLN_IMP */
#define IS_CLN_BUILD false
#endif /* AVA6_CLN_IMP */

#if AVA6_USE_GLPK
#define IS_GLPK_BUILD true
#else /* AVA6_USE_GLPK */
#define IS_GLPK_BUILD false
#endif /* AVA6_USE_GLPK */

#if AVA6_USE_CRYPTOMINISAT
#define IS_CRYPTOMINISAT_BUILD true
#else /* AVA6_USE_CRYPTOMINISAT */
#define IS_CRYPTOMINISAT_BUILD false
#endif /* AVA6_USE_CRYPTOMINISAT */

#if AVA6_USE_KISSAT
#define IS_KISSAT_BUILD true
#else /* AVA6_USE_KISSAT */
#define IS_KISSAT_BUILD false
#endif /* AVA6_USE_KISSAT */

#if AVA6_USE_POLY
#define IS_POLY_BUILD true
#else /* AVA6_USE_POLY */
#define IS_POLY_BUILD false
#endif /* AVA6_USE_POLY */

#if AVA6_USE_COCOA
#define IS_COCOA_BUILD true
#else /* AVA6_USE_COCOA */
#define IS_COCOA_BUILD false
#endif /* AVA6_USE_COCOA */

#if AVA6_USE_NORMALIZ
#define IS_NORMALIZ_BUILD true
#else /* AVA6_USE_NORMALIZ */
#define IS_NORMALIZ_BUILD false
#endif /* AVA6_USE_NORMALIZ */

#if HAVE_LIBEDITLINE
#define IS_EDITLINE_BUILD true
#else /* HAVE_LIBEDITLINE */
#define IS_EDITLINE_BUILD false
#endif /* HAVE_LIBEDITLINE */


#if AVA6_GPL_DEPS
#define IS_GPL_BUILD true
#else /* AVA6_GPL_DEPS */
#define IS_GPL_BUILD false
#endif /* AVA6_GPL_DEPS */

#define IS_ASAN_BUILD false

// GCC test
#if defined(__SANITIZE_ADDRESS__)
#undef IS_ASAN_BUILD
#define IS_ASAN_BUILD true
#endif /* defined(__SANITIZE_ADDRESS__) */

// Clang test
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#undef IS_ASAN_BUILD
#define IS_ASAN_BUILD true
#endif /* __has_feature(address_sanitizer) */
#endif /* defined(__has_feature) */

#ifdef AVA6_USE_UBSAN
#define IS_UBSAN_BUILD true
#else /* AVA6_USE_UBSAN */
#define IS_UBSAN_BUILD false
#endif /* AVA6_USE_UBSAN */

#define IS_TSAN_BUILD false

// GCC test
#if defined(__SANITIZE_THREAD__)
#undef IS_TSAN_BUILD
#define IS_TSAN_BUILD true
#endif /* defined(__SANITIZE_THREAD__) */

// Clang test
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#undef IS_TSAN_BUILD
#define IS_TSAN_BUILD true
#endif /* __has_feature(thread_sanitizer) */
#endif /* defined(__has_feature) */

}  // namespace ava6::internal

#endif /* AVA6__CONFIGURATION_PRIVATE_H */
