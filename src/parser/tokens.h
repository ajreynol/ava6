/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Definitions of SMT2 tokens.
 */

#include "ava6parser_public.h"

#ifndef AVA6__PARSER__TOKENS_H
#define AVA6__PARSER__TOKENS_H

#include <sstream>
#include <string>

namespace ava6 {
namespace parser {

/**
 * Definitions of tokens used in parsers.
 */
enum Token : uint32_t
{
  EOF_TOK = 0,
  ALPHA,
  ASSERT_TOK,
  AS_TOK,
  ATTRIBUTE_TOK,
  BINARY_LITERAL,
  CHECK_SAT_ASSUMING_TOK,
  CHECK_SAT_TOK,
  DECIMAL_LITERAL,
  DECLARE_CONST_TOK,
  DECLARE_DATATYPES_TOK,
  DECLARE_DATATYPE_TOK,
  DECLARE_FUN_TOK,
  DECLARE_HEAP_TOK,
  DECLARE_ORACLE_FUN_TOK,
  DECLARE_SORT_TOK,
  DECLARE_SORT_PARAMETER_TOK,
  DEFINE_CONST_TOK,
  DEFINE_FUNS_REC_TOK,
  DEFINE_FUN_REC_TOK,
  DEFINE_FUN_TOK,
  DEFINE_SORT_TOK,
  ECHO_TOK,
  EXIT_TOK,
  GET_ASSERTIONS_TOK,
  GET_ASSIGNMENT_TOK,
  GET_INFO_TOK,
  GET_MODEL_TOK,
  GET_OPTION_TOK,
  GET_PROOF_TOK,
  GET_UNSAT_ASSUMPTIONS_TOK,
  GET_UNSAT_CORE_TOK,
  GET_UNSAT_CORE_LEMMAS_TOK,
  GET_VALUE_TOK,
  GET_MODEL_DOMAIN_ELEMENTS_TOK,
  HEX_LITERAL,
  INCLUDE_TOK,
  INDEX_TOK,
  INTEGER_LITERAL,
  KEYWORD,
  LET_TOK,
  LPAREN_TOK,
  MATCH_TOK,
  NUMERAL,
  PAR_TOK,
  POP_TOK,
  PUSH_TOK,
  QUOTED_SYMBOL,
  RATIONAL_LITERAL,
  RESET_ASSERTIONS_TOK,
  RESET_TOK,
  RPAREN_TOK,
  SET_FEATURE_TOK,
  SET_INFO_TOK,
  SET_LOGIC_TOK,
  SET_OPTION_TOK,
  SIMPLIFY_TOK,
  STRING_LITERAL,
  SYMBOL,
  UNTERMINATED_QUOTED_SYMBOL,
  UNTERMINATED_STRING_LITERAL,
  NONE
};

/** Print a token to the stream, for debugging */
std::ostream& operator<<(std::ostream& o, Token t);

}  // namespace parser
}  // namespace ava6

#endif /* AVA6__PARSER__SMT2_H */
