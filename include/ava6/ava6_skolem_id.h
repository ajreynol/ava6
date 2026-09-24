/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Skolem identifier enumeration.
 */

#if (!defined(AVA6_API_USE_C_ENUMS)                \
     && !defined(AVA6__API__AVA6_CPP_SKOLEM_ID_H)) \
    || (defined(AVA6_API_USE_C_ENUMS)              \
        && !defined(AVA6__API__AVA6_C_SKOLEM_ID_H))

#ifdef AVA6_API_USE_C_ENUMS
#include <stddef.h>
#include <stdint.h>
#undef ENUM
#define ENUM(name) Ava6##name
#else
#include <ava6/ava6_export.h>

#include <cstdint>
#include <ostream>
namespace ava6 {
#undef ENUM
#define ENUM(name) class name
#undef EVALUE
#define EVALUE(name) name
#endif

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_SKOLEM_ID_##name
#endif

// clang-format off
/**
 * The kind of a ava6 skolem. A skolem is a (family of) internal functions or
 * constants that are introduced by ava6. These symbols are treated as
 * uninterpreted internally. We track their definition for the purposes of
 * formal bookkeeping for the user of features like proofs, lemma exporting,
 * simplification and so on.
 *
 * A skolem has an identifier and a set of "skolem indices". The skolem
 * indices are *not* children of the skolem function, but rather should
 * be seen as the way of distinguishing skolems from the same family.
 *
 * For example, the family of "array diff" skolems ``ARRAY_DEQ_DIFF`` witness
 * the disequality between two arrays, which are its skolem indices.
 *
 * Say that skolem k witnesses the disequality between two arrays A and B
 * of type ``(Array Int Int)``. Then, k is a term whose skolem identifier is
 * ``ARRAY_DEQ_DIFF``, skolem indices are A and B, and whose type is ``Int``.
 *
 * Note the type of k is not ``(-> (Array Int Int) (Array Int Int) Int)``.
 * Intuitively, this is due to the fact that ava6 does not reason about array
 * diff skolem as a function symbol. Furthermore, the array diff skolem that
 * witnesses the disequality of arrays C and D is a separate skolem function k2
 * from this family, also of type ``Int``, where internally k2 has no relation
 * to k apart from having the same skolem identifier.
 *
 * In contrast, ava6 reasons about division-by-zero using a single skolem
 * function whose identifier is ``DIV_BY_ZERO``. This means its skolem indices
 * are empty and the skolem has a functional type ``(-> Real Real)``.
 *
 * \internal
 *
 */
enum ENUM(SkolemId)
{
  /**
   * The identifier of the skolem is not exported. These skolems should not
   * appear in any user-level API calls.
   */
  EVALUE(INTERNAL),
  /**
   * The purification skolem for a term. This is a variable that is semantically
   * equivalent to the indexed term t.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` The term t that this skolem purifies.
   * - Sort: The sort of t.
   * 
   * The term `(@purify t)` is equivalent to `t`.
   */
  EVALUE(PURIFY),
  /**
   * An arbitrary ground term of a given sort.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` A term that represents the sort of the term.
   * - Sort: The sort given by the index.
   * 
   * The term `(@ground_term T)` is totally unconstrained.
   */
  EVALUE(GROUND_TERM),
  /**
   * The array diff skolem, which is the witness k for the inference
   * ``(=> (not (= A B)) (not (= (select A k) (select B k))))``.
   *
   * - Number of skolem indices: ``2``
   *   - ``1:`` The first array of sort ``(Array T1 T2)``.
   *   - ``2:`` The second array of sort ``(Array T1 T2)``.
   * - Sort: ``T1``
   */
  EVALUE(ARRAY_DEQ_DIFF),
  /**
   * The empty bitvector.
   *
   * - Number of skolem indices: ``0``
   * - Type: ``(_ BitVec 0)``
   * 
   * The term `@bv_empty` is equivalent to the empty bit-vector.
   */
  EVALUE(BV_EMPTY),
  /**
   * The function for division by zero.
   *
   * - Number of skolem indices: ``0``
   * - Sort: ``(-> Real Real)``
   *
   * The term `@div_by_zero` is equivalent to `(lambda ((x Real)) (/ x 0.0))`.
   */
  EVALUE(DIV_BY_ZERO),
  /**
   * The function for integer division by zero.
   *
   * - Number of skolem indices: ``0``
   * - Sort: ``(-> Int Int)``
   *
   * The term `@int_div_by_zero` is equivalent to `(lambda ((x Int)) (div x 0))`.
   */
  EVALUE(INT_DIV_BY_ZERO),
  /**
   * The function for integer modulus by zero.
   *
   * - Number of skolem indices: ``0``
   * - Sort: ``(-> Int Int)``
   * 
   * The term `@int_mod_by_zero` is equivalent to `(lambda ((x Int)) (mod x 0))`.
   */
  EVALUE(MOD_BY_ZERO),
  /**
   * A function introduced to eliminate extended transcendental functions.
   * Transcendental functions like sqrt, arccos, arcsin, etc. are replaced
   * during processing with uninterpreted functions that are unique to
   * each function.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` A lambda corresponding to the function, e.g.,
   *   `(lambda ((x Real)) (sqrt x))`.
   * - Sort: ``(-> Real Real)``
   * 
   * The term `(@transcendental_purify f)` is equivalent to `f`.
   */
  EVALUE(TRANSCENDENTAL_PURIFY),
  /**
   * Argument used to purify transcendental function app ``(f x)``.
   * For ``(sin x)``, this is a variable that is assumed to be in phase with
   * ``x`` that is between ``-pi`` and ``pi``.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` The application of a transcendental function.
   * - Sort: ``Real``
   */
  EVALUE(TRANSCENDENTAL_PURIFY_ARG),
  /**
   * Argument used to reason about the phase shift of arguments to sine.
   * In particular, this is an integral rational indicating the number of times
   * :math:`2\pi` is added to a real value between :math:`-\pi` and :math:`\pi`
   * to obtain the value of argument to sine.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` The argument to sine.
   * - Sort: ``Real``
   */
  EVALUE(TRANSCENDENTAL_SINE_PHASE_SHIFT),
  /**
   * Used to reason about virtual term substitution. This term represents
   * an infinitesimal. This skolem is expected to appear in instantiations
   * and immediately be rewritten via virtual term substitution.
   *
   * - Number of skolem indices: ``0``
   * - Sort: ``Real``
   */
  EVALUE(ARITH_VTS_DELTA),
  /**
   * Used to reason about virtual term substitution. This term represents
   * an infinitesimal. Unlike ARITH_VTS_DELTA, this skolem may appear in
   * lemmas.
   *
   * - Number of skolem indices: ``0``
   * - Sort: ``Real``
   */
  EVALUE(ARITH_VTS_DELTA_FREE),
  /**
   * Used to reason about virtual term substitution. This term represents
   * infinity.  This skolem is expected to appear in instantiations
   * and immediately be rewritten via virtual term substitution.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` A term that represents an arithmetic sort (Int or Real).
   * - Sort: The sort given by the index.
   */
  EVALUE(ARITH_VTS_INFINITY),
  /**
   * Used to reason about virtual term substitution. This term represents
   * infinity. Unlike ARITH_VTS_INFINITY, this skolem may appear in
   * lemmas.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` A term that represents an arithmetic sort (Int or Real).
   * - Sort: The sort given by the index.
   */
  EVALUE(ARITH_VTS_INFINITY_FREE),
  /**
   * The n^th skolem for the negation of universally quantified formula Q.
   *
   * - Number of skolem indices: ``2``
   *   - ``1:`` The universally quantified formula Q.
   *   - ``2:`` The index of the variable in the binder of Q to skolemize.
   * - Sort: The type of the variable referenced by the second index.
   */
  EVALUE(QUANTIFIERS_SKOLEMIZE),
  /**
   * A witness for a string or sequence of a given length. Skolems in this family can
   * be assumed to be distinct if their identifiers (given by their third index) are
   * distinct modulo :math:`A` to the power of their length (given by their second index),
   * where :math:`A` is the cardinality of the characters of their sort.
   *
   * - Number of skolem indices: ``3``
   *   - ``1:`` A term that represents the sort of the term.
   *   - ``2:`` The assumed length of this term, expected to be a non-negative integer.
   *   - ``3:`` A numeral identifier.
   * - Sort: The sort given by the first index.
   */
  EVALUE(WITNESS_STRING_LENGTH),
  /**
   * A witness for an invertibility condition.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` A formula of the form ``(exists x. (x <op> s) <rel> t)``
   *            or ``(exists x. x <rel> t)``, where s and t are ground
   *            (bitvector) terms.
   * - Sort: The sort of x is given by the formula in the first index.
   */
  EVALUE(WITNESS_INV_CONDITION),
  /**
   * An integer corresponding to the number of times a string occurs in another
   * string. This is used to reason about str.replace_all.
   *
   * - Number of skolem indices: ``2``
   *   - ``1:`` The first string.
   *   - ``2:`` The second string.
   * - Sort: ``Int``
   */
  EVALUE(STRINGS_NUM_OCCUR),
  /**
   * A function k such that for x = 0...n, (k x) is the end
   * index of the x^th occurrence of a string b in string a, where n is the
   * number of occurrences of b in a, and ``(= (k 0) 0)``. This is used to
   * reason about str.replace_all.
   *
   * - Number of skolem indices: ``2``
   *   - ``1:`` The first string.
   *   - ``2:`` The second string.
   * - Sort: ``(-> Int Int)``
   */
  EVALUE(STRINGS_OCCUR_INDEX),
  /**
   * Analogous to STRINGS_NUM_OCCUR, but for regular expressions.
   * An integer corresponding to the number of times a regular expression can
   * be matched in a string.  This is used to reason about str.replace_all_re.
   *
   * - Number of skolem indices: ``2``
   *   - ``1:`` The string to match.
   *   - ``2:`` The regular expression to find.
   * - Sort: ``Int``
   */
  EVALUE(STRINGS_NUM_OCCUR_RE),
  /**
   * Analogous to STRINGS_OCCUR_INDEX, but for regular expressions.
   * A function k such that for x = 0...n, (k x) is the end
   * index of the x^th occurrence of a regular expression R in string a, where
   * n is the number of occurrences of R in a, and ``(= (k 0) 0)``. This is used
   * to reason about str.replace_all_re.
   *
   * - Number of skolem indices: ``2``
   *   - ``1:`` The string to match.
   *   - ``2:`` The regular expression to find.
   * - Sort: ``(-> Int Int)``
   */
  EVALUE(STRINGS_OCCUR_INDEX_RE),
  /**
   * Difference index for string disequalities, such that k is the witness for
   * the inference
   *  ``(=> (not (= a b)) (not (= (substr a k 1) (substr b k 1))))``
   * where note that `k` may be out of bounds for at most one of `a` and `b`.
   *
   * - Number of skolem indices: ``2``
   *   - ``1:`` The first string.
   *   - ``2:`` The second string.
   * - Sort: ``Int``
   */
  EVALUE(STRINGS_DEQ_DIFF),
  /**
   * A function used to define intermediate results of str.replace_all
   * applications. This denotes a function that denotes the result of processing
   * the string or sequence after processing the n^th occurrence of the string
   * in the given replace_all term.
   *
   * - Number of skolem indices: ``3``
   *   - ``1:`` The string or sequence to process.
   *   - ``2:`` The string or sequence to replace.
   *   - ``3:`` The replacement string or sequence.
   * - Sort: ``(-> Int S)`` where S is either ``String`` or ``(Seq T)`` for
   * some ``T``.
   */
  EVALUE(STRINGS_REPLACE_ALL_RESULT),
  /**
   * A function used to define intermediate results of str.replace_re_all
   * applications. This denotes a function that denotes the result of processing
   * the string after processing the n^th match of the regular expression in the
   * given replace_re_all term.
   *
   * - Number of skolem indices: ``3``
   *   - ``1:`` The string to process.
   *   - ``2:`` The regular expression to match.
   *   - ``3:`` The replacement string.
   * - Sort: ``(-> Int String)``
   */
  EVALUE(STRINGS_REPLACE_RE_ALL_RESULT),
  /**
   * A function used to define intermediate results of str.from_int
   * applications. This is a function k denoting the result
   * of processing the first n digits of the argument.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` The argument to str.from_int.
   * - Sort: ``(-> Int Int)``
   *
   * The term `(@strings_itos_result n)` is equivalent to
   * `(lambda ((x Int)) (ite (= x 0) 0 (str.to_int (str.substr (str.from_int n) 0 x))))`.
   */
  EVALUE(STRINGS_ITOS_RESULT),
  /**
   * A function used to define intermediate results of str.to_int
   * applications. This is a function k of type ``(-> Int Int)`` denoting the
   * result of processing the first n characters of the argument.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` The argument to str.to_int.
   * - Sort: ``(-> Int Int)``
   *
   * The term `(@strings_stoi_result s)` is equivalent to
   * `(lambda ((x Int)) (ite (= x 0) 0 (str.to_int (str.substr s 0 x))))`.
   */
  EVALUE(STRINGS_STOI_RESULT),
  /**
   * A position containing a non-digit in a string, used when ``(str.to_int a)``
   * is equal to -1. This is an integer that returns a position for which the
   * argument string is not a digit if one exists, or -1 otherwise.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` The argument to str.to_int.
   * - Sort: ``Int``
   *
   * The term `(@strings_stoi_non_digit s)` is equivalent to
   * `(str.indexof_re s (re.comp (re.range "0" "9")) 0)`.
   */
  EVALUE(STRINGS_STOI_NON_DIGIT),
  /**
   * Regular expression unfold component: if ``(str.in_re a R)``, where R is
   * ``(re.++ R0 ... Rn)``, then the ``RE_UNFOLD_POS_COMPONENT`` for indices
   * (a,R,i) is a string ki such that ``(= a (str.++ k0 ... kn))`` and
   * ``(str.in_re k0 R0)`` for i = 0, ..., n.
   *
   * - Number of skolem indices: ``3``
   *   - ``1:`` The string.
   *   - ``2:`` The regular expression.
   *   - ``3:`` The index of the skolem.
   * - Sort: ``String``
   */
  EVALUE(RE_UNFOLD_POS_COMPONENT),
 /**
   * A skolem for the preimage of an element y in ``(bag.map f A)`` such that
   * ``(= (f x) y)`` where f: ``(-> E T)`` is an injective function.
   *
   * - Number of skolem indices: ``3``
   *   - ``1:`` the function f of type ``(-> E T)``.
   *   - ``2:`` the bag argument A of ``(Bag E)``.
   *   - ``3:`` the element argument y type ``T``.
   * - Sort: ``E``
   */
  /**
   * An interpreted function for set.choose operator, where ``(set.choose A)``
   * is expanded to ``(uf A)`` along with the inference
   * ``(set.member (uf A) A))`` when ``A`` is non-empty,
   * where uf: ``(-> (Set E) E)`` is this skolem function, and E is the type of
   * elements of ``A``.
   *
   * - Number of skolem indices: ``1``
   *   - ``1:`` a ground value for the type ``(Set E)``.
   * - Sort: ``(-> (Set E) E)``
   */
  EVALUE(SETS_CHOOSE),
  /**
   * The set diff skolem, which is the witness k for the inference
   * ``(=> (not (= A B)) (not (= (set.member k A) (set.member k B))))``.
   *
   * - Number of skolem indices: ``2``
   *   - ``1:`` The first set of type ``(Set E)``.
   *   - ``2:`` The second set of type ``(Set E)``.
   * - Sort: ``E``
   */
  EVALUE(SETS_DEQ_DIFF),


