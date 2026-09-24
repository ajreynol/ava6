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

#ifndef AVA6_API_USE_C_ENUMS
}  // namespace ava6
#endif

#ifndef AVA6_API_USE_C_ENUMS
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
 * Serialize a ProofFormat to given stream.
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
