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

#include <ava6/ava6_export.h>

#if (!defined(AVA6_API_USE_C_ENUMS) && !defined(AVA6__API__AVA6_CPP_TYPES_H)) \
    || (defined(AVA6_API_USE_C_ENUMS) && !defined(AVA6__API__AVA6_C_TYPES_H))

#ifdef AVA6_API_USE_C_ENUMS
#define ENUM(name) Ava6##name
#else
#include <iosfwd>
namespace ava6 {
#define ENUM(name) class name
#undef EVALUE
#define EVALUE(name) name
#endif

/* -------------------------------------------------------------------------- */
/* UnknownExplanation                                                         */
/* -------------------------------------------------------------------------- */

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_UNKNOWN_EXPLANATION_##name
#endif

/**
 * The different reasons for returning an "unknown" result.
 */
enum ENUM(UnknownExplanation)
{
  /**
   * Full satisfiability check required (e.g., if only preprocessing was
   * performed).
   */
  EVALUE(REQUIRES_FULL_CHECK) = 0,
  /** Incomplete theory solver. */
  EVALUE(INCOMPLETE),
  /** Time limit reached. */
  EVALUE(TIMEOUT),
  /** Resource limit reached. */
  EVALUE(RESOURCEOUT),
  /** Memory limit reached. */
  EVALUE(MEMOUT),
  /** Solver was interrupted. */
  EVALUE(INTERRUPTED),
  /** Unsupported feature encountered. */
  EVALUE(UNSUPPORTED),
  /** Other reason. */
  EVALUE(OTHER),
  /** Requires another satisfiability check */
  EVALUE(REQUIRES_CHECK_AGAIN),
  /** No specific reason given. */
  EVALUE(UNKNOWN_REASON),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(UnknownExplanation) ENUM(UnknownExplanation);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6UnknownExplanation.
 * @param exp The unknown explanation.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_unknown_explanation_to_string(
    Ava6UnknownExplanation exp);
#else
/**
 * Serialize an UnknownExplanation to given stream.
 * @param out The output stream
 * @param e The explanation to be serialized to the given output stream
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, UnknownExplanation e);
}  // namespace ava6

namespace std {
AVA6_EXPORT std::string to_string(ava6::UnknownExplanation exp);
}

namespace ava6 {
#endif

/* -------------------------------------------------------------------------- */
/* RoundingMode                                                               */
/* -------------------------------------------------------------------------- */

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_RM_##name
#endif

/**
 * Rounding modes for floating-point numbers.
 *
 * For many floating-point operations, infinitely precise results may not be
 * representable with the number of available bits. Thus, the results are
 * rounded in a certain way to one of the representable floating-point numbers.
 *
 * \verbatim embed:rst:leading-asterisk
 * These rounding modes directly follow the SMT-LIB theory for floating-point
 * arithmetic, which in turn is based on IEEE Standard 754 :cite:`IEEE754`.
 * The rounding modes are specified in Sections 4.3.1 and 4.3.2 of the IEEE
 * Standard 754.
 * \endverbatim
 */
enum ENUM(RoundingMode)
{
  /**
   * Round to the nearest even number.
   *
   * If the two nearest floating-point numbers bracketing an unrepresentable
   * infinitely precise result are equally near, the one with an even least
   * significant digit will be delivered.
   */
  EVALUE(ROUND_NEAREST_TIES_TO_EVEN) = 0,
  /**
   * Round towards positive infinity (SMT-LIB: ``+oo``).
   *
   * The result shall be the format's floating-point number (possibly ``+oo``)
   * closest to and no less than the infinitely precise result.
   */
  EVALUE(ROUND_TOWARD_POSITIVE),
  /**
   * Round towards negative infinity (``-oo``).
   *
   * The result shall be the format's floating-point number (possibly ``-oo``)
   * closest to and no less than the infinitely precise result.
   */
  EVALUE(ROUND_TOWARD_NEGATIVE),
  /**
   * Round towards zero.
   *
   * The result shall be the format's floating-point number closest to and no
   * greater in magnitude than the infinitely precise result.
   */
  EVALUE(ROUND_TOWARD_ZERO),
  /**
   * Round to the nearest number away from zero.
   *
   * If the two nearest floating-point numbers bracketing an unrepresentable
   * infinitely precise result are equally near), the one with larger magnitude
   * will be selected.
   */
  EVALUE(ROUND_NEAREST_TIES_TO_AWAY),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(RoundingMode) ENUM(RoundingMode);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6RoundingMode.
 * @param rm The rounding mode.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_rm_to_string(Ava6RoundingMode rm);
#else
/**
 * Serialize a RoundingMode to given stream.
 * @param out The output stream
 * @param rm The rounding mode to be serialized to the given output stream
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, RoundingMode rm);
}  // namespace ava6
namespace std {
AVA6_EXPORT std::string to_string(ava6::RoundingMode rm);
}
#endif

#ifndef AVA6_API_USE_C_ENUMS
namespace ava6::modes {
#endif

/* -------------------------------------------------------------------------- */
/* BlockModelsMode                                                            */
/* -------------------------------------------------------------------------- */

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_BLOCK_MODELS_MODE_##name
#endif

/**
 * Mode for blocking models.
 *
 * Specifies how models are blocked in Solver::blockModel and
 * Solver::blockModelValues.
 */
enum ENUM(BlockModelsMode)
{
  /** Block models based on the SAT skeleton. */
  EVALUE(LITERALS) = 0,
  /** Block models based on the concrete model values for the free variables. */
  EVALUE(VALUES),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(BlockModelsMode) ENUM(BlockModelsMode);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6BlockModelsMode.
 * @param mode The mode.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_modes_block_models_mode_to_string(
    Ava6BlockModelsMode mode);
#else
/**
 * Serialize a BlockModelsMode to given stream.
 * @param out The output stream
 * @param mode The mode.
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, BlockModelsMode mode);
}

namespace std {
AVA6_EXPORT std::string to_string(ava6::modes::BlockModelsMode mode);
}

namespace ava6::modes {
#endif

/* -------------------------------------------------------------------------- */
/* LearnedLitType                                                             */
/* -------------------------------------------------------------------------- */

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_LEARNED_LIT_TYPE_##name
#endif

/**
 * Types of learned literals.
 *
 * Specifies categories of literals learned for the method
 * Solver::getLearnedLiterals.
 *
 * Note that a literal may conceptually belong to multiple categories. We
 * classify literals based on the first criteria in this list that they meet.
 */
enum ENUM(LearnedLitType)
{
  /**
   * An equality that was turned into a substitution during preprocessing.
   *
   * In particular, literals in this category are of the form (= x t) where
   * x does not occur in t.
   */
  EVALUE(PREPROCESS_SOLVED) = 0,
  /**
   * A top-level literal (unit clause) from the preprocessed set of input
   * formulas.
   */
  EVALUE(PREPROCESS),
  /**
   * A literal from the preprocessed set of input formulas that does not
   * occur at top-level after preprocessing.
   *
   * Typically), this is the most interesting category of literals to learn.
   */
  EVALUE(INPUT),
  /**
   * An internal literal that is solvable for an input variable.
   *
   * In particular, literals in this category are of the form (= x t) where
   * x does not occur in t, the preprocessed set of input formulas contains the
   * term x, but not the literal (= x t).
   *
   * Note that solvable literals can be turned into substitutions during
   * preprocessing.
   */
  EVALUE(SOLVABLE),
  /**
   * An internal literal that can be made into a constant propagation for an
   * input term.
   *
   * In particular, literals in this category are of the form (= t c) where
   * c is a constant, the preprocessed set of input formulas contains the
   * term t, but not the literal (= t c).
   */
  EVALUE(CONSTANT_PROP),
  /** Any internal literal that does not fall into the above categories. */
  EVALUE(INTERNAL),
  /** Special case for when produce-learned-literals is not set.  */
  EVALUE(UNKNOWN),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(LearnedLitType) ENUM(LearnedLitType);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6LearnedLitType.
 * @param type The learned literal type.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_modes_learned_lit_type_to_string(
    Ava6LearnedLitType type);
#else
/**
 * Serialize a LearnedLitType to given stream.
 * @param out The output stream
 * @param type The learned literal type.
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, LearnedLitType type);
}

namespace std {
AVA6_EXPORT std::string to_string(ava6::modes::LearnedLitType type);
}

namespace ava6::modes {
#endif

/* -------------------------------------------------------------------------- */
/* ProofComponent                                                             */
/* -------------------------------------------------------------------------- */

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_PROOF_COMPONENT_##name
#endif

/**
 * Components to include in a proof.
 */
enum ENUM(ProofComponent)
{
  /**
   * Proofs of G1 ... Gn whose free assumptions are a subset of
   * F1, ... Fm, where:
   * - G1, ... Gn are the preprocessed input formulas,
   * - F1, ... Fm are the input formulas.
   *
   * Note that G1 ... Gn may be arbitrary formulas, not necessarily clauses.
   */
  EVALUE(RAW_PREPROCESS) = 0,
  /**
   * Proofs of Gu1 ... Gun whose free assumptions are Fu1, ... Fum,
   * where:
   * - Gu1, ... Gun are clauses corresponding to input formulas used in the SAT
   * proof,
   * - Fu1, ... Fum is the subset of the input formulas that are used in the SAT
   * proof (i.e. the unsat core).
   *
   * Note that Gu1 ... Gun are clauses that are added to the SAT solver before
   * its main search.
   *
   * Only valid immediately after an unsat response.
   */
  EVALUE(PREPROCESS),
  /**
   * A proof of false whose free assumptions are Gu1, ... Gun, L1 ... Lk,
   * where:
   * - Gu1, ... Gun, is a set of clauses corresponding to input formulas,
   * - L1, ..., Lk is a set of clauses corresponding to theory lemmas.
   *
   * Only valid immediately after an unsat response.
   */
  EVALUE(SAT),
  /**
   * Proofs of L1 ... Lk where:
   * - L1, ..., Lk are clauses corresponding to theory lemmas used in the SAT
   * proof.
   *
   * In contrast to proofs given for preprocess, L1 ... Lk are clauses that are
   * added to the SAT solver after its main search.
   *
   * Only valid immediately after an unsat response.
   */
  EVALUE(THEORY_LEMMAS),
  /**
   * A proof of false whose free assumptions are a subset of the input formulas
   * F1), ... Fm.
   *
   * Only valid immediately after an unsat response.
   */
  EVALUE(FULL),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(ProofComponent) ENUM(ProofComponent);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6ProofComponent.
 * @param pc The proof component.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_modes_proof_component_to_string(
    Ava6ProofComponent pc);
#else
/**
 * Serialize a ProofComponent to given stream.
 * @param out The output stream
 * @param pc The proof component.
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, ProofComponent pc);
}

namespace std {
AVA6_EXPORT std::string to_string(ava6::modes::ProofComponent pc);
}

namespace ava6::modes {
#endif

/* -------------------------------------------------------------------------- */
/* ProofFormat                                                                */
/* -------------------------------------------------------------------------- */

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_PROOF_FORMAT_##name
#endif
/**
 * Proof format used for proof printing.
 */
enum ENUM(ProofFormat)
{
  EVALUE(CPC) = 0,
  EVALUE(DEFAULT) = CPC,
};

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(ProofFormat) ENUM(ProofFormat);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6ProofFormat.
 * @param format The proof format.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_modes_proof_format_to_string(
    Ava6ProofFormat format);
#else
/**
 * Serialize a FindSynthTarget to given stream.
 * @param out    The output stream
 * @param format The proof format.
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, ProofFormat format);
}

namespace std {
AVA6_EXPORT std::string to_string(ava6::modes::ProofFormat format);
}

namespace ava6::modes {
#endif

/* -------------------------------------------------------------------------- */
/* FindSynthTarget                                                            */
/* -------------------------------------------------------------------------- */

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_FIND_SYNTH_TARGET_##name
#endif

/**
 * Find synthesis targets, used as an argument to Solver::findSynth. These
 * specify various kinds of terms that can be found by this method.
 */
enum ENUM(FindSynthTarget)
{
  /**
   * Find the next term in the enumeration of the target grammar.
   */
  EVALUE(ENUM) = 0,
  /**
   * Find a pair of terms (t,s) in the target grammar which are equivalent
   * but do not rewrite to the same term in the given rewriter
   * (--sygus-rewrite=MODE). If so, the equality (= t s) is returned by
   * findSynth.
   *
   * This can be used to synthesize rewrite rules. Note if the rewriter is set
   * to none (--sygus-rewrite=none), this indicates a possible rewrite when
   * implementing a rewriter from scratch.
   */
  EVALUE(REWRITE),
  /**
   * Find a term t in the target grammar which rewrites to a term s that is
   * not equivalent to it. If so, the equality (= t s) is returned by
   * findSynth.
   *
   * This can be used to test the correctness of the given rewriter. Any
   * returned rewrite indicates an unsoundness in the given rewriter.
   */
  EVALUE(REWRITE_UNSOUND),
  /**
   * Find a rewrite between pairs of terms (t,s) that are matchable with terms
   * in the input assertions where t and s are equivalent but do not rewrite
   * to the same term in the given rewriter (--sygus-rewrite=MODE).
   *
   * This can be used to synthesize rewrite rules that apply to the current
   * problem.
   */
  EVALUE(REWRITE_INPUT),
  /**
   * Find a query over the given grammar. If the given grammar generates terms
   * that are not Boolean, we consider equalities over terms from the given
   * grammar.
   *
   * The algorithm for determining which queries to generate is configured by
   * --sygus-query-gen=MODE. Queries that are internally solved can be
   * filtered by the option --sygus-query-gen-filter-solved.
   */
  EVALUE(QUERY),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(FindSynthTarget) ENUM(FindSynthTarget);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6FindSynthTarget.
 * @param target The synthesis find target.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_modes_find_synth_target_to_string(
    Ava6FindSynthTarget target);
#else
/**
 * Serialize a FindSynthTarget to given stream.
 * @param out    The output stream
 * @param target The synthesis find target.
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, FindSynthTarget target);
}

namespace std {
AVA6_EXPORT std::string to_string(ava6::modes::FindSynthTarget target);
}

namespace ava6::modes {
#endif
/* -------------------------------------------------------------------------- */
/* OptionCategory                                                             */
/* -------------------------------------------------------------------------- */
#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_OPTION_CATEGORY_##name
#endif
/**
 * Option category enumeration.
 * Specifies the category of an option for user interface purposes.
 */
enum ENUM(OptionCategory)
{
  /** Option available to regular users */
  EVALUE(REGULAR) = 0,
  /** Option available to expert users */
  EVALUE(EXPERT),
  /** Common options */
  EVALUE(COMMON),
  /** Undocumented options */
  EVALUE(UNDOCUMENTED),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};
#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(OptionCategory) ENUM(OptionCategory);
#endif
#endif
#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6OptionCategory.
 * @param cat The option category.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_modes_option_category_to_string(
    Ava6OptionCategory cat);
#else
/**
 * Serialize an OptionCategory to given stream.
 * @param out The output stream
 * @param cat The option category to be serialized to the given output stream
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, OptionCategory cat);
}
namespace std {
AVA6_EXPORT std::string to_string(ava6::modes::OptionCategory cat);
}

namespace ava6::modes {
#endif

/* -------------------------------------------------------------------------- */
/* InputLanguage                                                              */
/* -------------------------------------------------------------------------- */

#ifdef AVA6_API_USE_C_ENUMS
#undef EVALUE
#define EVALUE(name) AVA6_INPUT_LANGUAGE_##name
#endif

/**
 * The different reasons for returning an "unknown" result.
 */
enum ENUM(InputLanguage)
{
  /** The SMT-LIB version 2.6 language */
  EVALUE(SMT_LIB_2_6) = 0,
  /** The SyGuS version 2.1 language. */
  EVALUE(SYGUS_2_1),
  /** No language given. */
  EVALUE(UNKNOWN),
#ifdef AVA6_API_USE_C_ENUMS
  // must be last entry
  EVALUE(LAST),
#endif
};

#ifdef AVA6_API_USE_C_ENUMS
#ifndef DOXYGEN_SKIP
typedef enum ENUM(InputLanguage) ENUM(InputLanguage);
#endif
#endif

#ifdef AVA6_API_USE_C_ENUMS
/**
 * Get a string representation of a Ava6InputLanguage.
 * @param lang The input language.
 * @return The string representation.
 */
AVA6_EXPORT const char* ava6_modes_input_language_to_string(
    Ava6InputLanguage lang);
#else
/**
 * Serialize an InputLanguage to given stream.
 * @param out The output stream
 * @param lang The language to be serialized to the given output stream
 * @return The output stream
 */
AVA6_EXPORT std::ostream& operator<<(std::ostream& out, InputLanguage lang);
}  // namespace ava6::modes

namespace std {
AVA6_EXPORT std::string to_string(ava6::modes::InputLanguage lang);
}
#endif

#endif

#ifdef AVA6_API_USE_C_ENUMS
#ifndef AVA6__API__AVA6_C_TYPES_H
#define AVA6__API__AVA6_C_TYPES_H
#endif
#else
#ifndef AVA6__API__AVA6_CPP_TYPES_H
#define AVA6__API__AVA6_CPP_TYPES_H
#endif
#endif