  //================================================= Unknown rule
  /** Indicates this is not a skolem. */
  EVALUE(NONE),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};
// clang-format on

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(SkolemId) ENUM(SkolemId);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS

/**
 * Get a string representation of a Ava6SkolemId.
 * @param id The skolem id.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_skolem_id_to_string(Ava6SkolemId id);

/**
 * Hash function for Ava6SkolemId.
 * @param id The skolem id.
 * @return The hash value.
 */
AVA6_EXPORT size_t ava6_skolem_id_hash(Ava6SkolemId id);

#else

/**
 * Writes a skolem id to a stream.
 *
 * @param out The stream to write to
 * @param id The skolem id to write to the stream
 * @return The stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, SkolemId id);
}  // namespace ava6

namespace std {
/**
 * Hash function for SkolemIds.
 */
template <>
struct AVA6_EXPORT hash<ava6::SkolemId>
{
  /**
   * Hashes a SkolemId to a size_t.
   * @param id The skolem id.
   * @return The hash value.
   */
  size_t operator()(ava6::SkolemId id) const;
};
/**
 * Get the string representation of a given skolem identifier.
 * @param id The skolem identifier
 * @return The string representation.
 */
AVA6_EXPORT std::string to_string(ava6::SkolemId id);

}  // namespace std

#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
#ifndef AVA6__API__AVA6_C_SKOLEM_ID_H
#define AVA6__API__AVA6_C_SKOLEM_ID_H
#endif
#else
#ifndef AVA6__API__AVA6_CPP_SKOLEM_ID_H
#define AVA6__API__AVA6_CPP_SKOLEM_ID_H
#endif
#endif
