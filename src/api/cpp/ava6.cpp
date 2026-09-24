/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The ava6 C++ API.
 *
 * A brief note on how to guard API functions:
 *
 * In general, we think of API guards as a fence -- they are supposed to make
 * sure that no invalid arguments get passed into internal realms of ava6.
 * Thus we always want to catch such cases on the API level (and can then
 * assert internally that no invalid argument is passed in).
 *
 * The only special case is when we use 3rd party back-ends we have no control
 * over, and which throw (invalid_argument) exceptions anyways. In this case,
 * we do not replicate argument checks but delegate them to the back-end,
 * catch thrown exceptions, and raise a Ava6ApiException.
 *
 * Our Integer implementation, e.g., is such a special case since we support
 * two different back end implementations (GMP, CLN). Be aware that they do
 * not fully agree on what is (in)valid input, which requires extra checks for
 * consistent behavior (see TermManager::mkRealOrIntegerFromStrHelper for example).
 */

#include <ava6/ava6.h>

#include <cstring>
#include <sstream>

#include "api/cpp/ava6_checks.h"
#include "base/check.h"
#include "base/configuration.h"
#include "expr/array_store_all.h"
#include "expr/ascription_type.h"
#include "expr/dtype.h"
#include "expr/dtype_cons.h"
#include "expr/dtype_selector.h"
#include "expr/emptyset.h"
#include "expr/kind.h"
#include "expr/metakind.h"
#include "expr/node.h"
#include "expr/node_algorithm.h"
#include "expr/node_builder.h"
#include "expr/node_manager.h"
#include "expr/plugin.h"
#include "expr/sequence.h"
#include "expr/skolem_manager.h"
#include "expr/type_node.h"
#include "options/base_options.h"
#include "options/expr_options.h"
#include "options/main_options.h"
#include "options/options.h"
#include "options/options_public.h"
#include "options/quantifiers_options.h"
#include "options/smt_options.h"
#include "proof/proof_node.h"
#include "proof/unsat_core.h"
#include "smt/env.h"
#include "smt/model.h"
#include "smt/smt_mode.h"
#include "smt/solver_engine.h"
#include "theory/datatypes/project_op.h"
#include "theory/logic_info.h"
#include "theory/theory_model.h"
#include "util/bitvector.h"
#include "util/divisible.h"
#include "util/iand.h"
#include "util/random.h"
#include "util/regexp.h"
#include "util/result.h"
#include "util/statistics_registry.h"
#include "util/statistics_stats.h"
#include "util/statistics_value.h"
#include "util/string.h"
#include "util/uninterpreted_sort_value.h"
#include "util/utility.h"

namespace ava6 {

/* -------------------------------------------------------------------------- */
/* APIStatistics                                                              */
/* -------------------------------------------------------------------------- */

struct APIStatistics
{
  internal::HistogramStat<internal::TypeConstant> d_consts;
  internal::HistogramStat<internal::TypeConstant> d_vars;
  internal::HistogramStat<Kind> d_terms;
};

/* -------------------------------------------------------------------------- */
/* Kind                                                                       */
/* -------------------------------------------------------------------------- */

#define KIND_ENUM(external_name, internal_name) \
  {external_name,                               \
   std::make_pair(internal_name, std::string(#external_name).substr(6))}

/* Mapping from external (API) kind to internal kind. */
const static std::unordered_map<Kind, std::pair<internal::Kind, std::string>>
    s_kinds{
        KIND_ENUM(Kind::INTERNAL_KIND, internal::Kind::UNDEFINED_KIND),
        KIND_ENUM(Kind::UNDEFINED_KIND, internal::Kind::UNDEFINED_KIND),
        KIND_ENUM(Kind::NULL_TERM, internal::Kind::NULL_EXPR),
        /* Builtin ---------------------------------------------------------- */
        KIND_ENUM(Kind::UNINTERPRETED_SORT_VALUE,
                  internal::Kind::UNINTERPRETED_SORT_VALUE),
        KIND_ENUM(Kind::EQUAL, internal::Kind::EQUAL),
        KIND_ENUM(Kind::DISTINCT, internal::Kind::DISTINCT),
        KIND_ENUM(Kind::CONSTANT, internal::Kind::VARIABLE),
        KIND_ENUM(Kind::VARIABLE, internal::Kind::BOUND_VARIABLE),
        KIND_ENUM(Kind::SKOLEM, internal::Kind::SKOLEM),
        KIND_ENUM(Kind::SEXPR, internal::Kind::SEXPR),
        KIND_ENUM(Kind::LAMBDA, internal::Kind::LAMBDA),
        KIND_ENUM(Kind::WITNESS, internal::Kind::WITNESS),
        /* Boolean ---------------------------------------------------------- */
        KIND_ENUM(Kind::CONST_BOOLEAN, internal::Kind::CONST_BOOLEAN),
        KIND_ENUM(Kind::NOT, internal::Kind::NOT),
        KIND_ENUM(Kind::AND, internal::Kind::AND),
        KIND_ENUM(Kind::IMPLIES, internal::Kind::IMPLIES),
        KIND_ENUM(Kind::OR, internal::Kind::OR),
        KIND_ENUM(Kind::XOR, internal::Kind::XOR),
        KIND_ENUM(Kind::ITE, internal::Kind::ITE),
        /* UF --------------------------------------------------------------- */
        KIND_ENUM(Kind::APPLY_UF, internal::Kind::APPLY_UF),
        /* Arithmetic ------------------------------------------------------- */
        KIND_ENUM(Kind::ADD, internal::Kind::ADD),
        KIND_ENUM(Kind::MULT, internal::Kind::MULT),
        KIND_ENUM(Kind::SUB, internal::Kind::SUB),
        KIND_ENUM(Kind::NEG, internal::Kind::NEG),
        KIND_ENUM(Kind::DIVISION, internal::Kind::DIVISION),
        KIND_ENUM(Kind::DIVISION_TOTAL, internal::Kind::DIVISION_TOTAL),
        KIND_ENUM(Kind::INTS_DIVISION, internal::Kind::INTS_DIVISION),
        KIND_ENUM(Kind::INTS_DIVISION_TOTAL,
                  internal::Kind::INTS_DIVISION_TOTAL),
        KIND_ENUM(Kind::INTS_MODULUS, internal::Kind::INTS_MODULUS),
        KIND_ENUM(Kind::INTS_MODULUS_TOTAL, internal::Kind::INTS_MODULUS_TOTAL),
        KIND_ENUM(Kind::ABS, internal::Kind::ABS),
        KIND_ENUM(Kind::DIVISIBLE, internal::Kind::DIVISIBLE),
        KIND_ENUM(Kind::CONST_RATIONAL, internal::Kind::CONST_RATIONAL),
        KIND_ENUM(Kind::CONST_INTEGER, internal::Kind::CONST_INTEGER),
        KIND_ENUM(Kind::LT, internal::Kind::LT),
        KIND_ENUM(Kind::LEQ, internal::Kind::LEQ),
        KIND_ENUM(Kind::GT, internal::Kind::GT),
        KIND_ENUM(Kind::GEQ, internal::Kind::GEQ),
        KIND_ENUM(Kind::IS_INTEGER, internal::Kind::IS_INTEGER),
        KIND_ENUM(Kind::TO_INTEGER, internal::Kind::TO_INTEGER),
        KIND_ENUM(Kind::TO_REAL, internal::Kind::TO_REAL),
        /* BV --------------------------------------------------------------- */
        KIND_ENUM(Kind::CONST_BITVECTOR, internal::Kind::CONST_BITVECTOR),
        KIND_ENUM(Kind::BITVECTOR_CONCAT, internal::Kind::BITVECTOR_CONCAT),
        KIND_ENUM(Kind::BITVECTOR_AND, internal::Kind::BITVECTOR_AND),
        KIND_ENUM(Kind::BITVECTOR_OR, internal::Kind::BITVECTOR_OR),
        KIND_ENUM(Kind::BITVECTOR_XOR, internal::Kind::BITVECTOR_XOR),
        KIND_ENUM(Kind::BITVECTOR_NOT, internal::Kind::BITVECTOR_NOT),
        KIND_ENUM(Kind::BITVECTOR_NAND, internal::Kind::BITVECTOR_NAND),
        KIND_ENUM(Kind::BITVECTOR_NOR, internal::Kind::BITVECTOR_NOR),
        KIND_ENUM(Kind::BITVECTOR_XNOR, internal::Kind::BITVECTOR_XNOR),
        KIND_ENUM(Kind::BITVECTOR_COMP, internal::Kind::BITVECTOR_COMP),
        KIND_ENUM(Kind::BITVECTOR_MULT, internal::Kind::BITVECTOR_MULT),
        KIND_ENUM(Kind::BITVECTOR_ADD, internal::Kind::BITVECTOR_ADD),
        KIND_ENUM(Kind::BITVECTOR_SUB, internal::Kind::BITVECTOR_SUB),
        KIND_ENUM(Kind::BITVECTOR_NEG, internal::Kind::BITVECTOR_NEG),
        KIND_ENUM(Kind::BITVECTOR_UDIV, internal::Kind::BITVECTOR_UDIV),
        KIND_ENUM(Kind::BITVECTOR_UREM, internal::Kind::BITVECTOR_UREM),
        KIND_ENUM(Kind::BITVECTOR_SDIV, internal::Kind::BITVECTOR_SDIV),
        KIND_ENUM(Kind::BITVECTOR_SREM, internal::Kind::BITVECTOR_SREM),
        KIND_ENUM(Kind::BITVECTOR_SMOD, internal::Kind::BITVECTOR_SMOD),
        KIND_ENUM(Kind::BITVECTOR_SHL, internal::Kind::BITVECTOR_SHL),
        KIND_ENUM(Kind::BITVECTOR_LSHR, internal::Kind::BITVECTOR_LSHR),
        KIND_ENUM(Kind::BITVECTOR_ASHR, internal::Kind::BITVECTOR_ASHR),
        KIND_ENUM(Kind::BITVECTOR_ULT, internal::Kind::BITVECTOR_ULT),
        KIND_ENUM(Kind::BITVECTOR_ULE, internal::Kind::BITVECTOR_ULE),
        KIND_ENUM(Kind::BITVECTOR_UGT, internal::Kind::BITVECTOR_UGT),
        KIND_ENUM(Kind::BITVECTOR_UGE, internal::Kind::BITVECTOR_UGE),
        KIND_ENUM(Kind::BITVECTOR_SLT, internal::Kind::BITVECTOR_SLT),
        KIND_ENUM(Kind::BITVECTOR_SLE, internal::Kind::BITVECTOR_SLE),
        KIND_ENUM(Kind::BITVECTOR_SGT, internal::Kind::BITVECTOR_SGT),
        KIND_ENUM(Kind::BITVECTOR_SGE, internal::Kind::BITVECTOR_SGE),
        KIND_ENUM(Kind::BITVECTOR_ULTBV, internal::Kind::BITVECTOR_ULTBV),
        KIND_ENUM(Kind::BITVECTOR_SLTBV, internal::Kind::BITVECTOR_SLTBV),
        KIND_ENUM(Kind::BITVECTOR_NEGO, internal::Kind::BITVECTOR_NEGO),
        KIND_ENUM(Kind::BITVECTOR_UADDO, internal::Kind::BITVECTOR_UADDO),
        KIND_ENUM(Kind::BITVECTOR_SADDO, internal::Kind::BITVECTOR_SADDO),
        KIND_ENUM(Kind::BITVECTOR_UMULO, internal::Kind::BITVECTOR_UMULO),
        KIND_ENUM(Kind::BITVECTOR_SMULO, internal::Kind::BITVECTOR_SMULO),
        KIND_ENUM(Kind::BITVECTOR_USUBO, internal::Kind::BITVECTOR_USUBO),
        KIND_ENUM(Kind::BITVECTOR_SSUBO, internal::Kind::BITVECTOR_SSUBO),
        KIND_ENUM(Kind::BITVECTOR_SDIVO, internal::Kind::BITVECTOR_SDIVO),
        KIND_ENUM(Kind::BITVECTOR_ITE, internal::Kind::BITVECTOR_ITE),
        KIND_ENUM(Kind::BITVECTOR_REDOR, internal::Kind::BITVECTOR_REDOR),
        KIND_ENUM(Kind::BITVECTOR_REDAND, internal::Kind::BITVECTOR_REDAND),
        KIND_ENUM(Kind::BITVECTOR_EXTRACT, internal::Kind::BITVECTOR_EXTRACT),
        KIND_ENUM(Kind::BITVECTOR_REPEAT, internal::Kind::BITVECTOR_REPEAT),
        KIND_ENUM(Kind::BITVECTOR_ZERO_EXTEND,
                  internal::Kind::BITVECTOR_ZERO_EXTEND),
        KIND_ENUM(Kind::BITVECTOR_SIGN_EXTEND,
                  internal::Kind::BITVECTOR_SIGN_EXTEND),
        KIND_ENUM(Kind::BITVECTOR_ROTATE_LEFT,
                  internal::Kind::BITVECTOR_ROTATE_LEFT),
        KIND_ENUM(Kind::BITVECTOR_ROTATE_RIGHT,
                  internal::Kind::BITVECTOR_ROTATE_RIGHT),
        KIND_ENUM(Kind::INT_TO_BITVECTOR, internal::Kind::INT_TO_BITVECTOR),
        KIND_ENUM(Kind::BITVECTOR_UBV_TO_INT,
                  internal::Kind::BITVECTOR_UBV_TO_INT),
        KIND_ENUM(Kind::BITVECTOR_SBV_TO_INT,
                  internal::Kind::BITVECTOR_SBV_TO_INT),
        // note we silently change BITVECTOR_TO_NAT to BITVECTOR_UBV_TO_INT
        KIND_ENUM(Kind::BITVECTOR_TO_NAT, internal::Kind::BITVECTOR_UBV_TO_INT),
        KIND_ENUM(Kind::BITVECTOR_FROM_BOOLS,
                  internal::Kind::BITVECTOR_FROM_BOOLS),
        KIND_ENUM(Kind::BITVECTOR_BIT, internal::Kind::BITVECTOR_BIT),
        /* Finite Fields --------------------------------------------------- */


        /* FP --------------------------------------------------------------- */


        /* Arrays ----------------------------------------------------------- */
        KIND_ENUM(Kind::SELECT, internal::Kind::SELECT),
        KIND_ENUM(Kind::STORE, internal::Kind::STORE),
        KIND_ENUM(Kind::EQ_RANGE, internal::Kind::EQ_RANGE),
        /* Datatypes -------------------------------------------------------- */
        KIND_ENUM(Kind::APPLY_SELECTOR, internal::Kind::APPLY_SELECTOR),
        KIND_ENUM(Kind::APPLY_CONSTRUCTOR, internal::Kind::APPLY_CONSTRUCTOR),
        KIND_ENUM(Kind::APPLY_TESTER, internal::Kind::APPLY_TESTER),
        KIND_ENUM(Kind::APPLY_UPDATER, internal::Kind::APPLY_UPDATER),
        KIND_ENUM(Kind::TUPLE_PROJECT, internal::Kind::TUPLE_PROJECT),
        /* Separation Logic ------------------------------------------------- */


        /* Sets ------------------------------------------------------------- */
        KIND_ENUM(Kind::SET_EMPTY, internal::Kind::SET_EMPTY),
        KIND_ENUM(Kind::SET_UNION, internal::Kind::SET_UNION),
        KIND_ENUM(Kind::SET_INTER, internal::Kind::SET_INTER),
        KIND_ENUM(Kind::SET_MINUS, internal::Kind::SET_MINUS),
        KIND_ENUM(Kind::SET_SUBSET, internal::Kind::SET_SUBSET),
        KIND_ENUM(Kind::SET_MEMBER, internal::Kind::SET_MEMBER),
        KIND_ENUM(Kind::SET_SINGLETON, internal::Kind::SET_SINGLETON),
        KIND_ENUM(Kind::SET_INSERT, internal::Kind::SET_INSERT),
        KIND_ENUM(Kind::SET_COMPREHENSION, internal::Kind::SET_COMPREHENSION),
        KIND_ENUM(Kind::SET_CHOOSE, internal::Kind::SET_CHOOSE),
        KIND_ENUM(Kind::SET_IS_EMPTY, internal::Kind::SET_IS_EMPTY),
        KIND_ENUM(Kind::SET_IS_SINGLETON, internal::Kind::SET_IS_SINGLETON),
        /* Relations -------------------------------------------------------- */
        /* Bags ------------------------------------------------------------- */


        /* Strings ---------------------------------------------------------- */
        KIND_ENUM(Kind::STRING_CONCAT, internal::Kind::STRING_CONCAT),
        KIND_ENUM(Kind::STRING_IN_REGEXP, internal::Kind::STRING_IN_REGEXP),
        KIND_ENUM(Kind::STRING_LENGTH, internal::Kind::STRING_LENGTH),
        KIND_ENUM(Kind::STRING_SUBSTR, internal::Kind::STRING_SUBSTR),
        KIND_ENUM(Kind::STRING_UPDATE, internal::Kind::STRING_UPDATE),
        KIND_ENUM(Kind::STRING_CHARAT, internal::Kind::STRING_CHARAT),
        KIND_ENUM(Kind::STRING_CONTAINS, internal::Kind::STRING_CONTAINS),
        KIND_ENUM(Kind::STRING_INDEXOF, internal::Kind::STRING_INDEXOF),
        KIND_ENUM(Kind::STRING_INDEXOF_RE, internal::Kind::STRING_INDEXOF_RE),
        KIND_ENUM(Kind::STRING_REPLACE, internal::Kind::STRING_REPLACE),
        KIND_ENUM(Kind::STRING_REPLACE_ALL, internal::Kind::STRING_REPLACE_ALL),
        KIND_ENUM(Kind::STRING_REPLACE_RE, internal::Kind::STRING_REPLACE_RE),
        KIND_ENUM(Kind::STRING_REPLACE_RE_ALL,
                  internal::Kind::STRING_REPLACE_RE_ALL),
        KIND_ENUM(Kind::STRING_TO_LOWER, internal::Kind::STRING_TO_LOWER),
        KIND_ENUM(Kind::STRING_TO_UPPER, internal::Kind::STRING_TO_UPPER),
        KIND_ENUM(Kind::STRING_REV, internal::Kind::STRING_REV),
        KIND_ENUM(Kind::STRING_FROM_CODE, internal::Kind::STRING_FROM_CODE),
        KIND_ENUM(Kind::STRING_TO_CODE, internal::Kind::STRING_TO_CODE),
        KIND_ENUM(Kind::STRING_LT, internal::Kind::STRING_LT),
        KIND_ENUM(Kind::STRING_LEQ, internal::Kind::STRING_LEQ),
        KIND_ENUM(Kind::STRING_PREFIX, internal::Kind::STRING_PREFIX),
        KIND_ENUM(Kind::STRING_SUFFIX, internal::Kind::STRING_SUFFIX),
        KIND_ENUM(Kind::STRING_IS_DIGIT, internal::Kind::STRING_IS_DIGIT),
        KIND_ENUM(Kind::STRING_FROM_INT, internal::Kind::STRING_ITOS),
        KIND_ENUM(Kind::STRING_TO_INT, internal::Kind::STRING_STOI),
        KIND_ENUM(Kind::CONST_STRING, internal::Kind::CONST_STRING),
        KIND_ENUM(Kind::STRING_TO_REGEXP, internal::Kind::STRING_TO_REGEXP),
        KIND_ENUM(Kind::REGEXP_CONCAT, internal::Kind::REGEXP_CONCAT),
        KIND_ENUM(Kind::REGEXP_UNION, internal::Kind::REGEXP_UNION),
        KIND_ENUM(Kind::REGEXP_INTER, internal::Kind::REGEXP_INTER),
        KIND_ENUM(Kind::REGEXP_DIFF, internal::Kind::REGEXP_DIFF),
        KIND_ENUM(Kind::REGEXP_STAR, internal::Kind::REGEXP_STAR),
        KIND_ENUM(Kind::REGEXP_PLUS, internal::Kind::REGEXP_PLUS),
        KIND_ENUM(Kind::REGEXP_OPT, internal::Kind::REGEXP_OPT),
        KIND_ENUM(Kind::REGEXP_RANGE, internal::Kind::REGEXP_RANGE),
        KIND_ENUM(Kind::REGEXP_REPEAT, internal::Kind::REGEXP_REPEAT),
        KIND_ENUM(Kind::REGEXP_LOOP, internal::Kind::REGEXP_LOOP),
        KIND_ENUM(Kind::REGEXP_NONE, internal::Kind::REGEXP_NONE),
        KIND_ENUM(Kind::REGEXP_ALL, internal::Kind::REGEXP_ALL),
        KIND_ENUM(Kind::REGEXP_ALLCHAR, internal::Kind::REGEXP_ALLCHAR),
        KIND_ENUM(Kind::REGEXP_COMPLEMENT, internal::Kind::REGEXP_COMPLEMENT),
        // maps to the same kind as the string versions
        KIND_ENUM(Kind::SEQ_CONCAT, internal::Kind::STRING_CONCAT),
        KIND_ENUM(Kind::SEQ_LENGTH, internal::Kind::STRING_LENGTH),
        KIND_ENUM(Kind::SEQ_EXTRACT, internal::Kind::STRING_SUBSTR),
        KIND_ENUM(Kind::SEQ_UPDATE, internal::Kind::STRING_UPDATE),
        KIND_ENUM(Kind::SEQ_AT, internal::Kind::STRING_CHARAT),
        KIND_ENUM(Kind::SEQ_CONTAINS, internal::Kind::STRING_CONTAINS),
        KIND_ENUM(Kind::SEQ_INDEXOF, internal::Kind::STRING_INDEXOF),
        KIND_ENUM(Kind::SEQ_REPLACE, internal::Kind::STRING_REPLACE),
        KIND_ENUM(Kind::SEQ_REPLACE_ALL, internal::Kind::STRING_REPLACE_ALL),
        KIND_ENUM(Kind::SEQ_REV, internal::Kind::STRING_REV),
        KIND_ENUM(Kind::SEQ_PREFIX, internal::Kind::STRING_PREFIX),
        KIND_ENUM(Kind::SEQ_SUFFIX, internal::Kind::STRING_SUFFIX),
        KIND_ENUM(Kind::CONST_SEQUENCE, internal::Kind::CONST_SEQUENCE),
        KIND_ENUM(Kind::SEQ_UNIT, internal::Kind::SEQ_UNIT),
        KIND_ENUM(Kind::SEQ_NTH, internal::Kind::SEQ_NTH),
        /* Quantifiers ------------------------------------------------------ */
        KIND_ENUM(Kind::FORALL, internal::Kind::FORALL),
        KIND_ENUM(Kind::EXISTS, internal::Kind::EXISTS),
        KIND_ENUM(Kind::VARIABLE_LIST, internal::Kind::BOUND_VAR_LIST),
        KIND_ENUM(Kind::INST_PATTERN, internal::Kind::INST_PATTERN),
        KIND_ENUM(Kind::INST_NO_PATTERN, internal::Kind::INST_NO_PATTERN),
        KIND_ENUM(Kind::INST_ATTRIBUTE, internal::Kind::INST_ATTRIBUTE),
        KIND_ENUM(Kind::INST_PATTERN_LIST, internal::Kind::INST_PATTERN_LIST),
        KIND_ENUM(Kind::LAST_KIND, internal::Kind::LAST_KIND),
    };

/* -------------------------------------------------------------------------- */
/* SortKind                                                                   */
/* -------------------------------------------------------------------------- */

#define SORT_KIND_ENUM(external_name, internal_name) \
  {external_name,                                    \
   std::make_pair(internal_name, std::string(#external_name).substr(10))}

/* Mapping from external (API) kind to internal kind. */
const static std::unordered_map<SortKind,
                                std::pair<internal::Kind, std::string>>
    s_sort_kinds{
        SORT_KIND_ENUM(SortKind::INTERNAL_SORT_KIND,
                       internal::Kind::UNDEFINED_KIND),
        SORT_KIND_ENUM(SortKind::UNDEFINED_SORT_KIND,
                       internal::Kind::UNDEFINED_KIND),
        SORT_KIND_ENUM(SortKind::NULL_SORT, internal::Kind::NULL_EXPR),
        /* Sorts ------------------------------------------------------------ */
        // Note that many entries in this map (e.g. for type constants) are
        // given only for completeness and are not used since we don't
        // construct sorts based on SortKind.
        SORT_KIND_ENUM(SortKind::ABSTRACT_SORT, internal::Kind::ABSTRACT_TYPE),
        SORT_KIND_ENUM(SortKind::ARRAY_SORT, internal::Kind::ARRAY_TYPE),

        SORT_KIND_ENUM(SortKind::BITVECTOR_SORT,
                       internal::Kind::BITVECTOR_TYPE),
        SORT_KIND_ENUM(SortKind::BOOLEAN_SORT, internal::Kind::TYPE_CONSTANT),
        SORT_KIND_ENUM(SortKind::DATATYPE_SORT, internal::Kind::DATATYPE_TYPE),


        SORT_KIND_ENUM(SortKind::FUNCTION_SORT, internal::Kind::FUNCTION_TYPE),
        SORT_KIND_ENUM(SortKind::INTEGER_SORT, internal::Kind::TYPE_CONSTANT),
        SORT_KIND_ENUM(SortKind::REAL_SORT, internal::Kind::TYPE_CONSTANT),
        SORT_KIND_ENUM(SortKind::REGLAN_SORT, internal::Kind::TYPE_CONSTANT),
        SORT_KIND_ENUM(SortKind::SEQUENCE_SORT, internal::Kind::SEQUENCE_TYPE),
        SORT_KIND_ENUM(SortKind::SET_SORT, internal::Kind::SET_TYPE),
        SORT_KIND_ENUM(SortKind::STRING_SORT, internal::Kind::TYPE_CONSTANT),
        SORT_KIND_ENUM(SortKind::TUPLE_SORT, internal::Kind::TUPLE_TYPE),
        SORT_KIND_ENUM(SortKind::UNINTERPRETED_SORT, internal::Kind::SORT_TYPE),
        SORT_KIND_ENUM(SortKind::LAST_SORT_KIND, internal::Kind::LAST_KIND),
    };

/* Mapping from internal kind to external (API) kind. */
const static std::unordered_map<internal::Kind,
                                Kind,
                                internal::kind::KindHashFunction>
    s_kinds_internal{
        {internal::Kind::UNDEFINED_KIND, Kind::UNDEFINED_KIND},
        {internal::Kind::NULL_EXPR, Kind::NULL_TERM},
        /* Builtin --------------------------------------------------------- */
        {internal::Kind::UNINTERPRETED_SORT_VALUE,
         Kind::UNINTERPRETED_SORT_VALUE},
        {internal::Kind::EQUAL, Kind::EQUAL},
        {internal::Kind::DISTINCT, Kind::DISTINCT},
        {internal::Kind::VARIABLE, Kind::CONSTANT},
        {internal::Kind::SKOLEM, Kind::SKOLEM},
        {internal::Kind::BOUND_VARIABLE, Kind::VARIABLE},
        {internal::Kind::SEXPR, Kind::SEXPR},
        {internal::Kind::LAMBDA, Kind::LAMBDA},
        {internal::Kind::WITNESS, Kind::WITNESS},
        /* Boolean --------------------------------------------------------- */
        {internal::Kind::CONST_BOOLEAN, Kind::CONST_BOOLEAN},
        {internal::Kind::NOT, Kind::NOT},
        {internal::Kind::AND, Kind::AND},
        {internal::Kind::IMPLIES, Kind::IMPLIES},
        {internal::Kind::OR, Kind::OR},
        {internal::Kind::XOR, Kind::XOR},
        {internal::Kind::ITE, Kind::ITE},
        /* UF -------------------------------------------------------------- */
        {internal::Kind::APPLY_UF, Kind::APPLY_UF},
        /* Arithmetic ------------------------------------------------------ */
        {internal::Kind::ADD, Kind::ADD},
        {internal::Kind::MULT, Kind::MULT},
        {internal::Kind::NONLINEAR_MULT, Kind::MULT},
        {internal::Kind::IAND, Kind::IAND},
        {internal::Kind::PIAND, Kind::PIAND},
        {internal::Kind::POW2, Kind::POW2},
        {internal::Kind::INTS_LOG2, Kind::LOG2},
        {internal::Kind::SUB, Kind::SUB},
        {internal::Kind::NEG, Kind::NEG},
        {internal::Kind::DIVISION, Kind::DIVISION},
        {internal::Kind::DIVISION_TOTAL, Kind::DIVISION_TOTAL},
        {internal::Kind::INTS_DIVISION, Kind::INTS_DIVISION},
        {internal::Kind::INTS_DIVISION_TOTAL, Kind::INTS_DIVISION_TOTAL},
        {internal::Kind::INTS_MODULUS, Kind::INTS_MODULUS},
        {internal::Kind::INTS_MODULUS_TOTAL, Kind::INTS_MODULUS_TOTAL},
        {internal::Kind::ABS, Kind::ABS},
        {internal::Kind::DIVISIBLE, Kind::DIVISIBLE},
        {internal::Kind::POW, Kind::POW},
        {internal::Kind::EXPONENTIAL, Kind::EXPONENTIAL},
        {internal::Kind::SINE, Kind::SINE},
        {internal::Kind::COSINE, Kind::COSINE},
        {internal::Kind::TANGENT, Kind::TANGENT},
        {internal::Kind::COSECANT, Kind::COSECANT},
        {internal::Kind::SECANT, Kind::SECANT},
        {internal::Kind::COTANGENT, Kind::COTANGENT},
        {internal::Kind::ARCSINE, Kind::ARCSINE},
        {internal::Kind::ARCCOSINE, Kind::ARCCOSINE},
        {internal::Kind::ARCTANGENT, Kind::ARCTANGENT},
        {internal::Kind::ARCCOSECANT, Kind::ARCCOSECANT},
        {internal::Kind::ARCSECANT, Kind::ARCSECANT},
        {internal::Kind::ARCCOTANGENT, Kind::ARCCOTANGENT},
        {internal::Kind::SQRT, Kind::SQRT},
        {internal::Kind::DIVISIBLE_OP, Kind::DIVISIBLE},
        {internal::Kind::CONST_RATIONAL, Kind::CONST_RATIONAL},
        {internal::Kind::CONST_INTEGER, Kind::CONST_INTEGER},
        {internal::Kind::LT, Kind::LT},
        {internal::Kind::LEQ, Kind::LEQ},
        {internal::Kind::GT, Kind::GT},
        {internal::Kind::GEQ, Kind::GEQ},
        {internal::Kind::IS_INTEGER, Kind::IS_INTEGER},
        {internal::Kind::TO_INTEGER, Kind::TO_INTEGER},
        {internal::Kind::TO_REAL, Kind::TO_REAL},
        {internal::Kind::PI, Kind::PI},
        {internal::Kind::IAND_OP, Kind::IAND},
        /* BV -------------------------------------------------------------- */
        {internal::Kind::CONST_BITVECTOR, Kind::CONST_BITVECTOR},
        {internal::Kind::BITVECTOR_CONCAT, Kind::BITVECTOR_CONCAT},
        {internal::Kind::BITVECTOR_AND, Kind::BITVECTOR_AND},
        {internal::Kind::BITVECTOR_OR, Kind::BITVECTOR_OR},
        {internal::Kind::BITVECTOR_XOR, Kind::BITVECTOR_XOR},
        {internal::Kind::BITVECTOR_NOT, Kind::BITVECTOR_NOT},
        {internal::Kind::BITVECTOR_NAND, Kind::BITVECTOR_NAND},
        {internal::Kind::BITVECTOR_NOR, Kind::BITVECTOR_NOR},
        {internal::Kind::BITVECTOR_XNOR, Kind::BITVECTOR_XNOR},
        {internal::Kind::BITVECTOR_COMP, Kind::BITVECTOR_COMP},
        {internal::Kind::BITVECTOR_MULT, Kind::BITVECTOR_MULT},
        {internal::Kind::BITVECTOR_ADD, Kind::BITVECTOR_ADD},
        {internal::Kind::BITVECTOR_SUB, Kind::BITVECTOR_SUB},
        {internal::Kind::BITVECTOR_NEG, Kind::BITVECTOR_NEG},
        {internal::Kind::BITVECTOR_UDIV, Kind::BITVECTOR_UDIV},
        {internal::Kind::BITVECTOR_UREM, Kind::BITVECTOR_UREM},
        {internal::Kind::BITVECTOR_SDIV, Kind::BITVECTOR_SDIV},
        {internal::Kind::BITVECTOR_SREM, Kind::BITVECTOR_SREM},
        {internal::Kind::BITVECTOR_SMOD, Kind::BITVECTOR_SMOD},
        {internal::Kind::BITVECTOR_SHL, Kind::BITVECTOR_SHL},
        {internal::Kind::BITVECTOR_LSHR, Kind::BITVECTOR_LSHR},
        {internal::Kind::BITVECTOR_ASHR, Kind::BITVECTOR_ASHR},
        {internal::Kind::BITVECTOR_ULT, Kind::BITVECTOR_ULT},
        {internal::Kind::BITVECTOR_ULE, Kind::BITVECTOR_ULE},
        {internal::Kind::BITVECTOR_UGT, Kind::BITVECTOR_UGT},
        {internal::Kind::BITVECTOR_UGE, Kind::BITVECTOR_UGE},
        {internal::Kind::BITVECTOR_SLT, Kind::BITVECTOR_SLT},
        {internal::Kind::BITVECTOR_SLE, Kind::BITVECTOR_SLE},
        {internal::Kind::BITVECTOR_SGT, Kind::BITVECTOR_SGT},
        {internal::Kind::BITVECTOR_SGE, Kind::BITVECTOR_SGE},
        {internal::Kind::BITVECTOR_ULTBV, Kind::BITVECTOR_ULTBV},
        {internal::Kind::BITVECTOR_SLTBV, Kind::BITVECTOR_SLTBV},
        {internal::Kind::BITVECTOR_NEGO, Kind::BITVECTOR_NEGO},
        {internal::Kind::BITVECTOR_UADDO, Kind::BITVECTOR_UADDO},
        {internal::Kind::BITVECTOR_SADDO, Kind::BITVECTOR_SADDO},
        {internal::Kind::BITVECTOR_UMULO, Kind::BITVECTOR_UMULO},
        {internal::Kind::BITVECTOR_SMULO, Kind::BITVECTOR_SMULO},
        {internal::Kind::BITVECTOR_USUBO, Kind::BITVECTOR_USUBO},
        {internal::Kind::BITVECTOR_SSUBO, Kind::BITVECTOR_SSUBO},
        {internal::Kind::BITVECTOR_SDIVO, Kind::BITVECTOR_SDIVO},
        {internal::Kind::BITVECTOR_ITE, Kind::BITVECTOR_ITE},
        {internal::Kind::BITVECTOR_REDOR, Kind::BITVECTOR_REDOR},
        {internal::Kind::BITVECTOR_REDAND, Kind::BITVECTOR_REDAND},
        {internal::Kind::BITVECTOR_BIT_OP, Kind::BITVECTOR_BIT},
        {internal::Kind::BITVECTOR_EXTRACT_OP, Kind::BITVECTOR_EXTRACT},
        {internal::Kind::BITVECTOR_REPEAT_OP, Kind::BITVECTOR_REPEAT},
        {internal::Kind::BITVECTOR_ZERO_EXTEND_OP, Kind::BITVECTOR_ZERO_EXTEND},
        {internal::Kind::BITVECTOR_SIGN_EXTEND_OP, Kind::BITVECTOR_SIGN_EXTEND},
        {internal::Kind::BITVECTOR_ROTATE_LEFT_OP, Kind::BITVECTOR_ROTATE_LEFT},
        {internal::Kind::BITVECTOR_ROTATE_RIGHT_OP,
         Kind::BITVECTOR_ROTATE_RIGHT},
        {internal::Kind::BITVECTOR_EXTRACT, Kind::BITVECTOR_EXTRACT},
        {internal::Kind::BITVECTOR_REPEAT, Kind::BITVECTOR_REPEAT},
        {internal::Kind::BITVECTOR_ZERO_EXTEND, Kind::BITVECTOR_ZERO_EXTEND},
        {internal::Kind::BITVECTOR_SIGN_EXTEND, Kind::BITVECTOR_SIGN_EXTEND},
        {internal::Kind::BITVECTOR_ROTATE_LEFT, Kind::BITVECTOR_ROTATE_LEFT},
        {internal::Kind::BITVECTOR_ROTATE_RIGHT, Kind::BITVECTOR_ROTATE_RIGHT},
        {internal::Kind::INT_TO_BITVECTOR_OP, Kind::INT_TO_BITVECTOR},
        {internal::Kind::INT_TO_BITVECTOR, Kind::INT_TO_BITVECTOR},
        // note that BITVECTOR_TO_NAT does not exist internally, only the
        // case for BITVECTOR_UBV_TO_INT is given
        {internal::Kind::BITVECTOR_UBV_TO_INT, Kind::BITVECTOR_UBV_TO_INT},
        {internal::Kind::BITVECTOR_SBV_TO_INT, Kind::BITVECTOR_SBV_TO_INT},
        {internal::Kind::BITVECTOR_FROM_BOOLS, Kind::BITVECTOR_FROM_BOOLS},
        {internal::Kind::BITVECTOR_BIT_OP, Kind::BITVECTOR_BIT},
        {internal::Kind::BITVECTOR_BIT, Kind::BITVECTOR_BIT},
        /* Finite Fields --------------------------------------------------- */


        /* FP -------------------------------------------------------------- */


        /* Arrays ---------------------------------------------------------- */
        {internal::Kind::SELECT, Kind::SELECT},
        {internal::Kind::STORE, Kind::STORE},
        {internal::Kind::STORE_ALL, Kind::CONST_ARRAY},
        /* Datatypes ------------------------------------------------------- */
        {internal::Kind::APPLY_SELECTOR, Kind::APPLY_SELECTOR},
        {internal::Kind::APPLY_CONSTRUCTOR, Kind::APPLY_CONSTRUCTOR},
        {internal::Kind::APPLY_TESTER, Kind::APPLY_TESTER},
        {internal::Kind::APPLY_UPDATER, Kind::APPLY_UPDATER},
        {internal::Kind::MATCH, Kind::MATCH},
        {internal::Kind::MATCH_CASE, Kind::MATCH_CASE},
        {internal::Kind::MATCH_BIND_CASE, Kind::MATCH_BIND_CASE},
        {internal::Kind::TUPLE_PROJECT, Kind::TUPLE_PROJECT},
        {internal::Kind::TUPLE_PROJECT_OP, Kind::TUPLE_PROJECT},
        /* Separation Logic ------------------------------------------------ */


        /* Sets ------------------------------------------------------------ */
        {internal::Kind::SET_EMPTY, Kind::SET_EMPTY},
        {internal::Kind::SET_UNION, Kind::SET_UNION},
        {internal::Kind::SET_INTER, Kind::SET_INTER},
        {internal::Kind::SET_MINUS, Kind::SET_MINUS},
        {internal::Kind::SET_SUBSET, Kind::SET_SUBSET},
        {internal::Kind::SET_MEMBER, Kind::SET_MEMBER},
        {internal::Kind::SET_SINGLETON, Kind::SET_SINGLETON},
        {internal::Kind::SET_INSERT, Kind::SET_INSERT},
        {internal::Kind::SET_COMPREHENSION, Kind::SET_COMPREHENSION},
        {internal::Kind::SET_CHOOSE, Kind::SET_CHOOSE},
        {internal::Kind::SET_IS_EMPTY, Kind::SET_IS_EMPTY},
        {internal::Kind::SET_IS_SINGLETON, Kind::SET_IS_SINGLETON},
        /* Relations ------------------------------------------------------- */
        /* Bags ------------------------------------------------------------ */


        /* Strings --------------------------------------------------------- */
        {internal::Kind::STRING_CONCAT, Kind::STRING_CONCAT},
        {internal::Kind::STRING_IN_REGEXP, Kind::STRING_IN_REGEXP},
        {internal::Kind::STRING_LENGTH, Kind::STRING_LENGTH},
        {internal::Kind::STRING_SUBSTR, Kind::STRING_SUBSTR},
        {internal::Kind::STRING_UPDATE, Kind::STRING_UPDATE},
        {internal::Kind::STRING_CHARAT, Kind::STRING_CHARAT},
        {internal::Kind::STRING_CONTAINS, Kind::STRING_CONTAINS},
        {internal::Kind::STRING_INDEXOF, Kind::STRING_INDEXOF},
        {internal::Kind::STRING_INDEXOF_RE, Kind::STRING_INDEXOF_RE},
        {internal::Kind::STRING_REPLACE, Kind::STRING_REPLACE},
        {internal::Kind::STRING_REPLACE_ALL, Kind::STRING_REPLACE_ALL},
        {internal::Kind::STRING_REPLACE_RE, Kind::STRING_REPLACE_RE},
        {internal::Kind::STRING_REPLACE_RE_ALL, Kind::STRING_REPLACE_RE_ALL},
        {internal::Kind::STRING_TO_LOWER, Kind::STRING_TO_LOWER},
        {internal::Kind::STRING_TO_UPPER, Kind::STRING_TO_UPPER},
        {internal::Kind::STRING_REV, Kind::STRING_REV},
        {internal::Kind::STRING_FROM_CODE, Kind::STRING_FROM_CODE},
        {internal::Kind::STRING_TO_CODE, Kind::STRING_TO_CODE},
        {internal::Kind::STRING_LT, Kind::STRING_LT},
        {internal::Kind::STRING_LEQ, Kind::STRING_LEQ},
        {internal::Kind::STRING_PREFIX, Kind::STRING_PREFIX},
        {internal::Kind::STRING_SUFFIX, Kind::STRING_SUFFIX},
        {internal::Kind::STRING_IS_DIGIT, Kind::STRING_IS_DIGIT},
        {internal::Kind::STRING_ITOS, Kind::STRING_FROM_INT},
        {internal::Kind::STRING_STOI, Kind::STRING_TO_INT},
        {internal::Kind::CONST_STRING, Kind::CONST_STRING},
        {internal::Kind::STRING_TO_REGEXP, Kind::STRING_TO_REGEXP},
        {internal::Kind::REGEXP_CONCAT, Kind::REGEXP_CONCAT},
        {internal::Kind::REGEXP_UNION, Kind::REGEXP_UNION},
        {internal::Kind::REGEXP_INTER, Kind::REGEXP_INTER},
        {internal::Kind::REGEXP_DIFF, Kind::REGEXP_DIFF},
        {internal::Kind::REGEXP_STAR, Kind::REGEXP_STAR},
        {internal::Kind::REGEXP_PLUS, Kind::REGEXP_PLUS},
        {internal::Kind::REGEXP_OPT, Kind::REGEXP_OPT},
        {internal::Kind::REGEXP_RANGE, Kind::REGEXP_RANGE},
        {internal::Kind::REGEXP_REPEAT, Kind::REGEXP_REPEAT},
        {internal::Kind::REGEXP_REPEAT_OP, Kind::REGEXP_REPEAT},
        {internal::Kind::REGEXP_LOOP, Kind::REGEXP_LOOP},
        {internal::Kind::REGEXP_LOOP_OP, Kind::REGEXP_LOOP},
        {internal::Kind::REGEXP_NONE, Kind::REGEXP_NONE},
        {internal::Kind::REGEXP_ALL, Kind::REGEXP_ALL},
        {internal::Kind::REGEXP_ALLCHAR, Kind::REGEXP_ALLCHAR},
        {internal::Kind::REGEXP_COMPLEMENT, Kind::REGEXP_COMPLEMENT},
        {internal::Kind::CONST_SEQUENCE, Kind::CONST_SEQUENCE},
        {internal::Kind::SEQ_UNIT, Kind::SEQ_UNIT},
        {internal::Kind::SEQ_NTH, Kind::SEQ_NTH},
        /* Quantifiers ----------------------------------------------------- */
        {internal::Kind::FORALL, Kind::FORALL},
        {internal::Kind::EXISTS, Kind::EXISTS},
        {internal::Kind::BOUND_VAR_LIST, Kind::VARIABLE_LIST},
        {internal::Kind::INST_PATTERN, Kind::INST_PATTERN},
        {internal::Kind::INST_NO_PATTERN, Kind::INST_NO_PATTERN},
        {internal::Kind::INST_ATTRIBUTE, Kind::INST_ATTRIBUTE},
        {internal::Kind::INST_PATTERN_LIST, Kind::INST_PATTERN_LIST},
        /* ----------------------------------------------------------------- */
        {internal::Kind::LAST_KIND, Kind::LAST_KIND},
    };

/* Mapping from internal kind to external (API) sort kind. */
const static std::
    unordered_map<internal::Kind, SortKind, internal::kind::KindHashFunction>
        s_sort_kinds_internal{
            {internal::Kind::UNDEFINED_KIND, SortKind::UNDEFINED_SORT_KIND},
            {internal::Kind::NULL_EXPR, SortKind::NULL_SORT},
            {internal::Kind::ABSTRACT_TYPE, SortKind::ABSTRACT_SORT},
            {internal::Kind::ARRAY_TYPE, SortKind::ARRAY_SORT},

            {internal::Kind::BITVECTOR_TYPE, SortKind::BITVECTOR_SORT},
            {internal::Kind::DATATYPE_TYPE, SortKind::DATATYPE_SORT},


            {internal::Kind::FUNCTION_TYPE, SortKind::FUNCTION_SORT},
            {internal::Kind::SEQUENCE_TYPE, SortKind::SEQUENCE_SORT},
            {internal::Kind::SET_TYPE, SortKind::SET_SORT},
            {internal::Kind::SORT_TYPE, SortKind::UNINTERPRETED_SORT},
            {internal::Kind::TUPLE_TYPE, SortKind::TUPLE_SORT},
        };

/* Set of kinds for indexed operators */
const static std::unordered_set<Kind> s_indexed_kinds(
    {Kind::DIVISIBLE,
     Kind::IAND,
     Kind::BITVECTOR_REPEAT,
     Kind::BITVECTOR_ZERO_EXTEND,
     Kind::BITVECTOR_SIGN_EXTEND,
     Kind::BITVECTOR_ROTATE_LEFT,
     Kind::BITVECTOR_ROTATE_RIGHT,
     Kind::INT_TO_BITVECTOR,
     Kind::BITVECTOR_BIT,
     Kind::BITVECTOR_EXTRACT});

/**
 * Mapping from external (API) kind to the corresponding internal operator kind.
 */
const static std::unordered_map<Kind, internal::Kind> s_op_kinds{
    {Kind::BITVECTOR_BIT, internal::Kind::BITVECTOR_BIT_OP},
    {Kind::BITVECTOR_EXTRACT, internal::Kind::BITVECTOR_EXTRACT_OP},
    {Kind::BITVECTOR_REPEAT, internal::Kind::BITVECTOR_REPEAT_OP},
    {Kind::BITVECTOR_ROTATE_LEFT, internal::Kind::BITVECTOR_ROTATE_LEFT_OP},
    {Kind::BITVECTOR_ROTATE_RIGHT, internal::Kind::BITVECTOR_ROTATE_RIGHT_OP},
    {Kind::BITVECTOR_SIGN_EXTEND, internal::Kind::BITVECTOR_SIGN_EXTEND_OP},
    {Kind::BITVECTOR_ZERO_EXTEND, internal::Kind::BITVECTOR_ZERO_EXTEND_OP},
    {Kind::DIVISIBLE, internal::Kind::DIVISIBLE_OP},


    {Kind::IAND, internal::Kind::IAND_OP},
    {Kind::INT_TO_BITVECTOR, internal::Kind::INT_TO_BITVECTOR_OP},
    {Kind::REGEXP_REPEAT, internal::Kind::REGEXP_REPEAT_OP},
    {Kind::REGEXP_LOOP, internal::Kind::REGEXP_LOOP_OP},
    {Kind::TUPLE_PROJECT, internal::Kind::TUPLE_PROJECT_OP},


};

/* -------------------------------------------------------------------------- */
/* Rounding Mode for Floating Points                                          */
/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */
/* Helpers                                                                    */
/* -------------------------------------------------------------------------- */

namespace {

/** Convert a internal::Kind (internal) to a ava6::Kind (external).
 */
ava6::Kind intToExtKind(internal::Kind k)
{
  auto it = s_kinds_internal.find(k);
  if (it == s_kinds_internal.end())
  {
    return Kind::INTERNAL_KIND;
  }
  return it->second;
}
/** Convert a internal::Kind (internal) to a ava6::Kind (external).
 */
SortKind intToExtSortKind(internal::Kind k)
{
  auto it = s_sort_kinds_internal.find(k);
  if (it == s_sort_kinds_internal.end())
  {
    return SortKind::INTERNAL_SORT_KIND;
  }
  return it->second;
}

/** Convert a ava6::Kind (external) to a internal::Kind (internal).
 */
internal::Kind extToIntKind(ava6::Kind k)
{
  auto it = s_kinds.find(k);
  if (it == s_kinds.end())
  {
    return internal::Kind::UNDEFINED_KIND;
  }
  return it->second.first;
}

/** Convert a ava6::SortKind (external) to a internal::Kind (internal).
 */
internal::Kind extToIntSortKind(SortKind k)
{
  auto it = s_sort_kinds.find(k);
  if (it == s_sort_kinds.end())
  {
    return internal::Kind::UNDEFINED_KIND;
  }
  return it->second.first;
}

/** Return true if given kind is a defined external kind. */
bool isDefinedKind(Kind k)
{
  return k > Kind::UNDEFINED_KIND && k < Kind::LAST_KIND
         && extToIntKind(k) != internal::Kind::UNDEFINED_KIND;
}

/**
 * Return true if the internal kind is one where the API term structure
 * differs from internal structure. This happens for APPLY_* kinds.
 * The API takes a "higher-order" perspective and treats functions as well
 * as datatype constructors/selectors/testers as terms
 * but interally they are not
 */
bool isApplyKind(internal::Kind k)
{
  return (k == internal::Kind::APPLY_UF
          || k == internal::Kind::APPLY_CONSTRUCTOR
          || k == internal::Kind::APPLY_SELECTOR
          || k == internal::Kind::APPLY_TESTER
          || k == internal::Kind::APPLY_UPDATER);
}

#ifdef AVA6_ASSERTIONS
/** Return true if given kind is a defined internal kind. */
bool isDefinedIntKind(internal::Kind k)
{
  return k != internal::Kind::UNDEFINED_KIND && k != internal::Kind::LAST_KIND;
}
#endif

/** Return the minimum arity of given kind. */
uint32_t minArity(Kind k)
{
  Assert(isDefinedKind(k));
  Assert(isDefinedIntKind(extToIntKind(k)));
  uint32_t min = internal::kind::metakind::getMinArityForKind(extToIntKind(k));

  // At the API level, we treat functions/constructors/selectors/testers as
  // normal terms instead of making them part of the operator
  if (isApplyKind(extToIntKind(k)))
  {
    min++;
  }
  return min;
}

/** Return the maximum arity of given kind. */
uint32_t maxArity(Kind k)
{
  Assert(isDefinedKind(k));
  Assert(isDefinedIntKind(extToIntKind(k)));
  uint32_t max = internal::kind::metakind::getMaxArityForKind(extToIntKind(k));

  // At the API level, we treat functions/constructors/selectors/testers as
  // normal terms instead of making them part of the operator
  if (isApplyKind(extToIntKind(k))
      && max != std::numeric_limits<uint32_t>::max())  // be careful not to
                                                       // overflow
  {
    max++;
  }
  return max;
}

}  // namespace

/**
 * Class that acts as a converter from an external to an internal plugin.
 */
class PluginInternal : public internal::Plugin
{
 public:
  PluginInternal(NodeManagerSharedPtr nm, ava6::Plugin& e)
      : internal::Plugin(nm.get()), d_nm(std::move(nm)), d_external(e)
  {
  }
  /** Check method */
  std::vector<internal::Node> check() override
  {
    std::vector<Term> lemsExt = d_external.check();
    return Term::termVectorToNodes(lemsExt);
  }
  /** Notify SAT clause method */
  void notifySatClause(const internal::Node& n) override
  {
    Term t = Term(d_nm, n);
    return d_external.notifySatClause(t);
  }
  /** Notify theory lemma method */
  void notifyTheoryLemma(const internal::Node& n) override
  {
    Term t = Term(d_nm, n);
    return d_external.notifyTheoryLemma(t);
  }
  /** Get name */
  std::string getName() override { return d_external.getName(); }

 private:
  /** Reference to the node manager */
  NodeManagerSharedPtr d_nm;
  /** Reference to the external (user-provided) plugin */
  ava6::Plugin& d_external;
};

std::string kindToString(Kind k)
{
  return std::to_string(k);
}

std::ostream& operator<<(std::ostream& out, Kind k)
{
  return out << std::to_string(k);
}

std::string sortKindToString(SortKind k)
{
  return std::to_string(k);
}

std::ostream& operator<<(std::ostream& out, SortKind k)
{
  return out << std::to_string(k);
}

/* -------------------------------------------------------------------------- */
/* Result                                                                     */
/* -------------------------------------------------------------------------- */

Result::Result(const internal::Result& r) : d_result(new internal::Result(r)) {}

Result::Result() : d_result(new internal::Result()) {}

bool Result::isNull() const
{
  return d_result->getStatus() == internal::Result::NONE;
}

bool Result::isSat(void) const
{
  return d_result->getStatus() == internal::Result::SAT;
}

bool Result::isUnsat(void) const
{
  return d_result->getStatus() == internal::Result::UNSAT;
}

bool Result::isUnknown(void) const
{
  return d_result->getStatus() == internal::Result::UNKNOWN;
}

bool Result::operator==(const Result& r) const
{
  return *d_result == *r.d_result;
}

bool Result::operator!=(const Result& r) const
{
  return *d_result != *r.d_result;
}

UnknownExplanation Result::getUnknownExplanation(void) const
{
  return d_result->getUnknownExplanation();
}

std::string Result::toString(void) const { return d_result->toString(); }

std::ostream& operator<<(std::ostream& out, const Result& r)
{
  out << r.toString();
  return out;
}

}  // namespace ava6

namespace std {

size_t hash<ava6::Result>::operator()(const ava6::Result& result) const
{
  return std::hash<std::string>{}(result.toString());
}
}  // namespace std

namespace ava6 {

/* -------------------------------------------------------------------------- */
/* Sort                                                                       */
/* -------------------------------------------------------------------------- */

Sort::Sort(NodeManagerSharedPtr nm, const internal::TypeNode& t)
    : d_nm(std::move(nm)), d_type(new internal::TypeNode(t))
{
}

Sort::Sort() : d_nm(nullptr), d_type(new internal::TypeNode()) {}

Sort::~Sort()
{
  Assert(isNull() || d_nm != nullptr);
  d_type.reset();
}

std::vector<internal::TypeNode> Sort::sortVectorToTypeNodes(
    const std::vector<Sort>& sorts)
{
  std::vector<internal::TypeNode> typeNodes;
  for (const Sort& sort : sorts)
  {
    typeNodes.push_back(sort.getTypeNode());
  }
  return typeNodes;
}

std::vector<Sort> Sort::typeNodeVectorToSorts(
    NodeManagerSharedPtr nm, const std::vector<internal::TypeNode>& types)
{
  std::vector<Sort> sorts;
  for (size_t i = 0, tsize = types.size(); i < tsize; i++)
  {
    sorts.push_back(Sort(nm, types[i]));
  }
  return sorts;
}

bool Sort::operator==(const Sort& s) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_type == *s.d_type;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::operator!=(const Sort& s) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_type != *s.d_type;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::operator<(const Sort& s) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_type < *s.d_type;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::operator>(const Sort& s) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_type > *s.d_type;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::operator<=(const Sort& s) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_type <= *s.d_type;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::operator>=(const Sort& s) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_type >= *s.d_type;
  ////////
  AVA6_API_TRY_CATCH_END;
}

SortKind Sort::getKind() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  internal::Kind tk = d_type->getKind();
  // Base types are type constants, which have to be special cased to return
  // the appropriate kind.
  if (tk == internal::Kind::TYPE_CONSTANT)
  {
    switch (d_type->getConst<internal::TypeConstant>())
    {
      case internal::BOOLEAN_TYPE: return SortKind::BOOLEAN_SORT; break;
      case internal::REAL_TYPE: return SortKind::REAL_SORT; break;
      case internal::INTEGER_TYPE: return SortKind::INTEGER_SORT; break;
      case internal::STRING_TYPE: return SortKind::STRING_SORT; break;
      case internal::REGEXP_TYPE: return SortKind::REGLAN_SORT; break;
      default: return SortKind::INTERNAL_SORT_KIND; break;
    }
  }
  // otherwise we rely on the mapping
  return intToExtSortKind(tk);
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::hasSymbol() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_type->hasName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Sort::getSymbol() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->hasName())
      << "invalid call to '" << __PRETTY_FUNCTION__
      << "', expected the sort to have a symbol.";
  //////// all checks before this line
  return d_type->getName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isNull() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isNullHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isBoolean() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isBoolean();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isInteger() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isInteger();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isReal() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  // notice that we do not expose internal subtyping to the user
  return d_type->isReal() && !d_type->isInteger();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isString();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isRegExp() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isRegExp();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isBitVector() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isBitVector();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isDatatype() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isDatatype();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isDatatypeConstructor() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isDatatypeConstructor();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isDatatypeSelector() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isDatatypeSelector();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isDatatypeTester() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isDatatypeTester();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isDatatypeUpdater() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isDatatypeUpdater();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isFunction() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isFunction();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isPredicate() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isPredicate();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isTuple() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isTuple();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isRecord() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isRecord();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isArray() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isArray();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isSet() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isSet();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isSequence() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isSequence();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isAbstract() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isAbstract();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isUninterpretedSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isUninterpretedSort();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isUninterpretedSortConstructor() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->isUninterpretedSortConstructor();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::getUninterpretedSortConstructor() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isInstantiatedUninterpretedSort())
      << "expected instantiated uninterpreted sort.";
  //////// all checks before this line
  return Sort(d_nm, d_type->getUninterpretedSortConstructor());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Datatype Sort::getDatatype() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatype()) << "expected datatype sort.";
  //////// all checks before this line
  return Datatype(d_nm, d_type->getDType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Sort::isInstantiated() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_type->isInstantiated();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::instantiate(const std::vector<Sort>& params) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_DOMAIN_SORTS(params);
  AVA6_API_CHECK(d_type->isParametricDatatype()
                 || d_type->isUninterpretedSortConstructor())
      << "expected parametric datatype or sort constructor sort.";
  AVA6_API_CHECK(!d_type->isParametricDatatype()
                 || d_type->getNumChildren() == params.size() + 1)
      << "arity mismatch for instantiated parametric datatype";
  AVA6_API_CHECK(!d_type->isUninterpretedSortConstructor()
                 || d_type->getUninterpretedSortConstructorArity()
                        == params.size())
      << "arity mismatch for instantiated sort constructor";
  //////// all checks before this line
  std::vector<internal::TypeNode> tparams = sortVectorToTypeNodes(params);
  return Sort(d_nm, d_type->instantiate(tparams));
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Sort> Sort::getInstantiatedParameters() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isInstantiated())
      << "expected instantiated parametric sort";
  //////// all checks before this line
  return typeNodeVectorToSorts(d_nm, d_type->getInstantiatedParamTypes());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::substitute(const Sort& sort, const Sort& replacement) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_SORT(sort);
  AVA6_API_CHECK_SORT(replacement);
  //////// all checks before this line
  return Sort(
      d_nm, d_type->substitute(sort.getTypeNode(), replacement.getTypeNode()));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::substitute(const std::vector<Sort>& sorts,
                      const std::vector<Sort>& replacements) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_SORTS(sorts);
  AVA6_API_CHECK_SORTS(replacements);
  //////// all checks before this line

  std::vector<internal::TypeNode> tSorts = sortVectorToTypeNodes(sorts),
                                  tReplacements =
                                      sortVectorToTypeNodes(replacements);
  return Sort(d_nm,
              d_type->substitute(tSorts.begin(),
                                 tSorts.end(),
                                 tReplacements.begin(),
                                 tReplacements.end()));
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Sort::toString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_type->toString();
  ////////
  AVA6_API_TRY_CATCH_END;
}

const internal::TypeNode& Sort::getTypeNode(void) const { return *d_type; }

/* Constructor sort ------------------------------------------------------- */

size_t Sort::getDatatypeConstructorArity() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatypeConstructor())
      << "not a constructor sort: " << (*this);
  //////// all checks before this line
  return d_type->getNumChildren() - 1;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Sort> Sort::getDatatypeConstructorDomainSorts() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatypeConstructor())
      << "not a constructor sort: " << (*this);
  //////// all checks before this line
  return typeNodeVectorToSorts(d_nm, d_type->getArgTypes());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::getDatatypeConstructorCodomainSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatypeConstructor())
      << "not a constructor sort: " << (*this);
  //////// all checks before this line
  return Sort(d_nm, d_type->getDatatypeConstructorRangeType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Selector sort ------------------------------------------------------- */

Sort Sort::getDatatypeSelectorDomainSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatypeSelector())
      << "not a selector sort: " << (*this);
  //////// all checks before this line
  return Sort(d_nm, d_type->getDatatypeSelectorDomainType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::getDatatypeSelectorCodomainSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatypeSelector())
      << "not a selector sort: " << (*this);
  //////// all checks before this line
  return Sort(d_nm, d_type->getDatatypeSelectorRangeType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Tester sort ------------------------------------------------------- */

Sort Sort::getDatatypeTesterDomainSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatypeTester())
      << "not a tester sort: " << (*this);
  //////// all checks before this line
  return Sort(d_nm, d_type->getDatatypeTesterDomainType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::getDatatypeTesterCodomainSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatypeTester())
      << "not a tester sort: " << (*this);
  //////// all checks before this line
  return Sort(d_nm, d_nm->booleanType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Function sort ------------------------------------------------------- */

size_t Sort::getFunctionArity() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isFunction()) << "not a function sort: " << (*this);
  //////// all checks before this line
  return d_type->getNumChildren() - 1;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Sort> Sort::getFunctionDomainSorts() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isFunction()) << "not a function sort: " << (*this);
  //////// all checks before this line
  return typeNodeVectorToSorts(d_nm, d_type->getArgTypes());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::getFunctionCodomainSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isFunction()) << "not a function sort" << (*this);
  //////// all checks before this line
  return Sort(d_nm, d_type->getRangeType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Array sort ---------------------------------------------------------- */

Sort Sort::getArrayIndexSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isArray()) << "not an array sort.";
  //////// all checks before this line
  return Sort(d_nm, d_type->getArrayIndexType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Sort::getArrayElementSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isArray()) << "not an array sort.";
  //////// all checks before this line
  return Sort(d_nm, d_type->getArrayConstituentType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Set sort ------------------------------------------------------------ */

Sort Sort::getSetElementSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isSet()) << "not a set sort.";
  //////// all checks before this line
  return Sort(d_nm, d_type->getSetElementType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Bag sort ------------------------------------------------------------ */

/* Sequence sort ------------------------------------------------------- */

Sort Sort::getSequenceElementSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isSequence()) << "not a sequence sort.";
  //////// all checks before this line
  return Sort(d_nm, d_type->getSequenceElementType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Abstract sort ------------------------------------------------------- */

SortKind Sort::getAbstractedKind() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isAbstract()) << "not an abstract sort.";
  //////// all checks before this line
  return intToExtSortKind(d_type->getAbstractedKind());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Sort constructor sort ----------------------------------------------- */

size_t Sort::getUninterpretedSortConstructorArity() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isUninterpretedSortConstructor())
      << "not a sort constructor sort.";
  //////// all checks before this line
  return d_type->getUninterpretedSortConstructorArity();
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Bit-vector sort ----------------------------------------------------- */

uint32_t Sort::getBitVectorSize() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isBitVector()) << "not a bit-vector sort.";
  //////// all checks before this line
  return d_type->getBitVectorSize();
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Finite field sort --------------------------------------------------- */

/* Floating-point sort ------------------------------------------------- */

/* Datatype sort ------------------------------------------------------- */

size_t Sort::getDatatypeArity() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isDatatype()) << "not a datatype sort.";
  //////// all checks before this line
  return d_type->isParametricDatatype() ? d_type->getNumChildren() - 1 : 0;
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Tuple sort ---------------------------------------------------------- */

size_t Sort::getTupleLength() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isTuple()) << "not a tuple sort.";
  //////// all checks before this line
  return d_type->getTupleLength();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Sort> Sort::getTupleSorts() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_type->isTuple()) << "not a tuple sort.";
  //////// all checks before this line
  return typeNodeVectorToSorts(d_nm, d_type->getTupleTypes());
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* --------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& out, const Sort& s)
{
  out << s.toString();
  return out;
}

/* Helpers                                                                    */
/* -------------------------------------------------------------------------- */

/* Split out to avoid nested API calls (problematic with API tracing).        */
/* .......................................................................... */

bool Sort::isNullHelper() const { return d_type->isNull(); }

/* -------------------------------------------------------------------------- */
/* Op                                                                     */
/* -------------------------------------------------------------------------- */

Op::Op() : d_nm(nullptr), d_kind(Kind::NULL_TERM), d_node(new internal::Node())
{
}

Op::Op(NodeManagerSharedPtr nm, const Kind k)
    : d_nm(std::move(nm)), d_kind(k), d_node(new internal::Node())
{
}

Op::Op(NodeManagerSharedPtr nm, const Kind k, const internal::Node& n)
    : d_nm(std::move(nm)), d_kind(k), d_node(new internal::Node(n))
{
}

Op::~Op()
{
  Assert(isNull() || d_nm != nullptr);
  d_node.reset();
}

/* Public methods                                                             */
bool Op::operator==(const Op& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  if (d_node->isNull() && t.d_node->isNull())
  {
    return (d_kind == t.d_kind);
  }
  else if (d_node->isNull() || t.d_node->isNull())
  {
    return false;
  }
  return (d_kind == t.d_kind) && (*d_node == *t.d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Op::operator!=(const Op& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return !(*this == t);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Kind Op::getKind() const
{
  AVA6_API_CHECK(d_kind != Kind::NULL_TERM) << "expected a non-null Kind";
  //////// all checks before this line
  return d_kind;
}

bool Op::isNull() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isNullHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Op::isIndexed() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isIndexedHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

size_t Op::getNumIndices() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return getNumIndicesHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

size_t Op::getNumIndicesHelper() const
{
  if (!isIndexedHelper())
  {
    return 0;
  }

  Kind k = intToExtKind(d_node->getKind());
  size_t size = 0;
  switch (k)
  {
    case Kind::DIVISIBLE: size = 1; break;
    case Kind::BITVECTOR_REPEAT: size = 1; break;
    case Kind::BITVECTOR_ZERO_EXTEND: size = 1; break;
    case Kind::BITVECTOR_SIGN_EXTEND: size = 1; break;
    case Kind::BITVECTOR_ROTATE_LEFT: size = 1; break;
    case Kind::BITVECTOR_ROTATE_RIGHT: size = 1; break;
    case Kind::BITVECTOR_BIT: size = 1; break;
    case Kind::INT_TO_BITVECTOR: size = 1; break;
    case Kind::IAND: size = 1; break;
    case Kind::REGEXP_REPEAT: size = 1; break;
    case Kind::BITVECTOR_EXTRACT: size = 2; break;
    case Kind::REGEXP_LOOP: size = 2; break;
    case Kind::TUPLE_PROJECT:
      size = d_node->getConst<internal::ProjectOp>().getIndices().size();
      break;
    default: AVA6_API_CHECK(false) << "Unhandled kind " << k;
  }
  return size;
}

Term Op::operator[](size_t index) { return getIndexHelper(index); }

Term Op::getIndexHelper(size_t index)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(!d_node->isNull())
      << "expected a non-null internal expression. This Op is not indexed.";
  AVA6_API_CHECK(index < getNumIndicesHelper()) << "index out of bound";
  Kind k = intToExtKind(d_node->getKind());
  Term t;
  switch (k)
  {
    case Kind::DIVISIBLE:
    {
      t = TermManager::mkRationalValHelper(
          d_nm,
          internal::Rational(d_node->getConst<internal::Divisible>().k),
          true);
      break;
    }
    case Kind::BITVECTOR_REPEAT:
    {
      t = TermManager::mkRationalValHelper(
          d_nm,
          d_node->getConst<internal::BitVectorRepeat>().d_repeatAmount,
          true);
      break;
    }
    case Kind::BITVECTOR_ZERO_EXTEND:
    {
      t = TermManager::mkRationalValHelper(
          d_nm,
          d_node->getConst<internal::BitVectorZeroExtend>().d_zeroExtendAmount,
          true);
      break;
    }
    case Kind::BITVECTOR_SIGN_EXTEND:
    {
      t = TermManager::mkRationalValHelper(
          d_nm,
          d_node->getConst<internal::BitVectorSignExtend>().d_signExtendAmount,
          true);
      break;
    }
    case Kind::BITVECTOR_ROTATE_LEFT:
    {
      t = TermManager::mkRationalValHelper(
          d_nm,
          d_node->getConst<internal::BitVectorRotateLeft>().d_rotateLeftAmount,
          true);
      break;
    }
    case Kind::BITVECTOR_ROTATE_RIGHT:
    {
      t = TermManager::mkRationalValHelper(
          d_nm,
          d_node->getConst<internal::BitVectorRotateRight>()
              .d_rotateRightAmount,
          true);
      break;
    }
    case Kind::INT_TO_BITVECTOR:
    {
      t = TermManager::mkRationalValHelper(
          d_nm, d_node->getConst<internal::IntToBitVector>().d_size, true);
      break;
    }
    case Kind::BITVECTOR_BIT:
    {
      t = TermManager::mkRationalValHelper(
          d_nm, d_node->getConst<internal::BitVectorBit>().d_bitIndex, true);
      break;
    }
    case Kind::IAND:
    {
      t = TermManager::mkRationalValHelper(
          d_nm, d_node->getConst<internal::IntAnd>().d_size, true);
      break;
    }
    case Kind::REGEXP_REPEAT:
    {
      t = TermManager::mkRationalValHelper(
          d_nm,
          d_node->getConst<internal::RegExpRepeat>().d_repeatAmount,
          true);
      break;
    }
    case Kind::BITVECTOR_EXTRACT:
    {
      internal::BitVectorExtract ext =
          d_node->getConst<internal::BitVectorExtract>();
      t = index == 0 ? TermManager::mkRationalValHelper(d_nm, ext.d_high, true)
                     : TermManager::mkRationalValHelper(d_nm, ext.d_low, true);
      break;
    }
    case Kind::REGEXP_LOOP:
    {
      internal::RegExpLoop ext = d_node->getConst<internal::RegExpLoop>();
      t = index == 0
              ? TermManager::mkRationalValHelper(d_nm, ext.d_loopMinOcc, true)
              : TermManager::mkRationalValHelper(d_nm, ext.d_loopMaxOcc, true);

      break;
    }
    case Kind::TUPLE_PROJECT:
      t = TermManager::mkRationalValHelper(
          d_nm, d_node->getConst<internal::ProjectOp>().getIndices()[index], true);
      break;
    default:
    {
      AVA6_API_CHECK(false) << "Unhandled kind " << k;
      break;
    }
  }

  //////// all checks before this line
  return t;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Op::toString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  if (d_node->isNull())
  {
    return std::to_string(d_kind);
  }
  else
  {
    AVA6_API_CHECK(!d_node->isNull())
        << "expected a non-null internal expression";
    Assert(isNull() || d_nm != nullptr);
    return d_node->toString();
  }
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::ostream& operator<<(std::ostream& out, const Op& t)
{
  out << t.toString();
  return out;
}

/* Helpers                                                                    */
/* -------------------------------------------------------------------------- */

/* Split out to avoid nested API calls (problematic with API tracing).        */
/* .......................................................................... */

bool Op::isNullHelper() const
{
  return (d_node->isNull() && (d_kind == Kind::NULL_TERM));
}

bool Op::isIndexedHelper() const { return !d_node->isNull(); }

/* -------------------------------------------------------------------------- */
/* Term                                                                       */
/* -------------------------------------------------------------------------- */

Term::Term() : d_nm(nullptr), d_node(new internal::Node()) {}

Term::Term(NodeManagerSharedPtr nm, const internal::Node& n)
    : d_nm(std::move(nm))
{
  d_node.reset(new internal::Node(n));
}

Term::~Term()
{
  Assert(isNull() || d_nm != nullptr);
  d_node.reset();
}

bool Term::operator==(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_node == *t.d_node;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::operator!=(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_node != *t.d_node;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::operator<(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_node < *t.d_node;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::operator>(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_node > *t.d_node;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::operator<=(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_node <= *t.d_node;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::operator>=(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return *d_node >= *t.d_node;
  ////////
  AVA6_API_TRY_CATCH_END;
}

size_t Term::getNumChildren() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line

  // special case for apply kinds
  if (isApplyKind(d_node->getKind()))
  {
    return d_node->getNumChildren() + 1;
  }
  return d_node->getNumChildren();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::operator[](size_t index) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(index < getNumChildren()) << "index out of bound";
  AVA6_API_CHECK(!isApplyKind(d_node->getKind()) || d_node->hasOperator())
      << "expected apply kind to have operator when accessing child of Term";
  //////// all checks before this line

  // special cases for apply kinds
  if (isApplyKind(d_node->getKind()))
  {
    if (index == 0)
    {
      // return the operator
      return Term(d_nm, d_node->getOperator());
    }
    else
    {
      index -= 1;
    }
  }
  // otherwise we are looking up child at (index-1)
  return Term(d_nm, (*d_node)[index]);
  ////////
  AVA6_API_TRY_CATCH_END;
}

uint64_t Term::getId() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getId();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Kind Term::getKind() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return getKindHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Term::getSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return Sort(d_nm, d_node->getType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::substitute(const Term& term, const Term& replacement) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_TERM(term);
  AVA6_API_CHECK_TERM(replacement);
  AVA6_API_CHECK(AVA6_EQUAL(term.getSort(), replacement.getSort()))
      << "expected terms of the same sort in substitute";
  //////// all checks before this line
  return Term(d_nm,
              d_node->substitute(internal::TNode(*term.d_node),
                                 internal::TNode(*replacement.d_node)));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::substitute(const std::vector<Term>& terms,
                      const std::vector<Term>& replacements) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(terms.size() == replacements.size())
      << "expected vectors of the same arity in substitute";
  AVA6_API_TERM_CHECK_TERMS_WITH_TERMS_SORT_EQUAL_TO(terms, replacements);
  //////// all checks before this line
  std::vector<internal::Node> nodes = Term::termVectorToNodes(terms);
  std::vector<internal::Node> nodeReplacements =
      Term::termVectorToNodes(replacements);
  return Term(d_nm,
              d_node->substitute(nodes.begin(),
                                 nodes.end(),
                                 nodeReplacements.begin(),
                                 nodeReplacements.end()));
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::hasOp() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->hasOperator();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Op Term::getOp() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_node->hasOperator())
      << "expected Term to have an Op when calling getOp()";
  //////// all checks before this line

  // special cases for parameterized operators that are not indexed operators
  // the API level differs from the internal structure
  // indexed operators are stored in Ops
  // whereas functions and datatype operators are terms, and the Op
  // is one of the APPLY_* kinds
  if (isApplyKind(d_node->getKind()))
  {
    return Op(d_nm, intToExtKind(d_node->getKind()));
  }
  else if (d_node->getMetaKind() == internal::kind::metakind::PARAMETERIZED)
  {
    // it's an indexed operator
    // so we should return the indexed op
    internal::Node op = d_node->getOperator();
    return Op(d_nm, intToExtKind(d_node->getKind()), op);
  }
  // Notice this is the only case where getKindHelper is used, since the
  // cases above do not have special cases for intToExtKind.
  return Op(d_nm, getKindHelper());
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::hasSymbol() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->hasName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Term::getSymbol() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_node->hasName())
      << "invalid call to '" << __PRETTY_FUNCTION__
      << "', expected the term to have a symbol.";
  //////// all checks before this line
  return d_node->getName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isNull() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isNullHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::notTerm() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  internal::Node res = d_node->notNode();
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::andTerm(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_TERM(t);
  //////// all checks before this line
  internal::Node res = d_node->andNode(*t.d_node);
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::orTerm(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_TERM(t);
  //////// all checks before this line
  internal::Node res = d_node->orNode(*t.d_node);
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::xorTerm(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_TERM(t);
  //////// all checks before this line
  internal::Node res = d_node->xorNode(*t.d_node);
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::eqTerm(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_TERM(t);
  //////// all checks before this line
  internal::Node res = d_node->eqNode(*t.d_node);
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::impTerm(const Term& t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_TERM(t);
  //////// all checks before this line
  internal::Node res = d_node->impNode(*t.d_node);
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::iteTerm(const Term& then_t, const Term& else_t) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_TERM(then_t);
  AVA6_API_CHECK_TERM(else_t);
  //////// all checks before this line
  internal::Node res = d_node->iteNode(*then_t.d_node, *else_t.d_node);
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Term::toString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_node->toString();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term::const_iterator::const_iterator()
    : d_nm(nullptr), d_origNode(nullptr), d_pos(0)
{
}

Term::const_iterator::const_iterator(NodeManagerSharedPtr nm,
                                     const std::shared_ptr<internal::Node>& n,
                                     uint32_t p)
    : d_nm(std::move(nm)), d_origNode(n), d_pos(p)
{
}

bool Term::const_iterator::operator==(const const_iterator& it) const
{
  if (d_origNode == nullptr || it.d_origNode == nullptr)
  {
    return false;
  }
  return (d_nm == it.d_nm && *d_origNode == *it.d_origNode)
         && (d_pos == it.d_pos);
}

bool Term::const_iterator::operator!=(const const_iterator& it) const
{
  return !(*this == it);
}

Term::const_iterator& Term::const_iterator::operator++()
{
  Assert(d_origNode != nullptr);
  ++d_pos;
  return *this;
}

Term::const_iterator Term::const_iterator::operator++(int)
{
  Assert(d_origNode != nullptr);
  const_iterator it = *this;
  ++d_pos;
  return it;
}

Term Term::const_iterator::operator*() const
{
  Assert(d_origNode != nullptr);
  // this term has an extra child (mismatch between API and internal structure)
  // the extra child will be the first child
  bool extra_child = isApplyKind(d_origNode->getKind());

  if (!d_pos && extra_child)
  {
    return Term(d_nm, d_origNode->getOperator());
  }
  else
  {
    uint32_t idx = d_pos;
    if (extra_child)
    {
      Assert(idx > 0);
      --idx;
    }
    return Term(d_nm, (*d_origNode)[idx]);
  }
}

Term::const_iterator Term::begin() const
{
  return Term::const_iterator(d_nm, d_node, 0);
}

Term::const_iterator Term::end() const
{
  int endpos = d_node->getNumChildren();
  // special cases for APPLY_*
  // the API differs from the internal structure
  // the API takes a "higher-order" perspective and the applied
  //   function or datatype constructor/selector/tester is a Term
  // which means it needs to be one of the children, even though
  //   internally it is not
  if (isApplyKind(d_node->getKind()))
  {
    // one more child if this is a UF application (count the UF as a child)
    ++endpos;
  }
  return Term::const_iterator(d_nm, d_node, endpos);
}

const internal::Node& Term::getNode(void) const { return *d_node; }

namespace detail {
const internal::Rational& getRational(const internal::Node& node)
{
  switch (node.getKind())
  {
    case internal::Kind::CONST_INTEGER:
    case internal::Kind::CONST_RATIONAL:
      return node.getConst<internal::Rational>();
    default:
      AVA6_API_CHECK(false) << "Node is not a rational.";
      return node.getConst<internal::Rational>();
  }
}
internal::Integer getInteger(const internal::Node& node)
{
  return node.getConst<internal::Rational>().getNumerator();
}
template <typename T>
bool checkIntegerBounds(const internal::Integer& i)
{
  return i >= std::numeric_limits<T>::min()
         && i <= std::numeric_limits<T>::max();
}
bool checkReal32Bounds(const internal::Rational& r)
{
  return checkIntegerBounds<std::int32_t>(r.getNumerator())
         && checkIntegerBounds<std::uint32_t>(r.getDenominator());
}
bool checkReal64Bounds(const internal::Rational& r)
{
  return checkIntegerBounds<std::int64_t>(r.getNumerator())
         && checkIntegerBounds<std::uint64_t>(r.getDenominator());
}

bool isReal(const internal::Node& node)
{
  return node.getKind() == internal::Kind::CONST_RATIONAL
         || node.getKind() == internal::Kind::CONST_INTEGER;
}
bool isReal32(const internal::Node& node)
{
  return isReal(node) && checkReal32Bounds(getRational(node));
}
bool isReal64(const internal::Node& node)
{
  return isReal(node) && checkReal64Bounds(getRational(node));
}

bool isInteger(const internal::Node& node)
{
  return (node.getKind() == internal::Kind::CONST_RATIONAL
          || node.getKind() == internal::Kind::CONST_INTEGER)
         && node.getConst<internal::Rational>().isIntegral();
}
bool isInt32(const internal::Node& node)
{
  return isInteger(node) && checkIntegerBounds<std::int32_t>(getInteger(node));
}
bool isUInt32(const internal::Node& node)
{
  return isInteger(node) && checkIntegerBounds<std::uint32_t>(getInteger(node));
}
bool isInt64(const internal::Node& node)
{
  return isInteger(node) && checkIntegerBounds<std::int64_t>(getInteger(node));
}
bool isUInt64(const internal::Node& node)
{
  return isInteger(node) && checkIntegerBounds<std::uint64_t>(getInteger(node));
}
}  // namespace detail

int32_t Term::getRealOrIntegerValueSign() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  const internal::Rational& r = detail::getRational(*d_node);
  return static_cast<int32_t>(r.sgn());
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isInt32Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return detail::isInt32(*d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::int32_t Term::getInt32Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(detail::isInt32(*d_node), *d_node)
      << "Term to be a 32-bit integer value when calling getInt32Value()";
  //////// all checks before this line
  return detail::getInteger(*d_node).getSignedInt();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isUInt32Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return detail::isUInt32(*d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::uint32_t Term::getUInt32Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(detail::isUInt32(*d_node), *d_node)
      << "Term to be a unsigned 32-bit integer value when calling "
         "getUInt32Value()";
  //////// all checks before this line
  return detail::getInteger(*d_node).getUnsignedInt();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isInt64Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return detail::isInt64(*d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::int64_t Term::getInt64Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(detail::isInt64(*d_node), *d_node)
      << "Term to be a 64-bit integer value when calling getInt64Value()";
  //////// all checks before this line
  return detail::getInteger(*d_node).getSigned64();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isUInt64Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return detail::isUInt64(*d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::uint64_t Term::getUInt64Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(detail::isUInt64(*d_node), *d_node)
      << "Term to be a unsigned 64-bit integer value when calling "
         "getUInt64Value()";
  //////// all checks before this line
  return detail::getInteger(*d_node).getUnsigned64();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isIntegerValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return detail::isInteger(*d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::string Term::getIntegerValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(detail::isInteger(*d_node), *d_node)
      << "Term to be an integer value when calling getIntegerValue()";
  //////// all checks before this line
  return detail::getInteger(*d_node).toString();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isStringValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::CONST_STRING;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::wstring Term::getStringValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(d_node->getKind() == internal::Kind::CONST_STRING,
                              *d_node)
      << "Term to be a string value when calling getStringValue()";
  //////// all checks before this line
  return d_node->getConst<internal::String>().toWString();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::u32string Term::getU32StringValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(d_node->getKind() == internal::Kind::CONST_STRING,
                              *d_node)
      << "Term to be a string value when calling getU32StringValue()";
  //////// all checks before this line
  return d_node->getConst<internal::String>().toU32String();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<internal::Node> Term::termVectorToNodes(
    const std::vector<Term>& terms)
{
  std::vector<internal::Node> res;
  for (const Term& t : terms)
  {
    res.push_back(t.getNode());
  }
  return res;
}

std::vector<Term> Term::nodeVectorToTerms(
    NodeManagerSharedPtr nm, const std::vector<internal::Node>& nodes)
{
  std::vector<Term> res;
  for (const internal::Node& n : nodes)
  {
    res.push_back(Term(nm, n));
  }
  return res;
}

bool Term::isReal32Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return detail::isReal32(*d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::pair<std::int32_t, std::uint32_t> Term::getReal32Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(detail::isReal32(*d_node), *d_node)
      << "Term to be a 32-bit rational value when calling getReal32Value()";
  //////// all checks before this line
  const internal::Rational& r = detail::getRational(*d_node);
  return std::make_pair(r.getNumerator().getSignedInt(),
                        r.getDenominator().getUnsignedInt());
  ////////
  AVA6_API_TRY_CATCH_END;
}
bool Term::isReal64Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return detail::isReal64(*d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::pair<std::int64_t, std::uint64_t> Term::getReal64Value() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(detail::isReal64(*d_node), *d_node)
      << "Term to be a 64-bit rational value when calling getReal64Value()";
  //////// all checks before this line
  const internal::Rational& r = detail::getRational(*d_node);
  return std::make_pair(r.getNumerator().getSigned64(),
                        r.getDenominator().getUnsigned64());
  ////////
  AVA6_API_TRY_CATCH_END;
}
bool Term::isRealValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return detail::isReal(*d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::string Term::getRealValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(detail::isReal(*d_node), *d_node)
      << "Term to be a rational value when calling getRealValue()";
  //////// all checks before this line
  const internal::Rational& rat = detail::getRational(*d_node);
  std::string res = rat.toString();
  if (rat.isIntegral())
  {
    return res + "/1";
  }
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isConstArray() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::STORE_ALL;
  ////////
  AVA6_API_TRY_CATCH_END;
}
Term Term::getConstArrayBase() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(d_node->getKind() == internal::Kind::STORE_ALL,
                              *d_node)
      << "Term to be a constant array when calling getConstArrayBase()";
  //////// all checks before this line
  const auto& ar = d_node->getConst<internal::ArrayStoreAll>();
  return Term(d_nm, ar.getValue());
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isBooleanValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::CONST_BOOLEAN;
  ////////
  AVA6_API_TRY_CATCH_END;
}
bool Term::getBooleanValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(
      d_node->getKind() == internal::Kind::CONST_BOOLEAN, *d_node)
      << "Term to be a Boolean value when calling getBooleanValue()";
  //////// all checks before this line
  return d_node->getConst<bool>();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isBitVectorValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::CONST_BITVECTOR;
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::string Term::getBitVectorValue(std::uint32_t base) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(
      d_node->getKind() == internal::Kind::CONST_BITVECTOR, *d_node)
      << "Term to be a bit-vector value when calling getBitVectorValue()";
  //////// all checks before this line
  return d_node->getConst<internal::BitVector>().toString(base);
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isUninterpretedSortValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::UNINTERPRETED_SORT_VALUE;
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::string Term::getUninterpretedSortValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(
      d_node->getKind() == internal::Kind::UNINTERPRETED_SORT_VALUE, *d_node)
      << "Term to be an abstract value when calling "
         "getUninterpretedSortValue()";
  //////// all checks before this line
  return d_node->getConst<internal::UninterpretedSortValue>().getSymbol();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isTupleValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::APPLY_CONSTRUCTOR
         && d_node->isConst() && d_node->getType().getDType().isTuple();
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::vector<Term> Term::getTupleValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(
      d_node->getKind() == internal::Kind::APPLY_CONSTRUCTOR
          && d_node->isConst() && d_node->getType().getDType().isTuple(),
      *d_node)
      << "Term to be a tuple value when calling getTupleValue()";
  //////// all checks before this line
  std::vector<Term> res;
  for (size_t i = 0, n = d_node->getNumChildren(); i < n; ++i)
  {
    res.emplace_back(Term(d_nm, (*d_node)[i]));
  }
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isSetValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getType().isSet() && d_node->isConst();
  ////////
  AVA6_API_TRY_CATCH_END;
}

void Term::collectSet(std::set<Term>& set,
                      const internal::Node& node,
                      const NodeManagerSharedPtr& nm)
{
  // We asserted that node has a set type, and node.isConst()
  // Thus, node only contains of SET_EMPTY, SET_UNION and SET_SINGLETON.
  switch (node.getKind())
  {
    case internal::Kind::SET_EMPTY: break;
    case internal::Kind::SET_SINGLETON: set.emplace(Term(nm, node[0])); break;
    case internal::Kind::SET_UNION:
    {
      for (const auto& sub : node)
      {
        collectSet(set, sub, nm);
      }
      break;
    }
    default:
      AVA6_API_ARG_CHECK_EXPECTED(false, node)
          << "Term to be a set value when calling getSetValue()";
      break;
  }
}

std::set<Term> Term::getSetValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(d_node->getType().isSet() && d_node->isConst(),
                              *d_node)
      << "Term to be a set value when calling getSetValue()";
  //////// all checks before this line
  std::set<Term> res;
  Term::collectSet(res, *d_node, d_nm);
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isSequenceValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::CONST_SEQUENCE;
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::vector<Term> Term::getSequenceValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(
      d_node->getKind() == internal::Kind::CONST_SEQUENCE, *d_node)
      << "Term to be a sequence value when calling getSequenceValue()";
  //////// all checks before this line
  std::vector<Term> res;
  const internal::Sequence& seq = d_node->getConst<internal::Sequence>();
  for (const auto& node : seq.getVec())
  {
    res.emplace_back(Term(d_nm, node));
  }
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isRealAlgebraicNumber() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::REAL_ALGEBRAIC_NUMBER;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::getRealAlgebraicNumberDefiningPolynomial(const Term& v) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(
      d_node->getKind() == internal::Kind::REAL_ALGEBRAIC_NUMBER, *d_node)
      << "Term to be a real algebraic number when calling "
         "getRealAlgebraicNumberDefiningPolynomial()";
  AVA6_API_ARG_CHECK_EXPECTED(v.getKind() == Kind::VARIABLE, v)
      << "expected a variable as argument when calling "
         "getRealAlgebraicNumberDefiningPolynomial()";
  throw Ava6ApiException(
      "expected libpoly enabled build when calling "
      "getRealAlgebraicNumberDefiningPolynomial");
  //////// all checks before this line
  return Term();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::getRealAlgebraicNumberLowerBound() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(
      d_node->getKind() == internal::Kind::REAL_ALGEBRAIC_NUMBER, *d_node)
      << "Term to be a real algebraic number when calling "
         "getRealAlgebraicNumberDefiningPolynomial()";
  throw Ava6ApiException(
      "expected libpoly enabled build when calling "
      "getRealAlgebraicNumberLowerBound");
  //////// all checks before this line
  return Term();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Term::getRealAlgebraicNumberUpperBound() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(
      d_node->getKind() == internal::Kind::REAL_ALGEBRAIC_NUMBER, *d_node)
      << "Term to be a real algebraic number when calling "
         "getRealAlgebraicNumberDefiningPolynomial()";
  throw Ava6ApiException(
      "expected libpoly enabled build when calling "
      "getRealAlgebraicNumberUpperBound");
  //////// all checks before this line
  return Term();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Term::isSkolem() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_node->getKind() == internal::Kind::SKOLEM;
  ////////
  AVA6_API_TRY_CATCH_END;
}

SkolemId Term::getSkolemId() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(d_node->isSkolem(), *d_node)
      << "Term to be a skolem when calling getSkolemId";
  //////// all checks before this line
  return d_node->getSkolemId();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Term> Term::getSkolemIndices() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_EXPECTED(d_node->isSkolem(), *d_node)
      << "Term to be a skolem when calling getSkolemIndices";
  //////// all checks before this line
  std::vector<internal::Node> indices = d_node->getSkolemIndices();
  return Term::nodeVectorToTerms(d_nm, indices);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::ostream& operator<<(std::ostream& out, const Term& t)
{
  // Note that this ignores the options::ioutils properties of `out`.
  out << t.toString();
  return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<Term>& vector)
{
  internal::container_to_stream(out, vector);
  return out;
}

std::ostream& operator<<(std::ostream& out, const std::set<Term>& set)
{
  internal::container_to_stream(out, set);
  return out;
}

std::ostream& operator<<(std::ostream& out,
                         const std::unordered_set<Term>& unordered_set)
{
  internal::container_to_stream(out, unordered_set);
  return out;
}

template <typename V>
std::ostream& operator<<(std::ostream& out, const std::map<Term, V>& map)
{
  internal::container_to_stream(out, map);
  return out;
}

template <typename V>
std::ostream& operator<<(std::ostream& out,
                         const std::unordered_map<Term, V>& unordered_map)
{
  internal::container_to_stream(out, unordered_map);
  return out;
}

/* Helpers                                                                    */
/* -------------------------------------------------------------------------- */

/* Split out to avoid nested API calls (problematic with API tracing).        */
/* .......................................................................... */

bool Term::isNullHelper() const
{
  /* Split out to avoid nested API calls (problematic with API tracing). */
  return d_node->isNull();
}

Kind Term::getKindHelper() const
{
  /* Sequence kinds do not exist internally, so we must convert their internal
   * (string) versions back to sequence. All operators where this is
   * necessary are such that their first child is of sequence type, which
   * we check here. */
  if (d_node->getNumChildren() > 0 && (*d_node)[0].getType().isSequence())
  {
    switch (d_node->getKind())
    {
      case internal::Kind::STRING_CONCAT: return Kind::SEQ_CONCAT;
      case internal::Kind::STRING_LENGTH: return Kind::SEQ_LENGTH;
      case internal::Kind::STRING_SUBSTR: return Kind::SEQ_EXTRACT;
      case internal::Kind::STRING_UPDATE: return Kind::SEQ_UPDATE;
      case internal::Kind::STRING_CHARAT: return Kind::SEQ_AT;
      case internal::Kind::STRING_CONTAINS: return Kind::SEQ_CONTAINS;
      case internal::Kind::STRING_INDEXOF: return Kind::SEQ_INDEXOF;
      case internal::Kind::STRING_REPLACE: return Kind::SEQ_REPLACE;
      case internal::Kind::STRING_REPLACE_ALL: return Kind::SEQ_REPLACE_ALL;
      case internal::Kind::STRING_REV: return Kind::SEQ_REV;
      case internal::Kind::STRING_PREFIX: return Kind::SEQ_PREFIX;
      case internal::Kind::STRING_SUFFIX: return Kind::SEQ_SUFFIX;
      default:
        // fall through to conversion below
        break;
    }
  }
  // Notice that kinds like APPLY_TYPE_ASCRIPTION will be converted to
  // INTERNAL_KIND.
  return intToExtKind(d_node->getKind());
}

/* -------------------------------------------------------------------------- */
/* Datatypes                                                                  */
/* -------------------------------------------------------------------------- */

/* DatatypeConstructorDecl -------------------------------------------------- */

DatatypeConstructorDecl::DatatypeConstructorDecl()
    : d_nm(nullptr), d_ctor(nullptr)
{
}

DatatypeConstructorDecl::DatatypeConstructorDecl(NodeManagerSharedPtr nm,
                                                 const std::string& name)
    : d_nm(std::move(nm)), d_ctor(new internal::DTypeConstructor(name))
{
}
DatatypeConstructorDecl::~DatatypeConstructorDecl()
{
  if (d_ctor != nullptr)
  {
    d_ctor.reset();
  }
}

bool DatatypeConstructorDecl::operator==(
    const DatatypeConstructorDecl& decl) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_ctor == decl.d_ctor;
  ////////
  AVA6_API_TRY_CATCH_END;
}

void DatatypeConstructorDecl::addSelector(const std::string& name,
                                          const Sort& sort)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK_SORT(sort);
  AVA6_API_ARG_CHECK_EXPECTED(!sort.isNull(), sort)
      << "non-null codomain sort for selector";
  //////// all checks before this line
  d_ctor->addArg(name, *sort.d_type);
  ////////
  AVA6_API_TRY_CATCH_END;
}

void DatatypeConstructorDecl::addSelectorSelf(const std::string& name)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  d_ctor->addArgSelf(name);
  ////////
  AVA6_API_TRY_CATCH_END;
}

void DatatypeConstructorDecl::addSelectorUnresolved(
    const std::string& name, const std::string& unresDataypeName)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  // make the unresolved sort with the given name
  internal::TypeNode usort = d_nm->mkUnresolvedDatatypeSort(unresDataypeName);
  d_ctor->addArg(name, usort);
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool DatatypeConstructorDecl::isNull() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isNullHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string DatatypeConstructorDecl::toString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  std::stringstream ss;
  ss << *d_ctor;
  return ss.str();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::ostream& operator<<(std::ostream& out,
                         const DatatypeConstructorDecl& ctordecl)
{
  out << ctordecl.toString();
  return out;
}

std::ostream& operator<<(std::ostream& out,
                         const std::vector<DatatypeConstructorDecl>& vector)
{
  internal::container_to_stream(out, vector);
  return out;
}

bool DatatypeConstructorDecl::isNullHelper() const { return d_ctor == nullptr; }

bool DatatypeConstructorDecl::isResolved() const
{
  return d_ctor == nullptr || d_ctor->isResolved();
}

/* DatatypeDecl ------------------------------------------------------------- */

DatatypeDecl::DatatypeDecl() : d_nm(nullptr), d_dtype(nullptr) {}

DatatypeDecl::DatatypeDecl(NodeManagerSharedPtr nm,
                           const std::string& name)
    : d_nm(std::move(nm)), d_dtype(new internal::DType(name))
{
}

DatatypeDecl::DatatypeDecl(NodeManagerSharedPtr nm,
                           const std::string& name,
                           const std::vector<Sort>& params)
    : d_nm(std::move(nm))
{
  std::vector<internal::TypeNode> tparams = Sort::sortVectorToTypeNodes(params);
  d_dtype = std::shared_ptr<internal::DType>(
      new internal::DType(name, tparams));
}

bool DatatypeDecl::operator==(const DatatypeDecl& decl) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype == decl.d_dtype;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool DatatypeDecl::isNullHelper() const { return !d_dtype; }

DatatypeDecl::~DatatypeDecl()
{
  if (d_dtype != nullptr)
  {
    d_dtype.reset();
  }
}

bool DatatypeDecl::isResolved() const
{
  if (d_dtype == nullptr)
  {
    return true;
  }
  // We are resolved if a constructor is resolved. Note that since
  // internal::DType objects are copied in TermManager::mkDatatypeSorts, the
  // constructors of d_dtype are passed to NodeManager but not d_type itself.
  // Thus, we must check whether our constructors are resolved.
  // This is a workaround; a clearer implementation would avoid the
  // copying of DType in TermManager::mkDatatypeSorts.
  const std::vector<std::shared_ptr<internal::DTypeConstructor>>& cons =
      d_dtype->getConstructors();
  for (const std::shared_ptr<internal::DTypeConstructor>& c : cons)
  {
    if (c->isResolved())
    {
      return true;
    }
  }
  Assert(!d_dtype->isResolved());
  return false;
}

void DatatypeDecl::addConstructor(const DatatypeConstructorDecl& ctor)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_ARG_CHECK_NOT_NULL(ctor);
  AVA6_API_ARG_CHECK_TM("datatype constructor declaration", ctor);
  //////// all checks before this line
  d_dtype->addConstructor(ctor.d_ctor);
  ////////
  AVA6_API_TRY_CATCH_END;
}

size_t DatatypeDecl::getNumConstructors() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->getNumConstructors();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool DatatypeDecl::isParametric() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->isParametric();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string DatatypeDecl::toString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  std::stringstream ss;
  ss << *d_dtype;
  return ss.str();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string DatatypeDecl::getName() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->getName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool DatatypeDecl::isNull() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isNullHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::ostream& operator<<(std::ostream& out, const DatatypeDecl& dtdecl)
{
  out << dtdecl.toString();
  return out;
}

internal::DType& DatatypeDecl::getDatatype(void) const { return *d_dtype; }

/* DatatypeSelector --------------------------------------------------------- */

DatatypeSelector::DatatypeSelector() : d_nm(nullptr), d_stor(nullptr) {}

DatatypeSelector::DatatypeSelector(NodeManagerSharedPtr nm,
                                   const internal::DTypeSelector& stor)
    : d_nm(std::move(nm)), d_stor(new internal::DTypeSelector(stor))
{
  AVA6_API_CHECK(d_stor->isResolved()) << "expected resolved datatype selector";
}

DatatypeSelector::~DatatypeSelector()
{
  if (d_stor != nullptr)
  {
    d_stor.reset();
  }
}

bool DatatypeSelector::operator==(const DatatypeSelector& sel) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_stor == sel.d_stor;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string DatatypeSelector::getName() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_stor->getName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term DatatypeSelector::getTerm() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return Term(d_nm, d_stor->getSelector());
  ////////
  AVA6_API_TRY_CATCH_END;
}
Term DatatypeSelector::getUpdaterTerm() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return Term(d_nm, d_stor->getUpdater());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort DatatypeSelector::getCodomainSort() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return Sort(d_nm, d_stor->getRangeType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool DatatypeSelector::isNull() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isNullHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string DatatypeSelector::toString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  std::stringstream ss;
  ss << *d_stor;
  return ss.str();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::ostream& operator<<(std::ostream& out, const DatatypeSelector& stor)
{
  out << stor.toString();
  return out;
}

bool DatatypeSelector::isNullHelper() const { return d_stor == nullptr; }

/* DatatypeConstructor ------------------------------------------------------ */

DatatypeConstructor::DatatypeConstructor() : d_nm(nullptr), d_ctor(nullptr) {}

DatatypeConstructor::DatatypeConstructor(NodeManagerSharedPtr nm,
                                         const internal::DTypeConstructor& ctor)
    : d_nm(std::move(nm)), d_ctor(new internal::DTypeConstructor(ctor))
{
  AVA6_API_CHECK(d_ctor->isResolved())
      << "expected resolved datatype constructor";
}

DatatypeConstructor::~DatatypeConstructor()
{
  if (d_ctor != nullptr)
  {
    d_ctor.reset();
  }
}

bool DatatypeConstructor::operator==(const DatatypeConstructor& cons) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_ctor == cons.d_ctor;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string DatatypeConstructor::getName() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_ctor->getName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term DatatypeConstructor::getTerm() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return Term(d_nm, d_ctor->getConstructor());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term DatatypeConstructor::getInstantiatedTerm(const Sort& retSort) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(d_ctor->isResolved())
      << "expected resolved datatype constructor";
  AVA6_API_CHECK(retSort.isDatatype())
      << "cannot get specialized constructor type for non-datatype type "
      << retSort;
  //////// all checks before this line
  internal::Node ret = d_ctor->getInstantiatedConstructor(*retSort.d_type);
  (void)ret.getType(true); /* kick off type checking */
  // apply type ascription to the operator
  Term sctor = Term(d_nm, ret);
  return sctor;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term DatatypeConstructor::getTesterTerm() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return Term(d_nm, d_ctor->getTester());
  ////////
  AVA6_API_TRY_CATCH_END;
}

size_t DatatypeConstructor::getNumSelectors() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_ctor->getNumArgs();
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeSelector DatatypeConstructor::operator[](size_t index) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(index < d_ctor->getNumArgs()) << "index out of bounds";
  //////// all checks before this line
  return DatatypeSelector(d_nm, (*d_ctor)[index]);
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeSelector DatatypeConstructor::operator[](const std::string& name) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return getSelectorForName(name);
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeSelector DatatypeConstructor::getSelector(const std::string& name) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return getSelectorForName(name);
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeConstructor::const_iterator DatatypeConstructor::begin() const
{
  return DatatypeConstructor::const_iterator(d_nm, *d_ctor, true);
}

DatatypeConstructor::const_iterator DatatypeConstructor::end() const
{
  return DatatypeConstructor::const_iterator(d_nm, *d_ctor, false);
}

DatatypeConstructor::const_iterator::const_iterator(
    NodeManagerSharedPtr nm, const internal::DTypeConstructor& ctor, bool begin)
{
  d_nm = std::move(nm);
  d_int_stors = &ctor.getArgs();

  const std::vector<std::shared_ptr<internal::DTypeSelector>>& sels =
      ctor.getArgs();
  for (const std::shared_ptr<internal::DTypeSelector>& s : sels)
  {
    /* Can not use emplace_back here since constructor is private. */
    d_stors.push_back(DatatypeSelector(d_nm, *s.get()));
  }
  d_idx = begin ? 0 : sels.size();
}

DatatypeConstructor::const_iterator::const_iterator()
    : d_nm(nullptr), d_int_stors(nullptr), d_idx(0)
{
}

const DatatypeSelector& DatatypeConstructor::const_iterator::operator*() const
{
  return d_stors[d_idx];
}

const DatatypeSelector* DatatypeConstructor::const_iterator::operator->() const
{
  return &d_stors[d_idx];
}

DatatypeConstructor::const_iterator&
DatatypeConstructor::const_iterator::operator++()
{
  ++d_idx;
  return *this;
}

DatatypeConstructor::const_iterator
DatatypeConstructor::const_iterator::operator++(int)
{
  DatatypeConstructor::const_iterator it(*this);
  ++d_idx;
  return it;
}

bool DatatypeConstructor::const_iterator::operator==(
    const DatatypeConstructor::const_iterator& other) const
{
  return d_int_stors == other.d_int_stors && d_idx == other.d_idx;
}

bool DatatypeConstructor::const_iterator::operator!=(
    const DatatypeConstructor::const_iterator& other) const
{
  return d_int_stors != other.d_int_stors || d_idx != other.d_idx;
}

bool DatatypeConstructor::isNull() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isNullHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string DatatypeConstructor::toString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  std::stringstream ss;
  ss << *d_ctor;
  return ss.str();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool DatatypeConstructor::isNullHelper() const { return d_ctor == nullptr; }

DatatypeSelector DatatypeConstructor::getSelectorForName(
    const std::string& name) const
{
  bool foundSel = false;
  size_t index = 0;
  for (size_t i = 0, nsels = getNumSelectors(); i < nsels; i++)
  {
    if ((*d_ctor)[i].getName() == name)
    {
      index = i;
      foundSel = true;
      break;
    }
  }
  if (!foundSel)
  {
    std::stringstream snames;
    snames << "{ ";
    for (size_t i = 0, ncons = getNumSelectors(); i < ncons; i++)
    {
      snames << (*d_ctor)[i].getName() << " ";
    }
    snames << "} ";
    AVA6_API_CHECK(foundSel) << "no selector " << name << " for constructor "
                             << getName() << " exists among " << snames.str();
  }
  return DatatypeSelector(d_nm, (*d_ctor)[index]);
}

std::ostream& operator<<(std::ostream& out, const DatatypeConstructor& ctor)
{
  out << ctor.toString();
  return out;
}

/* Datatype ----------------------------------------------------------------- */

Datatype::Datatype(NodeManagerSharedPtr nm, const internal::DType& dtype)
    : d_nm(std::move(nm)), d_dtype(new internal::DType(dtype))
{
  AVA6_API_CHECK(d_dtype->isResolved()) << "expected resolved datatype";
}

Datatype::Datatype() : d_nm(nullptr), d_dtype(nullptr) {}

Datatype::~Datatype()
{
  if (d_dtype != nullptr)
  {
    d_dtype.reset();
  }
}

bool Datatype::operator==(const Datatype& dt) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype == dt.d_dtype;
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeConstructor Datatype::operator[](size_t idx) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(idx < getNumConstructors()) << "index out of bounds.";
  //////// all checks before this line
  return DatatypeConstructor(d_nm, (*d_dtype)[idx]);
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeConstructor Datatype::operator[](const std::string& name) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return getConstructorForName(name);
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeConstructor Datatype::getConstructor(const std::string& name) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return getConstructorForName(name);
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeSelector Datatype::getSelector(const std::string& name) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return getSelectorForName(name);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Datatype::getName() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->getName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

size_t Datatype::getNumConstructors() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->getNumConstructors();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Sort> Datatype::getParameters() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(isParametric()) << "expected parametric datatype";
  //////// all checks before this line
  std::vector<internal::TypeNode> params = d_dtype->getParameters();
  return Sort::typeNodeVectorToSorts(d_nm, params);
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Datatype::isParametric() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->isParametric();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Datatype::isTuple() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->isTuple();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Datatype::isRecord() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->isRecord();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Datatype::isFinite() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  AVA6_API_CHECK(!d_dtype->isParametric())
      << "invalid call to 'isFinite()', expected non-parametric datatype";
  //////// all checks before this line
  // we assume that finite model finding is disabled by passing false as the
  // second argument
  return isCardinalityClassFinite(d_dtype->getCardinalityClass());
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Datatype::isWellFounded() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->isWellFounded();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Datatype::isNull() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return isNullHelper();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Datatype::toString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK_NOT_NULL;
  //////// all checks before this line
  return d_dtype->getName();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Datatype::const_iterator Datatype::begin() const
{
  return Datatype::const_iterator(d_nm, *d_dtype, true);
}

Datatype::const_iterator Datatype::end() const
{
  return Datatype::const_iterator(d_nm, *d_dtype, false);
}

DatatypeConstructor Datatype::getConstructorForName(
    const std::string& name) const
{
  bool foundCons = false;
  size_t index = 0;
  for (size_t i = 0, ncons = getNumConstructors(); i < ncons; i++)
  {
    if ((*d_dtype)[i].getName() == name)
    {
      index = i;
      foundCons = true;
      break;
    }
  }
  if (!foundCons)
  {
    std::stringstream snames;
    snames << "{ ";
    for (size_t i = 0, ncons = getNumConstructors(); i < ncons; i++)
    {
      snames << (*d_dtype)[i].getName() << " ";
    }
    snames << "}";
    AVA6_API_CHECK(foundCons) << "no constructor " << name << " for datatype "
                              << getName() << " exists, among " << snames.str();
  }
  return DatatypeConstructor(d_nm, (*d_dtype)[index]);
}

DatatypeSelector Datatype::getSelectorForName(const std::string& name) const
{
  bool foundSel = false;
  size_t index = 0;
  size_t sindex = 0;
  for (size_t i = 0, ncons = getNumConstructors(); i < ncons; i++)
  {
    int si = (*d_dtype)[i].getSelectorIndexForName(name);
    if (si >= 0)
    {
      sindex = static_cast<size_t>(si);
      index = i;
      foundSel = true;
      break;
    }
  }
  if (!foundSel)
  {
    AVA6_API_CHECK(foundSel)
        << "no selector " << name << " for datatype " << getName() << " exists";
  }
  return DatatypeSelector(d_nm, (*d_dtype)[index][sindex]);
}

Datatype::const_iterator::const_iterator(NodeManagerSharedPtr nm,
                                         const internal::DType& dtype,
                                         bool begin)
    : d_nm(std::move(nm)), d_int_ctors(&dtype.getConstructors())
{
  const std::vector<std::shared_ptr<internal::DTypeConstructor>>& cons =
      dtype.getConstructors();
  for (const std::shared_ptr<internal::DTypeConstructor>& c : cons)
  {
    /* Can not use emplace_back here since constructor is private. */
    d_ctors.push_back(DatatypeConstructor(d_nm, *c.get()));
  }
  d_idx = begin ? 0 : cons.size();
}

Datatype::const_iterator::const_iterator()
    : d_nm(nullptr), d_int_ctors(nullptr), d_idx(0)
{
}

const DatatypeConstructor& Datatype::const_iterator::operator*() const
{
  return d_ctors[d_idx];
}

const DatatypeConstructor* Datatype::const_iterator::operator->() const
{
  return &d_ctors[d_idx];
}

Datatype::const_iterator& Datatype::const_iterator::operator++()
{
  ++d_idx;
  return *this;
}

Datatype::const_iterator Datatype::const_iterator::operator++(int)
{
  Datatype::const_iterator it(*this);
  ++d_idx;
  return it;
}

bool Datatype::const_iterator::operator==(
    const Datatype::const_iterator& other) const
{
  return d_int_ctors == other.d_int_ctors && d_idx == other.d_idx;
}

bool Datatype::const_iterator::operator!=(
    const Datatype::const_iterator& other) const
{
  return d_int_ctors != other.d_int_ctors || d_idx != other.d_idx;
}

bool Datatype::isNullHelper() const { return d_dtype == nullptr; }

std::ostream& operator<<(std::ostream& out, const Datatype& dtype)
{
  return out << dtype.toString();
}

/* -------------------------------------------------------------------------- */
/* Options                                                                    */
/* -------------------------------------------------------------------------- */

DriverOptions::DriverOptions(const Solver& solver) : d_solver(solver) {}

std::istream& DriverOptions::in() const
{
  return *d_solver.d_slv->getOptions().io.in;
}
std::ostream& DriverOptions::err() const
{
  return *d_solver.d_slv->getOptions().io.err;
}
std::ostream& DriverOptions::out() const
{
  return *d_solver.d_slv->getOptions().io.out;
}

/* -------------------------------------------------------------------------- */
/* Statistics                                                                 */
/* -------------------------------------------------------------------------- */

struct Stat::StatData
{
  internal::StatExportData data;
  template <typename T>
  StatData(T&& t) : data(std::forward<T>(t))
  {
  }
  StatData() : data() {}
};

Stat::Stat() {}
Stat::~Stat() {}
Stat::Stat(const Stat& s) : d_internal(s.d_internal), d_default(s.d_default)
{
  if (s.d_data)
  {
    d_data = std::make_unique<StatData>(s.d_data->data);
  }
}
Stat& Stat::operator=(const Stat& s)
{
  d_internal = s.d_internal;
  d_default = s.d_default;
  if (s.d_data)
  {
    d_data = std::make_unique<StatData>(s.d_data->data);
  }
  return *this;
}

bool Stat::isInternal() const { return d_internal; }
bool Stat::isDefault() const { return d_default; }

bool Stat::isInt() const
{
  if (!d_data) return false;
  return std::holds_alternative<int64_t>(d_data->data);
}
int64_t Stat::getInt() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(static_cast<bool>(d_data))
      << "Stat holds no value";
  AVA6_API_RECOVERABLE_CHECK(isInt()) << "expected Stat of type int64_t.";
  return std::get<int64_t>(d_data->data);
  AVA6_API_TRY_CATCH_END;
}
bool Stat::isDouble() const
{
  if (!d_data) return false;
  return std::holds_alternative<double>(d_data->data);
}
double Stat::getDouble() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(static_cast<bool>(d_data))
      << "Stat holds no value";
  AVA6_API_RECOVERABLE_CHECK(isDouble()) << "expected Stat of type double.";
  return std::get<double>(d_data->data);
  AVA6_API_TRY_CATCH_END;
}
bool Stat::isString() const
{
  if (!d_data) return false;
  return std::holds_alternative<std::string>(d_data->data);
}
const std::string& Stat::getString() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(static_cast<bool>(d_data))
      << "Stat holds no value";
  AVA6_API_RECOVERABLE_CHECK(isString())
      << "expected Stat of type std::string.";
  return std::get<std::string>(d_data->data);
  AVA6_API_TRY_CATCH_END;
}
bool Stat::isHistogram() const
{
  if (!d_data) return false;
  return std::holds_alternative<HistogramData>(d_data->data);
}
const Stat::HistogramData& Stat::getHistogram() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(static_cast<bool>(d_data))
      << "Stat holds no value";
  AVA6_API_RECOVERABLE_CHECK(isHistogram())
      << "expected Stat of type histogram.";
  return std::get<HistogramData>(d_data->data);
  AVA6_API_TRY_CATCH_END;
}

Stat::Stat(bool internal, bool defaulted, StatData&& sd)
    : d_internal(internal),
      d_default(defaulted),
      d_data(std::make_unique<StatData>(std::move(sd)))
{
}

std::string Stat::toString() const
{
  std::stringstream ss;
  internal::detail::print(ss, d_data->data);
  return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Stat& stat)
{
  return internal::detail::print(os, stat.d_data->data);
}

Statistics::BaseType::const_reference Statistics::iterator::operator*() const
{
  return d_it.operator*();
}
Statistics::BaseType::const_pointer Statistics::iterator::operator->() const
{
  return d_it.operator->();
}
Statistics::iterator& Statistics::iterator::operator++()
{
  do
  {
    ++d_it;
  } while (!isVisible());
  return *this;
}
Statistics::iterator Statistics::iterator::operator++(int)
{
  iterator tmp = *this;
  do
  {
    ++d_it;
  } while (!isVisible());
  return tmp;
}
Statistics::iterator& Statistics::iterator::operator--()
{
  do
  {
    --d_it;
  } while (!isVisible());
  return *this;
}
Statistics::iterator Statistics::iterator::operator--(int)
{
  iterator tmp = *this;
  do
  {
    --d_it;
  } while (!isVisible());
  return tmp;
}
bool Statistics::iterator::operator==(const Statistics::iterator& rhs) const
{
  return d_it == rhs.d_it;
}
bool Statistics::iterator::operator!=(const Statistics::iterator& rhs) const
{
  return d_it != rhs.d_it;
}
Statistics::iterator::iterator(Statistics::BaseType::const_iterator it,
                               const Statistics::BaseType& base,
                               bool internal,
                               bool defaulted)
    : d_it(it),
      d_base(&base),
      d_showInternal(internal),
      d_showDefault(defaulted)
{
  while (!isVisible())
  {
    ++d_it;
  }
}
bool Statistics::iterator::isVisible() const
{
  if (d_it == d_base->end()) return true;
  if (!d_showInternal && d_it->second.isInternal()) return false;
  if (!d_showDefault && d_it->second.isDefault()) return false;
  return true;
}

const Stat& Statistics::get(const std::string& name)
{
  AVA6_API_TRY_CATCH_BEGIN;
  auto it = d_stats.find(name);
  AVA6_API_RECOVERABLE_CHECK(it != d_stats.end())
      << "no statistic with name \"" << name << "\" exists.";
  return it->second;
  AVA6_API_TRY_CATCH_END;
}

Statistics::iterator Statistics::begin(bool internal, bool defaulted) const
{
  return iterator(d_stats.begin(), d_stats, internal, defaulted);
}
Statistics::iterator Statistics::end() const
{
  return iterator(d_stats.end(), d_stats, false, false);
}

Statistics::Statistics(const internal::StatisticsRegistry& reg)
{
  for (const auto& svp : reg)
  {
    d_stats.emplace(svp.first,
                    Stat(svp.second->d_internal,
                         svp.second->isDefault(),
                         svp.second->getViewer()));
  }
}

std::string Statistics::toString() const
{
  std::stringstream ss;
  for (const auto& stat : *this)
  {
    ss << stat.first << " = " << stat.second << std::endl;
  }
  return ss.str();
}

std::ostream& operator<<(std::ostream& out, const Statistics& stats)
{
  out << stats.toString();
  return out;
}

/*--------------------------------------------------------------------------- */
/* Proof                                                                      */
/* -------------------------------------------------------------------------- */

Proof::Proof() : d_nm(nullptr) {}

Proof::Proof(NodeManagerSharedPtr nm,
             const std::shared_ptr<internal::ProofNode> p)
    : d_nm(std::move(nm)), d_proofNode(p)
{
}

Proof::~Proof() {}

bool Proof::isNull() const { return d_proofNode == nullptr; }

ProofRule Proof::getRule() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  if (d_proofNode != nullptr)
  {
    return d_proofNode->getRule();
  }
  return ProofRule::UNKNOWN;
  ////////
  AVA6_API_TRY_CATCH_END;
}

ProofRewriteRule Proof::getRewriteRule() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(d_proofNode->getRule() == ProofRule::DSL_REWRITE
                 || d_proofNode->getRule() == ProofRule::THEORY_REWRITE)
      << "expected `getRule()` to return `DSL_REWRITE` or `THEORY_REWRITE`, "
         "got "
      << d_proofNode->getRule() << " instead.";
  //////// all checks before this line
  if (d_proofNode != nullptr)
  {
    return static_cast<ProofRewriteRule>(
        detail::getInteger(d_proofNode->getArguments()[0]).getUnsignedInt());
  }
  return ProofRewriteRule::NONE;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Proof::getResult() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  if (d_proofNode != nullptr)
  {
    return Term(d_nm, d_proofNode->getResult());
  }
  return Term();
  ////////
  AVA6_API_TRY_CATCH_END;
}

const std::vector<Proof> Proof::getChildren() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  if (d_proofNode != nullptr)
  {
    std::vector<Proof> children;
    std::vector<std::shared_ptr<internal::ProofNode>> nodeChildren =
        d_proofNode->getChildren();
    for (size_t i = 0, psize = nodeChildren.size(); i < psize; i++)
    {
      children.push_back(Proof(d_nm, nodeChildren[i]));
    }
    return children;
  }
  return std::vector<Proof>();
  ////////
  AVA6_API_TRY_CATCH_END;
}

const std::vector<Term> Proof::getArguments() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  if (d_proofNode != nullptr)
  {
    std::vector<Term> args;
    const std::vector<internal::Node> nodeArgs = d_proofNode->getArguments();
    for (size_t i = 0, asize = nodeArgs.size(); i < asize; i++)
    {
      args.push_back(Term(d_nm, nodeArgs[i]));
    }
    return args;
  }
  return std::vector<Term>();
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Proof::operator==(const Proof& p) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_proofNode == p.d_proofNode;
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Proof::operator!=(const Proof& p) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_proofNode != p.d_proofNode;
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* -------------------------------------------------------------------------- */
/* Plugin                                                                     */
/* -------------------------------------------------------------------------- */

Plugin::Plugin(TermManager& tm)
    : d_pExtToInt(new PluginInternal(tm.d_nm, *this))
{
}

std::vector<Term> Plugin::check() { return {}; }
void Plugin::notifySatClause(AVA6_UNUSED const Term& clause) {}
void Plugin::notifyTheoryLemma(AVA6_UNUSED const Term& lemma) {}

/* -------------------------------------------------------------------------- */
/* TermManager                                                                */
/* -------------------------------------------------------------------------- */

TermManager::TermManager() : d_nm(std::make_unique<internal::NodeManager>())
{
  if constexpr (internal::configuration::isStatisticsBuild())
  {
    d_statsReg.reset(new internal::StatisticsRegistry());
    resetStatistics();
  }
}

TermManager::~TermManager() {}

Statistics TermManager::getStatistics() const
{
  return Statistics(*d_statsReg);
}

void TermManager::printStatisticsSafe(int fd) const
{
  d_statsReg->printSafe(fd);
}

/* Helpers and private functions                                              */
/* -------------------------------------------------------------------------- */

void TermManager::increment_term_stats(Kind kind) const
{
  if constexpr (internal::configuration::isStatisticsBuild())
  {
    d_stats->d_terms << kind;
  }
}

void TermManager::increment_vars_consts_stats(const internal::TypeNode& type,
                                              bool is_var) const
{
  if constexpr (internal::configuration::isStatisticsBuild())
  {
    internal::TypeConstant tc = type.getKind() == internal::Kind::TYPE_CONSTANT
                                    ? type.getConst<internal::TypeConstant>()
                                    : internal::LAST_TYPE;
    if (is_var)
    {
      d_stats->d_vars << tc;
    }
    else
    {
      d_stats->d_consts << tc;
    }
  }
}

void TermManager::resetStatistics()
{
  d_stats.reset(new APIStatistics{
      d_statsReg->registerHistogram<internal::TypeConstant>("ava6::CONSTANT"),
      d_statsReg->registerHistogram<internal::TypeConstant>("ava6::VARIABLE"),
      d_statsReg->registerHistogram<Kind>("ava6::TERM"),
  });
}

void TermManager::checkMkTerm(Kind kind, uint32_t nchildren) const
{
  AVA6_API_KIND_CHECK(kind);
  Assert(isDefinedIntKind(extToIntKind(kind)));
  const internal::kind::MetaKind mk =
      internal::kind::metaKindOf(extToIntKind(kind));
  AVA6_API_KIND_CHECK_EXPECTED(mk == internal::kind::metakind::PARAMETERIZED
                                   || mk == internal::kind::metakind::OPERATOR,
                               kind)
      << "Only operator-style terms are created with mkTerm(), "
         "to create variables, constants and values see mkVar(), mkConst() "
         "and the respective theory-specific functions to create values, "
         "e.g., mkBitVector().";
  AVA6_API_KIND_CHECK_EXPECTED(
      nchildren >= minArity(kind) && nchildren <= maxArity(kind), kind)
      << "Terms with kind " << std::to_string(kind) << " must have at least "
      << minArity(kind) << " children and at most " << maxArity(kind)
      << " children (the one under construction has " << nchildren << ")";
}

bool TermManager::isValidInteger(const std::string& s) const
{
  //////// all checks before this line
  if (s.length() == 0)
  {
    // string should not be empty
    return false;
  }

  size_t index = 0;
  if (s[index] == '-')
  {
    if (s.length() == 1)
    {
      // negative integers should contain at least one digit
      return false;
    }
    index = 1;
  }

  if (s[index] == '0' && s.length() > (index + 1))
  {
    // From SMT-Lib 2.6: A <numeral> is the digit 0 or a non-empty sequence of
    // digits not starting with 0. So integers like 001, 000 are not allowed
    return false;
  }

  // all remaining characters should be decimal digits
  for (; index < s.length(); ++index)
  {
    if (!std::isdigit(s[index]))
    {
      return false;
    }
  }

  return true;
}

// This helpers are split out to avoid nested API calls (problematic with API
// tracing).

internal::Node TermManager::mkVarHelper(
    const internal::TypeNode& type, const std::optional<std::string>& symbol)
{
  internal::Node res = symbol ? internal::NodeManager::mkBoundVar(*symbol, type)
                              : internal::NodeManager::mkBoundVar(type);
  (void)res.getType(true); /* kick off type checking */
  increment_vars_consts_stats(type, true);
  return res;
}

internal::Node TermManager::mkConstHelper(
    const internal::TypeNode& type,
    const std::optional<std::string>& symbol,
    bool fresh)
{
  Assert(fresh || symbol);
  internal::Node res =
      symbol ? d_nm->mkVar(*symbol, type, fresh) : d_nm->mkVar(type);
  (void)res.getType(true); /* kick off type checking */
  increment_vars_consts_stats(type, false);
  return res;
}

template <typename T>
Op TermManager::mkOpHelper(Kind kind, const T& t)
{
  //////// all checks before this line
  internal::Node res = d_nm->mkConst(s_op_kinds.at(kind), t);
  static_cast<void>(res.getType(true)); /* kick off type checking */
  return Op(d_nm, kind, res);
}

template <typename T>
Term TermManager::mkValHelper(NodeManagerSharedPtr nm, const T& t)
{
  //////// all checks before this line
  internal::Node res = nm->mkConst(t);
  (void)res.getType(true); /* kick off type checking */
  return Term(nm, res);
}

Term TermManager::mkRationalValHelper(NodeManagerSharedPtr nm,
                                      const internal::Rational& r,
                                      bool isInt)
{
  //////// all checks before this line
  internal::Node res = isInt ? nm->mkConstInt(r) : nm->mkConstReal(r);
  (void)res.getType(true); /* kick off type checking */
  return Term(nm, res);
}

Term TermManager::mkRealOrIntegerFromStrHelper(const std::string& s, bool isInt)
{
  //////// all checks before this line
  try
  {
    internal::Rational r;
    size_t spos = s.find('/');
    if (spos != std::string::npos)
    {
      // Ensure the denominator contains a non-zero digit. We catch this here to
      // avoid a floating point exception in GMP. This exception will be caught
      // and given the standard error message below.
      if (s.find_first_not_of('0', spos + 1) == std::string::npos)
      {
        throw std::invalid_argument("Zero denominator encountered");
      }
      r = internal::Rational(s);
    }
    else
    {
      r = internal::Rational::fromDecimal(s);
    }
    return TermManager::mkRationalValHelper(d_nm, r, isInt);
  }
  catch (const std::invalid_argument& e)
  {
    /* Catch to throw with a more meaningful error message. To be caught in
     * enclosing AVA6_API_TRY_CATCH_* block to throw Ava6ApiException. */
    std::stringstream message;
    message << "cannot construct Real or Int from string argument '" << s
            << "'";
    throw std::invalid_argument(message.str());
  }
}

Term TermManager::mkBVFromIntHelper(uint32_t size, uint64_t val)
{
  AVA6_API_ARG_CHECK_EXPECTED(size > 0, size) << "a bit-width > 0";
  //////// all checks before this line
  return mkValHelper(d_nm, internal::BitVector(size, val));
}

Term TermManager::mkBVFromStrHelper(uint32_t size,
                                    const std::string& s,
                                    uint32_t base)
{
  AVA6_API_ARG_CHECK_EXPECTED(size > 0, size) << "a bit-width > 0";
  AVA6_API_ARG_CHECK_EXPECTED(!s.empty(), s) << "a non-empty string";
  AVA6_API_ARG_CHECK_EXPECTED(base == 2 || base == 10 || base == 16, base)
      << "base 2, 10, or 16";
  //////// all checks before this line

  internal::Integer val(s, base);

  if (val.strictlyNegative())
  {
    AVA6_API_CHECK(val >= -internal::Integer(2).pow(size - 1))
        << "overflow in bit-vector construction (specified bit-vector size "
        << size << " too small to hold value " << s << ")";
  }
  else
  {
    AVA6_API_CHECK(val.modByPow2(size) == val)
        << "overflow in bit-vector construction (specified bit-vector size "
        << size << " too small to hold value " << s << ")";
  }
  return mkValHelper(d_nm, internal::BitVector(size, val));
}

Sort TermManager::mkTupleSortHelper(const std::vector<Sort>& sorts)
{
  // Note: Sorts are checked in the caller to avoid double checks
  //////// all checks before this line
  std::vector<internal::TypeNode> typeNodes =
      Sort::sortVectorToTypeNodes(sorts);
  return Sort(d_nm, d_nm->mkTupleType(typeNodes));
}

Term TermManager::mkTermFromKind(Kind kind)
{
  AVA6_API_KIND_CHECK_EXPECTED(
      kind == Kind::PI || kind == Kind::REGEXP_NONE || kind == Kind::REGEXP_ALL
          || kind == Kind::REGEXP_ALLCHAR,
      kind)
      << "PI, REGEXP_NONE, REGEXP_ALL, REGEXP_ALLCHAR";
  //////// all checks before this line
  internal::Node res;
  internal::Kind k = extToIntKind(kind);
  if (kind == Kind::REGEXP_NONE || kind == Kind::REGEXP_ALL
      || kind == Kind::REGEXP_ALLCHAR)
  {
    Assert(isDefinedIntKind(k));
    res = d_nm->mkNode(k, std::vector<internal::Node>());
  }
  else
  {
    Assert(kind == Kind::PI);
    res = d_nm->mkNullaryOperator(d_nm->realType(), k);
  }
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
}

Term TermManager::mkTermHelper(Kind kind, const std::vector<Term>& children)
{
  // Note: Kind and children are checked in the caller to avoid double checks
  //////// all checks before this line
  if (children.size() == 0)
  {
    return mkTermFromKind(kind);
  }
  std::vector<internal::Node> echildren = Term::termVectorToNodes(children);
  internal::Kind k = extToIntKind(kind);
  internal::Node res;
  if (echildren.size() > 2)
  {
    if (kind == Kind::INTS_DIVISION || kind == Kind::XOR || kind == Kind::SUB
        || kind == Kind::DIVISION || false
        || kind == Kind::REGEXP_DIFF || kind == Kind::SET_UNION
        || kind == Kind::SET_INTER || kind == Kind::SET_MINUS)
    {
      // left-associative, but ava6 internally only supports 2 args
      res = d_nm->mkLeftAssociative(k, echildren);
    }
    else if (kind == Kind::IMPLIES)
    {
      // right-associative, but ava6 internally only supports 2 args
      res = d_nm->mkRightAssociative(k, echildren);
    }
    else if (kind == Kind::EQUAL || kind == Kind::LT || kind == Kind::GT
             || kind == Kind::LEQ || kind == Kind::GEQ)
    {
      // "chainable", but ava6 internally only supports 2 args
      res = d_nm->mkChain(k, echildren);
    }
    else if (internal::kind::isAssociative(k))
    {
      // mkAssociative has special treatment for associative operators with lots
      // of children
      res = d_nm->mkAssociative(k, echildren);
    }
    else
    {
      // default case, must check kind
      checkMkTerm(kind, children.size());
      res = d_nm->mkNode(k, echildren);
    }
  }
  else if (internal::kind::isAssociative(k))
  {
    // associative case, same as above
    checkMkTerm(kind, children.size());
    res = d_nm->mkAssociative(k, echildren);
  }
  else
  {
    // default case, same as above
    checkMkTerm(kind, children.size());
    // make the term
    res = d_nm->mkNode(k, echildren);
  }

  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
}

Term TermManager::mkTermHelper(const Op& op, const std::vector<Term>& children)
{
  if (!op.isIndexedHelper())
  {
    return mkTermHelper(op.d_kind, children);
  }

  // Note: Op and children are checked in the caller to avoid double checks
  checkMkTerm(op.d_kind, children.size());
  //////// all checks before this line

  const internal::Kind int_kind = extToIntKind(op.d_kind);
  std::vector<internal::Node> echildren = Term::termVectorToNodes(children);

  internal::NodeBuilder nb(d_nm.get(), int_kind);
  nb << *op.d_node;
  nb.append(echildren);
  internal::Node res = nb.constructNode();

  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
}

/* Sorts -------------------------------------------------------------------- */

Sort TermManager::getBooleanSort(void)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Sort(d_nm, d_nm->booleanType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::getIntegerSort(void)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Sort(d_nm, d_nm->integerType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::getRealSort(void)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Sort(d_nm, d_nm->realType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::getRegExpSort(void)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Sort(d_nm, d_nm->regExpType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::getStringSort(void)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Sort(d_nm, d_nm->stringType());
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkArraySort(const Sort& indexSort, const Sort& elemSort)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_SORT(indexSort);
  AVA6_API_TM_CHECK_SORT(elemSort);
  //////// all checks before this line
  return Sort(d_nm, d_nm->mkArrayType(*indexSort.d_type, *elemSort.d_type));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkBitVectorSort(uint32_t size)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_ARG_CHECK_EXPECTED(size > 0, size) << "size > 0";
  //////// all checks before this line
  return Sort(d_nm, d_nm->mkBitVectorType(size));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkDatatypeSort(const DatatypeDecl& dtypedecl)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_DTDECL(dtypedecl);
  //////// all checks before this line
  Sort res = Sort(d_nm, d_nm->mkDatatypeType(*dtypedecl.d_dtype));
  const Datatype& dt = res.getDatatype();
  AVA6_API_CHECK(dt.d_dtype->isWellFounded())
      << "Datatype sort " << dt.d_dtype->getName() + " is not well-founded";
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Sort> TermManager::mkDatatypeSorts(
    const std::vector<DatatypeDecl>& dtypedecls)
{
  AVA6_API_TM_CHECK_DTDECLS(dtypedecls);
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  std::vector<internal::DType> datatypes;
  for (size_t i = 0, ndts = dtypedecls.size(); i < ndts; ++i)
  {
    datatypes.push_back(dtypedecls[i].getDatatype());
  }
  std::vector<internal::TypeNode> dtypes =
      d_nm->mkMutualDatatypeTypes(datatypes);
  std::vector<Sort> res = Sort::typeNodeVectorToSorts(d_nm, dtypes);
  for (size_t i = 0, ndts = datatypes.size(); i < ndts; ++i)
  {
    const Datatype& dt = res[i].getDatatype();
    AVA6_API_CHECK(dt.d_dtype->isWellFounded())
        << "Datatype sort " << dt.d_dtype->getName() + " is not well-founded";
  }
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkFunctionSort(const std::vector<Sort>& sorts,
                                 const Sort& codomain)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_ARG_SIZE_CHECK_EXPECTED(sorts.size() >= 1, sorts)
      << "at least one parameter sort for function sort";
  AVA6_API_TM_CHECK_DOMAIN_SORTS(sorts);
  AVA6_API_TM_CHECK_CODOMAIN_SORT(codomain);
  for (const Sort& sort : sorts)
  {
    AVA6_API_CHECK(!sort.isFunction()) << "Function-valued arguments are not supported";
  }
  AVA6_API_CHECK(!codomain.isFunction()) << "Function-valued results are not supported";
  //////// all checks before this line
  std::vector<internal::TypeNode> argTypes = Sort::sortVectorToTypeNodes(sorts);
  return Sort(d_nm, d_nm->mkFunctionType(argTypes, *codomain.d_type));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkSkolem(SkolemId id, const std::vector<Term>& indices)
{
  AVA6_API_TRY_CATCH_BEGIN;
  ava6::internal::SkolemManager* sm = d_nm->getSkolemManager();
  AVA6_API_CHECK(indices.size() == sm->getNumIndicesForSkolemId(id))
      << "invalid number of indices, expected "
      << sm->getNumIndicesForSkolemId(id) << " got " << indices.size();
  //////// all checks before this line
  // iterate over indices and convert the Terms to Nodes
  std::vector<internal::Node> nodeIndices = Term::termVectorToNodes(indices);
  // automatically sort if a commutative skolem
  if (internal::SkolemManager::isCommutativeSkolemId(id))
  {
    std::sort(nodeIndices.begin(), nodeIndices.end());
  }
  internal::Node res =
      d_nm->getSkolemManager()->mkSkolemFunction(id, nodeIndices);
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

size_t TermManager::getNumIndicesForSkolemId(SkolemId id)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_nm->getSkolemManager()->getNumIndicesForSkolemId(id);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkParamSort(const std::optional<std::string>& symbol)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  internal::TypeNode tn = symbol ? d_nm->mkSort(*symbol) : d_nm->mkSort();
  return Sort(d_nm, tn);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkPredicateSort(const std::vector<Sort>& sorts)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_ARG_SIZE_CHECK_EXPECTED(sorts.size() >= 1, sorts)
      << "at least one parameter sort for predicate sort";
  AVA6_API_TM_CHECK_DOMAIN_SORTS(sorts);
  //////// all checks before this line
  return Sort(d_nm, d_nm->mkPredicateType(Sort::sortVectorToTypeNodes(sorts)));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkRecordSort(
    const std::vector<std::pair<std::string, Sort>>& fields)
{
  AVA6_API_TRY_CATCH_BEGIN;
  std::vector<std::pair<std::string, internal::TypeNode>> f;
  for (size_t i = 0, size = fields.size(); i < size; ++i)
  {
    const auto& p = fields[i];
    AVA6_API_TM_CHECK_SORT_AT_INDEX(p.second, fields, i);
    f.emplace_back(p.first, *p.second.d_type);
  }
  //////// all checks before this line
  return Sort(d_nm, d_nm->mkRecordType(f));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkSetSort(const Sort& elemSort)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_SORT(elemSort);
  //////// all checks before this line
  return Sort(d_nm, d_nm->mkSetType(*elemSort.d_type));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkSequenceSort(const Sort& elemSort)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_SORT(elemSort);
  //////// all checks before this line
  return Sort(d_nm, d_nm->mkSequenceType(*elemSort.d_type));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkAbstractSort(SortKind k)
{
  AVA6_API_TRY_CATCH_BEGIN;
  internal::Kind ik = extToIntSortKind(k);
  AVA6_API_CHECK(d_nm->isSortKindAbstractable(ik))
      << "cannot construct abstract type for kind " << k;
  //////// all checks before this line
  return Sort(d_nm, d_nm->mkAbstractType(ik));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkUninterpretedSort(const std::optional<std::string>& symbol)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  internal::TypeNode tn = symbol ? d_nm->mkSort(*symbol) : d_nm->mkSort();
  return Sort(d_nm, tn);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkUnresolvedDatatypeSort(const std::string& symbol,
                                           size_t arity)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Sort(d_nm, d_nm->mkUnresolvedDatatypeSort(symbol, arity));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkUninterpretedSortConstructorSort(
    size_t arity, const std::optional<std::string>& symbol)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_ARG_CHECK_EXPECTED(arity > 0, arity) << "an arity > 0";
  //////// all checks before this line
  if (symbol)
  {
    return Sort(d_nm, d_nm->mkSortConstructor(*symbol, arity));
  }
  return Sort(d_nm, d_nm->mkSortConstructor("", arity));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort TermManager::mkTupleSort(const std::vector<Sort>& sorts)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_DOMAIN_SORTS(sorts);
  //////// all checks before this line
  return mkTupleSortHelper(sorts);
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Terms -------------------------------------------------------------------- */

Term TermManager::mkTerm(Kind kind, const std::vector<Term>& children)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_KIND_CHECK(kind);
  AVA6_API_TM_CHECK_TERMS(children);
  //////// all checks before this line
  Term res = mkTermHelper(kind, children);
  increment_term_stats(kind);
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkTerm(const Op& op, const std::vector<Term>& children)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_OP(op);
  AVA6_API_TM_CHECK_TERMS(children);
  //////// all checks before this line
  Term res = mkTermHelper(op, children);
  increment_term_stats(op.getKind());
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Operators ---------------------------------------------------------------- */

Op TermManager::mkOp(Kind kind, const std::vector<uint32_t>& args)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_KIND_CHECK(kind);
  //////// all checks before this line
  size_t nargs = args.size();

  Op res;
  switch (kind)
  {
    case Kind::BITVECTOR_EXTRACT:
      AVA6_API_OP_CHECK_ARITY(nargs, 2, kind);
      res = mkOpHelper(kind, internal::BitVectorExtract(args[0], args[1]));
      break;
    case Kind::BITVECTOR_REPEAT:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::BitVectorRepeat(args[0]));
      break;
    case Kind::BITVECTOR_ROTATE_LEFT:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::BitVectorRotateLeft(args[0]));
      break;
    case Kind::BITVECTOR_ROTATE_RIGHT:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::BitVectorRotateRight(args[0]));
      break;
    case Kind::BITVECTOR_SIGN_EXTEND:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::BitVectorSignExtend(args[0]));
      break;
    case Kind::BITVECTOR_ZERO_EXTEND:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::BitVectorZeroExtend(args[0]));
      break;
    case Kind::BITVECTOR_BIT:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::BitVectorBit(args[0]));
      break;
    case Kind::DIVISIBLE:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      AVA6_API_CHECK_OP_INDEX(args[0] != 0, args, 0) << "a value != 0";
      res = mkOpHelper(kind, internal::Divisible(args[0]));
      break;
    case Kind::IAND:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::IntAnd(args[0]));
      break;
    case Kind::INT_TO_BITVECTOR:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::IntToBitVector(args[0]));
      break;
    case Kind::REGEXP_REPEAT:
      AVA6_API_OP_CHECK_ARITY(nargs, 1, kind);
      res = mkOpHelper(kind, internal::RegExpRepeat(args[0]));
      break;
    case Kind::REGEXP_LOOP:
      AVA6_API_OP_CHECK_ARITY(nargs, 2, kind);
      res = mkOpHelper(kind, internal::RegExpLoop(args[0], args[1]));
      break;
    case Kind::TUPLE_PROJECT:
      res = mkOpHelper(kind, internal::ProjectOp(args));
      break;
    default:
      if (nargs == 0)
      {
        AVA6_API_CHECK(s_indexed_kinds.find(kind) == s_indexed_kinds.end())
            << "expected a kind for a non-indexed operator.";
        return Op(d_nm, kind);
      }
      else
      {
        AVA6_API_KIND_CHECK_EXPECTED(false, kind)
            << "operator kind with " << nargs << " uint32_t arguments";
      }
  }
  Assert(!res.isNull());
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Op TermManager::mkOp(Kind kind, const std::initializer_list<uint32_t>& args)
{
  return mkOp(kind, std::vector<uint32_t>(args));
}

Op TermManager::mkOp(Kind kind, const std::string& arg)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_KIND_CHECK(kind);
  AVA6_API_KIND_CHECK_EXPECTED((kind == Kind::DIVISIBLE), kind) << "DIVISIBLE";
  //////// all checks before this line
  Op res;
  /* CLN and GMP handle this case differently, CLN interprets it as 0, GMP
   * throws an std::invalid_argument exception. For consistency, we treat it
   * as invalid. */
  AVA6_API_ARG_CHECK_EXPECTED(arg != ".", arg)
      << "a string representing an integer, real or rational value.";
  res = mkOpHelper(kind, internal::Divisible(internal::Integer(arg)));
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Constants, Values and Special Terms -------------------------------------- */

Term TermManager::mkTrue(void)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Term(d_nm, d_nm->mkConst<bool>(true));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkFalse(void)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Term(d_nm, d_nm->mkConst<bool>(false));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkBoolean(bool val)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return Term(d_nm, d_nm->mkConst<bool>(val));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkPi()
{
  throw Ava6ApiException("This constructor is not part of the core SMT language");
}

Term TermManager::mkInteger(const std::string& s)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_ARG_CHECK_EXPECTED(isValidInteger(s), s) << "an integer ";
  Term res = mkRealOrIntegerFromStrHelper(s);
  AVA6_API_ARG_CHECK_EXPECTED(AVA6_EQUAL(res.getSort(), getIntegerSort()), s)
      << "a string representing an integer";
  //////// all checks before this line
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkInteger(int64_t val)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  Term res =
      TermManager::mkRationalValHelper(d_nm, internal::Rational(val), true);
  AssertEqual(res.getSort(), getIntegerSort());
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkReal(const std::string& s)
{
  AVA6_API_TRY_CATCH_BEGIN;
  /* CLN and GMP handle this case differently, CLN interprets it as 0, GMP
   * throws an std::invalid_argument exception. For consistency, we treat it
   * as invalid. */
  AVA6_API_ARG_CHECK_EXPECTED(s != ".", s)
      << "a string representing a real or rational value.";
  //////// all checks before this line
  return mkRealOrIntegerFromStrHelper(s, false);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkReal(int64_t val)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return TermManager::mkRationalValHelper(d_nm, internal::Rational(val), false);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkReal(int64_t num, int64_t den)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(den != 0) << "invalid denominator '" << den << "'";
  //////// all checks before this line
  return TermManager::mkRationalValHelper(
      d_nm, internal::Rational(num, den), false);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkRegexpAll()
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  internal::Node res =
      d_nm->mkNode(internal::Kind::REGEXP_ALL, std::vector<internal::Node>());
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkRegexpNone()
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  internal::Node res =
      d_nm->mkNode(internal::Kind::REGEXP_NONE, std::vector<internal::Node>());
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkRegexpAllchar()
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  internal::Node res = d_nm->mkNode(internal::Kind::REGEXP_ALLCHAR,
                                    std::vector<internal::Node>());
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkEmptySet(const Sort& sort)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_SORT(sort);
  AVA6_API_ARG_CHECK_EXPECTED(sort.isSet(), sort) << "null sort or set sort";
  //////// all checks before this line
  return mkValHelper(d_nm, internal::EmptySet(*sort.d_type));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkString(const std::string& s, bool useEscSequences)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return mkValHelper(d_nm, internal::String(s, useEscSequences));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkString(const std::wstring& s)
{
  AVA6_API_TRY_CATCH_BEGIN;
  for (size_t i = 0, n = s.size(); i < n; ++i)
  {
    AVA6_API_CHECK(static_cast<unsigned>(s[i]) < internal::String::num_codes())
        << "Expected unicode string whose characters are less than code point "
        << internal::String::num_codes();
  }
  //////// all checks before this line
  return mkValHelper(d_nm, internal::String(s));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkString(const std::u32string& s)
{
  AVA6_API_TRY_CATCH_BEGIN;
  for (size_t i = 0, n = s.size(); i < n; ++i)
  {
    AVA6_API_CHECK(static_cast<unsigned>(s[i]) < internal::String::num_codes())
        << "Expected unicode string whose characters are less than code point "
        << internal::String::num_codes();
  }
  //////// all checks before this line
  return mkValHelper(d_nm, internal::String(s));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkEmptySequence(const Sort& sort)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_SORT(sort);
  //////// all checks before this line
  std::vector<internal::Node> seq;
  internal::Node res = d_nm->mkConst(internal::Sequence(*sort.d_type, seq));
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkBitVector(uint32_t size, uint64_t val)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return mkBVFromIntHelper(size, val);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkBitVector(uint32_t size,
                              const std::string& s,
                              uint32_t base)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return mkBVFromStrHelper(size, s, base);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkConstArray(const Sort&, const Term&)
{
  throw Ava6ApiException("This constructor is not part of the core SMT language");
}

/* Constants and Variables -------------------------------------------------- */

Term TermManager::mkConst(const Sort& sort,
                          const std::optional<std::string>& symbol)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_SORT(sort);
  //////// all checks before this line
  return Term(d_nm, mkConstHelper(*sort.d_type, symbol));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkVar(const Sort& sort,
                        const std::optional<std::string>& symbol)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_SORT(sort);
  //////// all checks before this line
  return Term(d_nm, mkVarHelper(*sort.d_type, symbol));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term TermManager::mkTuple(const std::vector<Term>& terms)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_TERMS(terms);
  //////// all checks before this line
  std::vector<internal::Node> args;
  std::vector<internal::TypeNode> typeNodes;
  for (size_t i = 0, size = terms.size(); i < size; i++)
  {
    internal::Node n = *terms[i].d_node;
    args.push_back(n);
    typeNodes.push_back(n.getType());
  }
  internal::TypeNode tn = d_nm->mkTupleType(typeNodes);
  const internal::DType& dt = tn.getDType();
  internal::NodeBuilder nb(d_nm.get(), extToIntKind(Kind::APPLY_CONSTRUCTOR));
  nb << dt[0].getConstructor();
  nb.append(args);
  internal::Node res = nb.constructNode();
  (void)res.getType(true); /* kick off type checking */
  return Term(d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Datatype Constructor Declaration ----------------------------------------- */

DatatypeConstructorDecl TermManager::mkDatatypeConstructorDecl(
    const std::string& name)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return DatatypeConstructorDecl(d_nm, name);
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* Datatype Declaration ----------------------------------------------------- */

DatatypeDecl TermManager::mkDatatypeDecl(const std::string& name)
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return DatatypeDecl(d_nm, name);
  ////////
  AVA6_API_TRY_CATCH_END;
}

DatatypeDecl TermManager::mkDatatypeDecl(const std::string& name,
                                         const std::vector<Sort>& params)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_TM_CHECK_SORTS(params);
  //////// all checks before this line
  return DatatypeDecl(d_nm, name, params);
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* -------------------------------------------------------------------------- */
/* Solver                                                                     */
/* -------------------------------------------------------------------------- */

Solver::Solver(TermManager& tm, std::unique_ptr<internal::Options>&& original)
    : d_tm(tm)
{
  d_originalOptions = std::move(original);
  resetInternal();
}

void Solver::resetInternal()
{
  // Release the previous engine, if any, before constructing the new one so
  // that two engines are never alive at the same time.
  d_slv.reset();
  d_slv.reset(
      new internal::SolverEngine(d_tm.d_nm.get(), d_originalOptions.get()));
  d_slv->setSolver(this);
  d_rng.reset(new internal::Random(d_slv->getOptions().driver.seed));
}

Solver::Solver(TermManager& tm)
    : Solver(tm, std::make_unique<internal::Options>())
{
}

Solver::~Solver() {}

/* Helpers and private functions                                              */
/* -------------------------------------------------------------------------- */

void Solver::ensureWellFormedTerm(const Term& t) const
{
  // only check if option is set
  {
    // Call isWellFormedTerm of the underlying solver, which checks if the
    // given node has free variables. We do not check for variable shadowing,
    // since this can be handled by our rewriter.
    if (!d_slv->isWellFormedTerm(*t.d_node))
    {
      std::stringstream se;
      se << "cannot process term " << *t.d_node << " with free variables"
         << std::endl;
      throw Ava6ApiException(se.str().c_str());
    }
  }
}

void Solver::ensureWellFormedTerms(const std::vector<Term>& ts) const
{
  // only check if option is set
  {
    for (const Term& t : ts)
    {
      ensureWellFormedTerm(t);
    }
  }
}

void Solver::printStatisticsSafe(int fd) const
{
  d_slv->printStatisticsSafe(fd);
}

/* Non-SMT-LIB commands                                                       */
/* -------------------------------------------------------------------------- */

Term Solver::simplify(const Term& term, bool applySubs)
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_SOLVER_CHECK_TERM(term);
  //////// all checks before this line
  Term res = Term(d_tm.d_nm, d_slv->simplify(*term.d_node, applySubs));
  AssertEqual(*res.getSort().d_type, *term.getSort().d_type);
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

/* SMT-LIB commands                                                           */
/* -------------------------------------------------------------------------- */

void Solver::assertFormula(const Term& term) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_SOLVER_CHECK_TERM(term);
  AVA6_API_SOLVER_CHECK_TERM_WITH_SORT(term, d_tm.getBooleanSort());
  ensureWellFormedTerm(term);
  //////// all checks before this line
  d_slv->assertFormula(*term.d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Result Solver::checkSat(void) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(!d_slv->isQueryMade()
                 || d_slv->getOptions().base.incrementalSolving)
      << "cannot make multiple queries unless incremental solving is enabled "
         "(try --"
      << internal::options::base::longName::incrementalSolving << ")";
  //////// all checks before this line
  return d_slv->checkSat();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Result Solver::checkSatAssuming(const Term& assumption) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(!d_slv->isQueryMade()
                 || d_slv->getOptions().base.incrementalSolving)
      << "cannot make multiple queries unless incremental solving is enabled "
         "(try --"
      << internal::options::base::longName::incrementalSolving << ")";
  AVA6_API_SOLVER_CHECK_TERM_WITH_SORT(assumption, d_tm.getBooleanSort());
  ensureWellFormedTerm(assumption);
  //////// all checks before this line
  return d_slv->checkSat(*assumption.d_node);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Result Solver::checkSatAssuming(const std::vector<Term>& assumptions) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(!d_slv->isQueryMade() || assumptions.size() == 0
                 || d_slv->getOptions().base.incrementalSolving)
      << "cannot make multiple queries unless incremental solving is enabled "
         "(try --"
      << internal::options::base::longName::incrementalSolving << ")";
  AVA6_API_SOLVER_CHECK_TERMS_WITH_SORT(assumptions, d_tm.getBooleanSort());
  ensureWellFormedTerms(assumptions);
  //////// all checks before this line
  for (const Term& term : assumptions)
  {
    AVA6_API_SOLVER_CHECK_TERM(term);
  }
  std::vector<internal::Node> eassumptions =
      Term::termVectorToNodes(assumptions);
  return d_slv->checkSat(eassumptions);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Solver::declareDatatype(
    const std::string& symbol,
    const std::vector<DatatypeConstructorDecl>& ctors) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_ARG_CHECK_EXPECTED(ctors.size() > 0, ctors)
      << "a datatype declaration with at least one constructor";
  for (size_t i = 0, size = ctors.size(); i < size; i++)
  {
    AVA6_API_SOLVER_CHECK_DTCTORDECL_AT_INDEX(ctors[i], ctorsr, i);
    AVA6_API_CHECK(!ctors[i].isResolved())
        << "cannot use a constructor for multiple datatypes";
  }
  //////// all checks before this line
  DatatypeDecl dtdecl(d_tm.d_nm, symbol);
  for (size_t i = 0, size = ctors.size(); i < size; i++)
  {
    dtdecl.addConstructor(ctors[i]);
  }
  return Sort(d_tm.d_nm, d_tm.d_nm->mkDatatypeType(*dtdecl.d_dtype));
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Solver::declareFun(const std::string& symbol,
                        const std::vector<Sort>& sorts,
                        const Sort& sort,
                        bool fresh) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_SOLVER_CHECK_DOMAIN_SORTS(sorts);
  AVA6_API_SOLVER_CHECK_CODOMAIN_SORT(sort);
  //////// all checks before this line

  internal::TypeNode type = *sort.d_type;
  if (!sorts.empty())
  {
    std::vector<internal::TypeNode> types = Sort::sortVectorToTypeNodes(sorts);
    type = d_tm.d_nm->mkFunctionType(types, type);
  }
  internal::Node res = d_tm.mkConstHelper(type, symbol, fresh);
  // notify the solver engine of the declaration
  d_slv->declareConst(res);
  return Term(d_tm.d_nm, res);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Sort Solver::declareSort(const std::string& symbol,
                         uint32_t arity,
                         bool fresh) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  internal::TypeNode type = d_tm.d_nm->mkSortConstructor(symbol, arity, fresh);
  // notify the solver engine of the declaration
  d_slv->declareSort(type);
  return Sort(d_tm.d_nm, type);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Solver::defineFun(const std::string& symbol,
                       const std::vector<Term>& bound_vars,
                       const Sort& sort,
                       const Term& term,
                       bool global) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_SOLVER_CHECK_CODOMAIN_SORT(sort);
  AVA6_API_SOLVER_CHECK_TERM(term);
  // the sort of the body must match the return sort
  AVA6_API_CHECK(term.d_node->getType() == *sort.d_type)
      << "invalid sort of function body '" << term << "', expected '" << sort
      << "', found '" << term.getSort() << "'";

  std::vector<Sort> domain_sorts;
  for (size_t i = 0, n = bound_vars.size(); i < n; ++i)
  {
    AVA6_API_SOLVER_CHECK_BOUND_VAR_AT_INDEX(bound_vars[i], bound_vars, i);
    domain_sorts.push_back(bound_vars[i].getSort());
  }
  Sort fun_sort =
      domain_sorts.empty()
          ? sort
          : Sort(d_tm.d_nm,
                 d_tm.d_nm->mkFunctionType(
                     Sort::sortVectorToTypeNodes(domain_sorts), *sort.d_type));
  Term fun = d_tm.mkConst(fun_sort, symbol);
  AVA6_API_SOLVER_CHECK_BOUND_VARS_DEF_FUN_SORTS(bound_vars, domain_sorts);
  //////// all checks before this line

  d_slv->defineFunction(
      *fun.d_node, Term::termVectorToNodes(bound_vars), *term.d_node, global);
  return fun;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Solver::defineFunRec(const std::string& symbol,
                          const std::vector<Term>& bound_vars,
                          const Sort& sort,
                          const Term& term,
                          bool global) const
{
  AVA6_API_TRY_CATCH_BEGIN;

  AVA6_API_CHECK(d_slv->getUserLogicInfo().isQuantified())
      << "recursive function definitions require a logic with quantifiers";
  AVA6_API_CHECK(
      d_slv->getUserLogicInfo().isTheoryEnabled(internal::theory::THEORY_UF))
      << "recursive function definitions require a logic with uninterpreted "
         "functions";

  AVA6_API_SOLVER_CHECK_TERM(term);
  AVA6_API_SOLVER_CHECK_CODOMAIN_SORT(sort);
  AVA6_API_CHECK(term.d_node->getType() == *sort.d_type)
      << "invalid sort of function body '" << term << "', expected '" << sort
      << "'";

  std::vector<Sort> domain_sorts;
  for (size_t i = 0, n = bound_vars.size(); i < n; ++i)
  {
    AVA6_API_SOLVER_CHECK_BOUND_VAR_AT_INDEX(bound_vars[i], bound_vars, i);
    domain_sorts.push_back(bound_vars[i].getSort());
  }
  Sort fun_sort =
      domain_sorts.empty()
          ? sort
          : Sort(d_tm.d_nm,
                 d_tm.d_nm->mkFunctionType(
                     Sort::sortVectorToTypeNodes(domain_sorts), *sort.d_type));
  Term fun = d_tm.mkConst(fun_sort, symbol);
  AVA6_API_SOLVER_CHECK_BOUND_VARS_DEF_FUN_SORTS(bound_vars, domain_sorts);
  //////// all checks before this line

  d_slv->defineFunctionRec(
      *fun.d_node, Term::termVectorToNodes(bound_vars), *term.d_node, global);

  return fun;
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Solver::defineFunRec(const Term& fun,
                          const std::vector<Term>& bound_vars,
                          const Term& term,
                          bool global) const
{
  AVA6_API_TRY_CATCH_BEGIN;

  AVA6_API_CHECK(d_slv->getUserLogicInfo().isQuantified())
      << "recursive function definitions require a logic with quantifiers";
  AVA6_API_CHECK(
      d_slv->getUserLogicInfo().isTheoryEnabled(internal::theory::THEORY_UF))
      << "recursive function definitions require a logic with uninterpreted "
         "functions";

  AVA6_API_SOLVER_CHECK_TERM(fun);
  AVA6_API_SOLVER_CHECK_TERM(term);
  if (fun.getSort().isFunction())
  {
    std::vector<Sort> domain_sorts = fun.getSort().getFunctionDomainSorts();
    AVA6_API_SOLVER_CHECK_BOUND_VARS_DEF_FUN_SORTS(bound_vars, domain_sorts);
    Sort codomain = fun.getSort().getFunctionCodomainSort();
    AVA6_API_CHECK(*codomain.d_type == term.d_node->getType())
        << "invalid sort of function body '" << term << "', expected '"
        << codomain << "'";
  }
  else
  {
    AVA6_API_SOLVER_CHECK_BOUND_VARS(bound_vars);
    AVA6_API_ARG_CHECK_EXPECTED(bound_vars.size() == 0, fun)
        << "function or nullary symbol";
  }
  //////// all checks before this line

  std::vector<internal::Node> ebound_vars = Term::termVectorToNodes(bound_vars);
  d_slv->defineFunctionRec(*fun.d_node, ebound_vars, *term.d_node, global);
  return fun;
  ////////
  AVA6_API_TRY_CATCH_END;
}

void Solver::defineFunsRec(const std::vector<Term>& funs,
                           const std::vector<std::vector<Term>>& bound_vars,
                           const std::vector<Term>& terms,
                           bool global) const
{
  AVA6_API_TRY_CATCH_BEGIN;

  AVA6_API_CHECK(d_slv->getUserLogicInfo().isQuantified())
      << "recursive function definitions require a logic with quantifiers";
  AVA6_API_CHECK(
      d_slv->getUserLogicInfo().isTheoryEnabled(internal::theory::THEORY_UF))
      << "recursive function definitions require a logic with uninterpreted "
         "functions";
  AVA6_API_SOLVER_CHECK_TERMS(funs);
  AVA6_API_SOLVER_CHECK_TERMS(terms);

  size_t funs_size = funs.size();
  AVA6_API_ARG_SIZE_CHECK_EXPECTED(funs_size == bound_vars.size(), bound_vars)
      << "'" << funs_size << "'";
  AVA6_API_ARG_SIZE_CHECK_EXPECTED(funs_size == terms.size(), terms)
      << "'" << funs_size << "'";

  for (size_t j = 0; j < funs_size; ++j)
  {
    const Term& fun = funs[j];
    const std::vector<Term>& bvars = bound_vars[j];
    const Term& term = terms[j];

    AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(
        d_tm.d_nm == fun.d_nm, "function", funs, j)
        << "function associated with the node manager of this solver object";
    AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(
        d_tm.d_nm == term.d_nm, "term", terms, j)
        << "term associated with the node manager of this solver object";

    if (fun.getSort().isFunction())
    {
      std::vector<Sort> domain_sorts = fun.getSort().getFunctionDomainSorts();
      AVA6_API_SOLVER_CHECK_BOUND_VARS(bvars);
      AVA6_API_SOLVER_CHECK_BOUND_VARS_DEF_FUN_SORTS(bvars, domain_sorts);
      Sort codomain = fun.getSort().getFunctionCodomainSort();
      AVA6_API_ARG_AT_INDEX_CHECK_EXPECTED(
          codomain == term.getSort(), "sort of function body", terms, j)
          << "'" << codomain << "'";
    }
    else
    {
      AVA6_API_SOLVER_CHECK_BOUND_VARS(bvars);
      AVA6_API_ARG_CHECK_EXPECTED(bvars.size() == 0, fun)
          << "function or nullary symbol";
    }
  }
  //////// all checks before this line
  std::vector<internal::Node> efuns = Term::termVectorToNodes(funs);
  std::vector<std::vector<internal::Node>> ebound_vars;
  for (const auto& v : bound_vars)
  {
    ebound_vars.push_back(Term::termVectorToNodes(v));
  }
  std::vector<internal::Node> nodes = Term::termVectorToNodes(terms);
  d_slv->defineFunctionsRec(efuns, ebound_vars, nodes, global);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Term> Solver::getAssertions(void) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  std::vector<internal::Node> assertions = d_slv->getAssertions();
  /* Can not use
   *   return std::vector<Term>(assertions.begin(), assertions.end());
   * here since constructor is private */
  return Term::nodeVectorToTerms(d_tm.d_nm, assertions);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Solver::getInfo(const std::string& flag) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_UNSUPPORTED_CHECK(d_slv->isValidGetInfoFlag(flag))
      << "unrecognized flag: " << flag << ".";
  //////// all checks before this line
  return d_slv->getInfo(flag);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Solver::getOption(const std::string& option) const
{
  try
  {
    return d_slv->getOption(option);
  }
  catch (internal::OptionException& e)
  {
    throw Ava6ApiUnsupportedException(e.getMessage());
  }
}

// Supports a visitor from a list of lambdas
// Taken from https://en.cppreference.com/w/cpp/utility/variant/visit
template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

OptionInfo& OptionInfo::operator=(OptionInfo&& info)
{
  name = std::move(info.name);
  aliases = std::move(info.aliases);
  noSupports = std::move(info.noSupports);
  setByUser = std::move(info.setByUser);
  category = std::move(info.category);
  valueInfo = std::move(info.valueInfo);
  return *this;
}

bool OptionInfo::boolValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(std::holds_alternative<ValueInfo<bool>>(valueInfo))
      << name << " is not a bool option";
  //////// all checks before this line
  return std::get<ValueInfo<bool>>(valueInfo).currentValue;
  ////////
  AVA6_API_TRY_CATCH_END;
}
std::string OptionInfo::stringValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(
      std::holds_alternative<ValueInfo<std::string>>(valueInfo))
      << name << " is not a string option";
  //////// all checks before this line
  return std::get<ValueInfo<std::string>>(valueInfo).currentValue;
  ////////
  AVA6_API_TRY_CATCH_END;
}
int64_t OptionInfo::intValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(
      std::holds_alternative<NumberInfo<int64_t>>(valueInfo))
      << name << " is not an int option";
  //////// all checks before this line
  return std::get<NumberInfo<int64_t>>(valueInfo).currentValue;
  ////////
  AVA6_API_TRY_CATCH_END;
}
uint64_t OptionInfo::uintValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(
      std::holds_alternative<NumberInfo<uint64_t>>(valueInfo))
      << name << " is not a uint option";
  //////// all checks before this line
  return std::get<NumberInfo<uint64_t>>(valueInfo).currentValue;
  ////////
  AVA6_API_TRY_CATCH_END;
}
double OptionInfo::doubleValue() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(
      std::holds_alternative<NumberInfo<double>>(valueInfo))
      << name << " is not a double option";
  //////// all checks before this line
  return std::get<NumberInfo<double>>(valueInfo).currentValue;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string OptionInfo::toString() const
{
  std::stringstream os;
  os << "OptionInfo{ " << name;
  if (setByUser)
  {
    os << " | set by user";
  }
  if (!aliases.empty())
  {
    internal::container_to_stream(os, aliases, ", ", "", ", ");
  }
  if (!noSupports.empty())
  {
    internal::container_to_stream(os, noSupports, ", ", "", ", ");
  }
  auto printNum = [&os](const std::string& type, const auto& vi) {
    os << " | " << type << " | " << vi.currentValue << " | default "
       << vi.defaultValue;
    if (vi.minimum || vi.maximum)
    {
      os << " |";
      if (vi.minimum)
      {
        os << " " << *vi.minimum << " <=";
      }
      os << " x";
      if (vi.maximum)
      {
        os << " <= " << *vi.maximum;
      }
    }
  };
  std::visit(overloaded{
                 [&os](const OptionInfo::VoidInfo&) { os << " | void"; },
                 [&os](const OptionInfo::ValueInfo<bool>& vi) {
                   os << std::boolalpha << " | bool | " << vi.currentValue
                      << " | default " << vi.defaultValue << std::noboolalpha;
                 },
                 [&os](const OptionInfo::ValueInfo<std::string>& vi) {
                   os << " | string | \"" << vi.currentValue
                      << "\" | default \"" << vi.defaultValue << "\"";
                 },
                 [&printNum](const OptionInfo::NumberInfo<int64_t>& vi) {
                   printNum("int64_t", vi);
                 },
                 [&printNum](const OptionInfo::NumberInfo<uint64_t>& vi) {
                   printNum("uint64_t", vi);
                 },
                 [&printNum](const OptionInfo::NumberInfo<double>& vi) {
                   printNum("double", vi);
                 },
                 [&os](const OptionInfo::ModeInfo& vi) {
                   os << " | mode | " << vi.currentValue << " | default "
                      << vi.defaultValue << " | modes: ";
                   internal::container_to_stream(os, vi.modes, "", "", ", ");
                 },
             },
             valueInfo);
  os << " }";
  return os.str();
}

std::ostream& operator<<(std::ostream& os, const OptionInfo& oi)
{
  os << oi.toString();
  return os;
}

std::vector<std::string> Solver::getOptionNames() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return internal::options::getNames();
  ////////
  AVA6_API_TRY_CATCH_END;
}

// Helper function to convert internal category to external enum
modes::OptionCategory convertOptionCategory(
    internal::options::OptionInfo::Category internalCategory)
{
  switch (internalCategory)
  {
    case internal::options::OptionInfo::Category::REGULAR:
      return modes::OptionCategory::REGULAR;
    case internal::options::OptionInfo::Category::COMMON:
      return modes::OptionCategory::COMMON;
    default:
      Assert(internalCategory
             == internal::options::OptionInfo::Category::UNDOCUMENTED);
      return modes::OptionCategory::UNDOCUMENTED;
  }
}

OptionInfo Solver::getOptionInfo(const std::string& option) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  auto info = internal::options::getInfo(d_slv->getOptions(), option);
  AVA6_API_CHECK(info.name != "")
      << "Querying invalid or unknown option " << option;
  return std::visit(
      overloaded{
          [&info](const internal::options::OptionInfo::VoidInfo&) {
            return OptionInfo{info.name,
                              info.aliases,
                              info.noSupports,
                              info.setByUser,
                              
                              
                              convertOptionCategory(info.category),
                              OptionInfo::VoidInfo{}};
          },
          [&info](const internal::options::OptionInfo::ValueInfo<bool>& vi) {
            return OptionInfo{
                info.name,
                info.aliases,
                info.noSupports,
                info.setByUser,
                
                
                convertOptionCategory(info.category),
                OptionInfo::ValueInfo<bool>{vi.defaultValue, vi.currentValue}};
          },
          [&info](
              const internal::options::OptionInfo::ValueInfo<std::string>& vi) {
            return OptionInfo{info.name,
                              info.aliases,
                              info.noSupports,
                              info.setByUser,
                              
                              
                              convertOptionCategory(info.category),
                              OptionInfo::ValueInfo<std::string>{
                                  vi.defaultValue, vi.currentValue}};
          },
          [&info](
              const internal::options::OptionInfo::NumberInfo<int64_t>& vi) {
            return OptionInfo{
                info.name,
                info.aliases,
                info.noSupports,
                info.setByUser,
                
                
                convertOptionCategory(info.category),
                OptionInfo::NumberInfo<int64_t>{
                    vi.defaultValue, vi.currentValue, vi.minimum, vi.maximum}};
          },
          [&info](
              const internal::options::OptionInfo::NumberInfo<uint64_t>& vi) {
            return OptionInfo{
                info.name,
                info.aliases,
                info.noSupports,
                info.setByUser,
                
                
                convertOptionCategory(info.category),
                OptionInfo::NumberInfo<uint64_t>{
                    vi.defaultValue, vi.currentValue, vi.minimum, vi.maximum}};
          },
          [&info](const internal::options::OptionInfo::NumberInfo<double>& vi) {
            return OptionInfo{
                info.name,
                info.aliases,
                info.noSupports,
                info.setByUser,
                
                
                convertOptionCategory(info.category),
                OptionInfo::NumberInfo<double>{
                    vi.defaultValue, vi.currentValue, vi.minimum, vi.maximum}};
          },
          [&info](const internal::options::OptionInfo::ModeInfo& vi) {
            return OptionInfo{info.name,
                              info.aliases,
                              info.noSupports,
                              info.setByUser,
                              
                              
                              convertOptionCategory(info.category),
                              OptionInfo::ModeInfo{
                                  vi.defaultValue, vi.currentValue, vi.modes}};
          },
      },
      info.valueInfo);
  ////////
  AVA6_API_TRY_CATCH_END;
}

DriverOptions Solver::getDriverOptions() const { return DriverOptions(*this); }

std::vector<Term> Solver::getUnsatAssumptions(void) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(d_slv->getOptions().smt.unsatAssumptions)
      << "cannot get unsat assumptions unless explicitly enabled "
         "(try --"
      << internal::options::smt::longName::unsatAssumptions << ")";
  AVA6_API_CHECK(d_slv->getSmtMode() == internal::SmtMode::UNSAT)
      << "cannot get unsat assumptions unless in unsat mode.";
  //////// all checks before this line

  std::vector<internal::Node> uassumptions = d_slv->getUnsatAssumptions();
  /* Cannot use
   *   return std::vector<Term>(uassumptions.begin(), uassumptions.end());
   * here since constructor is private */
  std::vector<Term> res;
  for (const internal::Node& n : uassumptions)
  {
    res.push_back(Term(d_tm.d_nm, n));
  }
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Term> Solver::getUnsatCore(void) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(d_slv->getOptions().smt.produceUnsatCores)
      << "cannot get unsat core unless explicitly enabled "
         "(try --"
      << internal::options::smt::longName::produceUnsatCores << ")";
  AVA6_API_RECOVERABLE_CHECK(d_slv->getSmtMode() == internal::SmtMode::UNSAT)
      << "cannot get unsat core unless in unsat mode.";
  //////// all checks before this line
  internal::UnsatCore core = d_slv->getUnsatCore();
  /* Can not use
   *   return std::vector<Term>(core.begin(), core.end());
   * here since constructor is private */
  std::vector<Term> res;
  for (const internal::Node& e : core)
  {
    res.push_back(Term(d_tm.d_nm, e));
  }
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Term> Solver::getUnsatCoreLemmas(void) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(d_slv->getOptions().smt.produceUnsatCores)
      << "cannot get unsat core lemmas unless explicitly enabled "
         "(try --"
      << internal::options::smt::longName::produceUnsatCores << ")";
  AVA6_API_CHECK(d_slv->getOptions().solver.unsatCoresMode
                 == internal::options::UnsatCoresMode::SAT_PROOF)
      << "cannot get unsat core lemmas unless SAT proofs are enabled";
  AVA6_API_RECOVERABLE_CHECK(d_slv->getSmtMode() == internal::SmtMode::UNSAT)
      << "cannot get unsat core unless in unsat mode.";
  //////// all checks before this line
  std::vector<internal::Node> lemmas = d_slv->getUnsatCoreLemmas();
  /* Can not use
   *   return std::vector<Term>(assertions.begin(), assertions.end());
   * here since constructor is private */
  return Term::nodeVectorToTerms(d_tm.d_nm, lemmas);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Proof> Solver::getProof(modes::ProofComponent c) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(d_slv->getOptions().smt.produceProofs)
      << "cannot get proof unless proofs are enabled (try --"
      << internal::options::smt::longName::produceProofs << ")";
  AVA6_API_RECOVERABLE_CHECK(d_slv->getSmtMode() == internal::SmtMode::UNSAT)
      << "cannot get proof unless in unsat mode.";
  //////// all checks before this line
  std::vector<std::shared_ptr<internal::ProofNode>> proof_nodes =
      d_slv->getProof(c);
  std::vector<Proof> proofs;
  for (auto& p : proof_nodes)
  {
    proofs.push_back(Proof(d_tm.d_nm, p));
  }
  return proofs;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Solver::proofToString(
    Proof proof,
    modes::ProofFormat format,
    const std::map<ava6::Term, std::string>& assertionNames) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  std::ostringstream ss;
  // convert the map's domain to use nodes rather than terms
  std::map<internal::Node, std::string> nodeAssertionNames;
  for (const auto& p : assertionNames)
  {
    nodeAssertionNames[p.first.getNode()] = p.second;
  }
  this->d_slv->printProof(ss, proof.d_proofNode, format, nodeAssertionNames);
  return ss.str();
  ////////
  AVA6_API_TRY_CATCH_END;
}

Term Solver::getValueHelper(const Term& term) const
{
  // Note: Term is checked in the caller to avoid double checks
  bool wasShadow = false;
  bool freeOrShadowedVar =
      internal::expr::hasFreeOrShadowedVar(term.getNode(), wasShadow);
  AVA6_API_RECOVERABLE_CHECK(!freeOrShadowedVar)
      << "cannot get value of term containing "
      << (wasShadow ? "shadowed" : "free") << " variables";
  //////// all checks before this line
  internal::Node value = d_slv->getValue(*term.d_node, true);
  Term res = Term(d_tm.d_nm, value);
  AssertEqual(res.getSort(), term.getSort());
  return res;
}

Term Solver::getValue(const Term& term) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(d_slv->getOptions().smt.produceModels)
      << "cannot get value unless model generation is enabled "
         "(try --"
      << internal::options::smt::longName::produceModels << ")";
  AVA6_API_RECOVERABLE_CHECK(d_slv->isSmtModeSat())
      << "cannot get value unless after a SAT or UNKNOWN response.";
  AVA6_API_SOLVER_CHECK_TERM(term);
  AVA6_API_RECOVERABLE_CHECK(
      d_slv->getEnv().isFirstClassType(term.getSort().getTypeNode()))
      << "cannot get value of a term that is not first class.";
  AVA6_API_RECOVERABLE_CHECK(!term.getSort().isDatatype()
                             || term.getSort().getDatatype().isWellFounded())
      << "cannot get value of a term of non-well-founded datatype sort.";
  ensureWellFormedTerm(term);
  //////// all checks before this line
  return getValueHelper(term);
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Term> Solver::getValue(const std::vector<Term>& terms) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(d_slv->getOptions().smt.produceModels)
      << "cannot get value unless model generation is enabled "
         "(try --"
      << internal::options::smt::longName::produceModels << ")";
  AVA6_API_RECOVERABLE_CHECK(d_slv->isSmtModeSat())
      << "cannot get value unless after a SAT or UNKNOWN response.";
  for (const Term& t : terms)
  {
    AVA6_API_RECOVERABLE_CHECK(
        d_slv->getEnv().isFirstClassType(t.getSort().getTypeNode()))
        << "cannot get value of a term that is not first class.";
    AVA6_API_RECOVERABLE_CHECK(!t.getSort().isDatatype()
                               || t.getSort().getDatatype().isWellFounded())
        << "cannot get value of a term of non-well-founded datatype sort.";
  }
  AVA6_API_SOLVER_CHECK_TERMS(terms);
  ensureWellFormedTerms(terms);
  //////// all checks before this line

  std::vector<Term> res;
  for (size_t i = 0, n = terms.size(); i < n; ++i)
  {
    /* Can not use emplace_back here since constructor is private. */
    res.push_back(getValueHelper(terms[i]));
  }
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::vector<Term> Solver::getModelDomainElements(const Sort& s) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(d_slv->getOptions().smt.produceModels)
      << "cannot get domain elements unless model generation is enabled "
         "(try --"
      << internal::options::smt::longName::produceModels << ")";
  AVA6_API_RECOVERABLE_CHECK(d_slv->isSmtModeSat())
      << "cannot get domain elements unless after a SAT or UNKNOWN response.";
  AVA6_API_SOLVER_CHECK_SORT(s);
  AVA6_API_RECOVERABLE_CHECK(s.isUninterpretedSort())
      << "expected an uninterpreted sort as argument to "
         "getModelDomainElements.";
  //////// all checks before this line
  std::vector<Term> res;
  std::vector<internal::Node> elements =
      d_slv->getModelDomainElements(s.getTypeNode());
  for (const internal::Node& n : elements)
  {
    res.push_back(Term(d_tm.d_nm, n));
  }
  return res;
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Solver::getModel(const std::vector<Sort>& sorts,
                             const std::vector<Term>& vars) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(d_slv->getOptions().smt.produceModels)
      << "cannot get model unless model generation is enabled "
         "(try --"
      << internal::options::smt::longName::produceModels << ")";
  AVA6_API_RECOVERABLE_CHECK(d_slv->isSmtModeSat())
      << "cannot get model unless after a SAT or UNKNOWN response.";
  AVA6_API_SOLVER_CHECK_SORTS(sorts);
  for (const Sort& s : sorts)
  {
    AVA6_API_RECOVERABLE_CHECK(s.isUninterpretedSort())
        << "expected an uninterpreted sort as argument to "
           "getModel.";
  }
  AVA6_API_SOLVER_CHECK_TERMS(vars);
  for (const Term& v : vars)
  {
    AVA6_API_RECOVERABLE_CHECK(v.getKind() == Kind::CONSTANT)
        << "expected a free constant as argument to getModel.";
  }
  //////// all checks before this line
  return d_slv->getModel(Sort::sortVectorToTypeNodes(sorts),
                         Term::termVectorToNodes(vars));
  ////////
  AVA6_API_TRY_CATCH_END;
}

void Solver::addPlugin(Plugin& p)
{
  AVA6_API_TRY_CATCH_BEGIN;
  d_slv->addPlugin(p.d_pExtToInt.get());
  AVA6_API_TRY_CATCH_END;
}

void Solver::pop(uint32_t nscopes) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(d_slv->getOptions().base.incrementalSolving)
      << "cannot pop when not solving incrementally (use --"
      << internal::options::base::longName::incrementalSolving << ")";
  AVA6_API_CHECK(nscopes <= d_slv->getNumUserLevels())
      << "cannot pop beyond first pushed context";
  //////// all checks before this line
  for (uint32_t n = 0; n < nscopes; ++n)
  {
    d_slv->pop();
  }
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Solver::getInstantiations() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_RECOVERABLE_CHECK(d_slv->getSmtMode() == internal::SmtMode::UNSAT
                             || d_slv->getSmtMode() == internal::SmtMode::SAT
                             || d_slv->getSmtMode()
                                    == internal::SmtMode::SAT_UNKNOWN)
      << "cannot get instantiations unless after a UNSAT, SAT or UNKNOWN "
         "response.";
  //////// all checks before this line
  std::stringstream ss;
  d_slv->printInstantiations(ss);
  return ss.str();
  ////////
  AVA6_API_TRY_CATCH_END;
}

void Solver::push(uint32_t nscopes) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(d_slv->getOptions().base.incrementalSolving)
      << "cannot push when not solving incrementally (use --"
      << internal::options::base::longName::incrementalSolving << ")";
  //////// all checks before this line
  for (uint32_t n = 0; n < nscopes; ++n)
  {
    d_slv->push();
  }
  ////////
  AVA6_API_TRY_CATCH_END;
}

void Solver::resetAssertions(void) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  d_slv->resetAssertions();
  ////////
  AVA6_API_TRY_CATCH_END;
}

void Solver::setInfo(const std::string& keyword, const std::string& value) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_UNSUPPORTED_CHECK(
      keyword == "source" || keyword == "category" || keyword == "difficulty"
      || keyword == "filename" || keyword == "license" || keyword == "name"
      || keyword == "notes" || keyword == "smt-lib-version"
      || keyword == "status")
      << "unrecognized keyword: " << keyword
      << ", expected 'source', 'category', 'difficulty', "
         "'filename', 'license', 'name', "
         "'notes', 'smt-lib-version' or 'status'";
  AVA6_API_RECOVERABLE_ARG_CHECK_EXPECTED(
      keyword != "smt-lib-version" || value == "2" || value == "2.0"
          || value == "2.5" || value == "2.6" || value == "2.7",
      value)
      << "'2.0', '2.5', '2.6', '2.7'";
  AVA6_API_ARG_CHECK_EXPECTED(keyword != "status" || value == "sat"
                                  || value == "unsat" || value == "unknown",
                              value)
      << "'sat', 'unsat' or 'unknown'";
  //////// all checks before this line
  if (keyword == "filename")
  {
    // only the Solver object has non-const access to the original options
    d_originalOptions->write_driver().filename = value;
  }
  d_slv->setInfo(keyword, value);
  ////////
  AVA6_API_TRY_CATCH_END;
}

void Solver::setLogic(const std::string& logic) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(!d_slv->isLogicSet())
      << "invalid call to 'setLogic', logic is already set";
  AVA6_API_CHECK(!d_slv->isFullyInited())
      << "invalid call to 'setLogic', solver is already fully initialized";
  //////// all checks before this line
  internal::LogicInfo linfo(logic);
  d_slv->setLogic(linfo);
  ////////
  AVA6_API_TRY_CATCH_END;
}

bool Solver::isLogicSet() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  //////// all checks before this line
  return d_slv->isLogicSet();
  ////////
  AVA6_API_TRY_CATCH_END;
}

std::string Solver::getLogic() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  AVA6_API_CHECK(d_slv->isLogicSet())
      << "invalid call to 'getLogic', logic has not yet been set";
  //////// all checks before this line
  return d_slv->getUserLogicInfo().getLogicString();
  ////////
  AVA6_API_TRY_CATCH_END;
}

void Solver::setOption(const std::string& option,
                       const std::string& value) const
{
  AVA6_API_TRY_CATCH_BEGIN;
  std::vector<std::string> options = internal::options::getNames();
  AVA6_API_UNSUPPORTED_CHECK(
      option.find("command-verbosity") != std::string::npos
      || std::find(options.cbegin(), options.cend(), option) != options.cend())
      << "unrecognized option: " << option << '.';
  // this list includes options that are prescribed to be changable in any
  // context based on the SMT-LIB standard, as well as options (e.g. tlimit-per)
  // that have no impact on solver initialization or imply other options.
  static constexpr auto mutableOpts = {"diagnostic-output-channel",
                                       "print-success",
                                       "regular-output-channel",
                                       "reproducible-resource-limit",
                                       "verbosity",
                                       "tlimit-per"};
  if (std::find(mutableOpts.begin(), mutableOpts.end(), option)
      == mutableOpts.end())
  {
    AVA6_API_CHECK(!d_slv->isFullyInited())
        << "invalid call to 'setOption' for option '" << option
        << "', solver is already fully initialized";
  }
  //////// all checks before this line
  // mark that the option originated from the user here
  d_slv->setOption(option, value);
  ////////
  AVA6_API_TRY_CATCH_END;
}

Statistics Solver::getStatistics() const
{
  return Statistics(d_slv->getStatisticsRegistry());
}

bool Solver::isOutputOn(const std::string& tag) const
{
  // `isOutputOn(tag)` may raise an `OptionException`, which we do not want to
  // forward as such. We thus do not use the standard exception handling macros
  // here but roll our own.
  try
  {
    return d_slv->getEnv().isOutputOn(tag);
  }
  catch (const internal::Exception& e)
  {
    throw Ava6ApiException("invalid output tag " + tag);
  }
}

std::ostream& Solver::getOutput(const std::string& tag) const
{
  // `output(tag)` may raise an `OptionException`, which we do not want to
  // forward as such. We thus do not use the standard exception handling macros
  // here but roll our own.
  try
  {
    return d_slv->getEnv().output(tag);
  }
  catch (const internal::Exception& e)
  {
    throw Ava6ApiException("invalid output tag " + tag);
  }
}

std::string Solver::getVersion() const
{
  AVA6_API_TRY_CATCH_BEGIN;
  return internal::Configuration::getVersionString();
  AVA6_API_TRY_CATCH_END;
}

TermManager& Solver::getTermManager() const { return d_tm; }

}  // namespace ava6

namespace std {

size_t hash<ava6::Kind>::operator()(ava6::Kind k) const
{
  return static_cast<size_t>(k);
}

std::string to_string(ava6::Kind k)
{
  switch (k)
  {
    case ava6::Kind::INTERNAL_KIND: return "INTERNAL_KIND";
    case ava6::Kind::UNDEFINED_KIND: return "UNDEFINED_KIND";
    case ava6::Kind::NULL_TERM: return "NULL_TERM";
    case ava6::Kind::UNINTERPRETED_SORT_VALUE: return "UNINTERPRETED_SORT_VALUE";
    case ava6::Kind::EQUAL: return "EQUAL";
    case ava6::Kind::DISTINCT: return "DISTINCT";
    case ava6::Kind::CONSTANT: return "CONSTANT";
    case ava6::Kind::VARIABLE: return "VARIABLE";
    case ava6::Kind::SKOLEM: return "SKOLEM";
    case ava6::Kind::SEXPR: return "SEXPR";
    case ava6::Kind::LAMBDA: return "LAMBDA";
    case ava6::Kind::WITNESS: return "WITNESS";
    case ava6::Kind::CONST_BOOLEAN: return "CONST_BOOLEAN";
    case ava6::Kind::NOT: return "NOT";
    case ava6::Kind::AND: return "AND";
    case ava6::Kind::IMPLIES: return "IMPLIES";
    case ava6::Kind::OR: return "OR";
    case ava6::Kind::XOR: return "XOR";
    case ava6::Kind::ITE: return "ITE";
    case ava6::Kind::APPLY_UF: return "APPLY_UF";
    case ava6::Kind::ADD: return "ADD";
    case ava6::Kind::MULT: return "MULT";
    case ava6::Kind::IAND: return "IAND";
    case ava6::Kind::PIAND: return "PIAND";
    case ava6::Kind::POW2: return "POW2";
    case ava6::Kind::LOG2: return "LOG2";
    case ava6::Kind::SUB: return "SUB";
    case ava6::Kind::NEG: return "NEG";
    case ava6::Kind::DIVISION: return "DIVISION";
    case ava6::Kind::DIVISION_TOTAL: return "DIVISION_TOTAL";
    case ava6::Kind::INTS_DIVISION: return "INTS_DIVISION";
    case ava6::Kind::INTS_DIVISION_TOTAL: return "INTS_DIVISION_TOTAL";
    case ava6::Kind::INTS_MODULUS: return "INTS_MODULUS";
    case ava6::Kind::INTS_MODULUS_TOTAL: return "INTS_MODULUS_TOTAL";
    case ava6::Kind::ABS: return "ABS";
    case ava6::Kind::POW: return "POW";
    case ava6::Kind::EXPONENTIAL: return "EXPONENTIAL";
    case ava6::Kind::SINE: return "SINE";
    case ava6::Kind::COSINE: return "COSINE";
    case ava6::Kind::TANGENT: return "TANGENT";
    case ava6::Kind::COSECANT: return "COSECANT";
    case ava6::Kind::SECANT: return "SECANT";
    case ava6::Kind::COTANGENT: return "COTANGENT";
    case ava6::Kind::ARCSINE: return "ARCSINE";
    case ava6::Kind::ARCCOSINE: return "ARCCOSINE";
    case ava6::Kind::ARCTANGENT: return "ARCTANGENT";
    case ava6::Kind::ARCCOSECANT: return "ARCCOSECANT";
    case ava6::Kind::ARCSECANT: return "ARCSECANT";
    case ava6::Kind::ARCCOTANGENT: return "ARCCOTANGENT";
    case ava6::Kind::SQRT: return "SQRT";
    case ava6::Kind::DIVISIBLE: return "DIVISIBLE";
    case ava6::Kind::CONST_RATIONAL: return "CONST_RATIONAL";
    case ava6::Kind::CONST_INTEGER: return "CONST_INTEGER";
    case ava6::Kind::LT: return "LT";
    case ava6::Kind::LEQ: return "LEQ";
    case ava6::Kind::GT: return "GT";
    case ava6::Kind::GEQ: return "GEQ";
    case ava6::Kind::IS_INTEGER: return "IS_INTEGER";
    case ava6::Kind::TO_INTEGER: return "TO_INTEGER";
    case ava6::Kind::TO_REAL: return "TO_REAL";
    case ava6::Kind::PI: return "PI";
    case ava6::Kind::CONST_BITVECTOR: return "CONST_BITVECTOR";
    case ava6::Kind::BITVECTOR_CONCAT: return "BITVECTOR_CONCAT";
    case ava6::Kind::BITVECTOR_AND: return "BITVECTOR_AND";
    case ava6::Kind::BITVECTOR_OR: return "BITVECTOR_OR";
    case ava6::Kind::BITVECTOR_XOR: return "BITVECTOR_XOR";
    case ava6::Kind::BITVECTOR_NOT: return "BITVECTOR_NOT";
    case ava6::Kind::BITVECTOR_NAND: return "BITVECTOR_NAND";
    case ava6::Kind::BITVECTOR_NOR: return "BITVECTOR_NOR";
    case ava6::Kind::BITVECTOR_XNOR: return "BITVECTOR_XNOR";
    case ava6::Kind::BITVECTOR_COMP: return "BITVECTOR_COMP";
    case ava6::Kind::BITVECTOR_MULT: return "BITVECTOR_MULT";
    case ava6::Kind::BITVECTOR_ADD: return "BITVECTOR_ADD";
    case ava6::Kind::BITVECTOR_SUB: return "BITVECTOR_SUB";
    case ava6::Kind::BITVECTOR_NEG: return "BITVECTOR_NEG";
    case ava6::Kind::BITVECTOR_UDIV: return "BITVECTOR_UDIV";
    case ava6::Kind::BITVECTOR_UREM: return "BITVECTOR_UREM";
    case ava6::Kind::BITVECTOR_SDIV: return "BITVECTOR_SDIV";
    case ava6::Kind::BITVECTOR_SREM: return "BITVECTOR_SREM";
    case ava6::Kind::BITVECTOR_SMOD: return "BITVECTOR_SMOD";
    case ava6::Kind::BITVECTOR_SHL: return "BITVECTOR_SHL";
    case ava6::Kind::BITVECTOR_LSHR: return "BITVECTOR_LSHR";
    case ava6::Kind::BITVECTOR_ASHR: return "BITVECTOR_ASHR";
    case ava6::Kind::BITVECTOR_ULT: return "BITVECTOR_ULT";
    case ava6::Kind::BITVECTOR_ULE: return "BITVECTOR_ULE";
    case ava6::Kind::BITVECTOR_UGT: return "BITVECTOR_UGT";
    case ava6::Kind::BITVECTOR_UGE: return "BITVECTOR_UGE";
    case ava6::Kind::BITVECTOR_SLT: return "BITVECTOR_SLT";
    case ava6::Kind::BITVECTOR_SLE: return "BITVECTOR_SLE";
    case ava6::Kind::BITVECTOR_SGT: return "BITVECTOR_SGT";
    case ava6::Kind::BITVECTOR_SGE: return "BITVECTOR_SGE";
    case ava6::Kind::BITVECTOR_ULTBV: return "BITVECTOR_ULTBV";
    case ava6::Kind::BITVECTOR_SLTBV: return "BITVECTOR_SLTBV";
    case ava6::Kind::BITVECTOR_ITE: return "BITVECTOR_ITE";
    case ava6::Kind::BITVECTOR_REDOR: return "BITVECTOR_REDOR";
    case ava6::Kind::BITVECTOR_REDAND: return "BITVECTOR_REDAND";
    case ava6::Kind::BITVECTOR_NEGO: return "BITVECTOR_NEGO";
    case ava6::Kind::BITVECTOR_UADDO: return "BITVECTOR_UADDO";
    case ava6::Kind::BITVECTOR_SADDO: return "BITVECTOR_SADDO";
    case ava6::Kind::BITVECTOR_UMULO: return "BITVECTOR_UMULO";
    case ava6::Kind::BITVECTOR_SMULO: return "BITVECTOR_SMULO";
    case ava6::Kind::BITVECTOR_USUBO: return "BITVECTOR_USUBO";
    case ava6::Kind::BITVECTOR_SSUBO: return "BITVECTOR_SSUBO";
    case ava6::Kind::BITVECTOR_SDIVO: return "BITVECTOR_SDIVO";
    case ava6::Kind::BITVECTOR_EXTRACT: return "BITVECTOR_EXTRACT";
    case ava6::Kind::BITVECTOR_REPEAT: return "BITVECTOR_REPEAT";
    case ava6::Kind::BITVECTOR_ZERO_EXTEND: return "BITVECTOR_ZERO_EXTEND";
    case ava6::Kind::BITVECTOR_SIGN_EXTEND: return "BITVECTOR_SIGN_EXTEND";
    case ava6::Kind::BITVECTOR_ROTATE_LEFT: return "BITVECTOR_ROTATE_LEFT";
    case ava6::Kind::BITVECTOR_ROTATE_RIGHT: return "BITVECTOR_ROTATE_RIGHT";
    case ava6::Kind::INT_TO_BITVECTOR: return "INT_TO_BITVECTOR";
    case ava6::Kind::BITVECTOR_TO_NAT: return "BITVECTOR_TO_NAT";
    case ava6::Kind::BITVECTOR_UBV_TO_INT: return "BITVECTOR_UBV_TO_INT";
    case ava6::Kind::BITVECTOR_SBV_TO_INT: return "BITVECTOR_SBV_TO_INT";
    case ava6::Kind::BITVECTOR_FROM_BOOLS: return "BITVECTOR_FROM_BOOLS";
    case ava6::Kind::BITVECTOR_BIT: return "BITVECTOR_BIT";
    case ava6::Kind::SELECT: return "SELECT";
    case ava6::Kind::STORE: return "STORE";
    case ava6::Kind::CONST_ARRAY: return "CONST_ARRAY";
    case ava6::Kind::EQ_RANGE: return "EQ_RANGE";
    case ava6::Kind::APPLY_CONSTRUCTOR: return "APPLY_CONSTRUCTOR";
    case ava6::Kind::APPLY_SELECTOR: return "APPLY_SELECTOR";
    case ava6::Kind::APPLY_TESTER: return "APPLY_TESTER";
    case ava6::Kind::APPLY_UPDATER: return "APPLY_UPDATER";
    case ava6::Kind::MATCH: return "MATCH";
    case ava6::Kind::MATCH_CASE: return "MATCH_CASE";
    case ava6::Kind::MATCH_BIND_CASE: return "MATCH_BIND_CASE";
    case ava6::Kind::TUPLE_PROJECT: return "TUPLE_PROJECT";
    case ava6::Kind::SET_EMPTY: return "SET_EMPTY";
    case ava6::Kind::SET_UNION: return "SET_UNION";
    case ava6::Kind::SET_INTER: return "SET_INTER";
    case ava6::Kind::SET_MINUS: return "SET_MINUS";
    case ava6::Kind::SET_SUBSET: return "SET_SUBSET";
    case ava6::Kind::SET_MEMBER: return "SET_MEMBER";
    case ava6::Kind::SET_SINGLETON: return "SET_SINGLETON";
    case ava6::Kind::SET_INSERT: return "SET_INSERT";
    case ava6::Kind::SET_COMPREHENSION: return "SET_COMPREHENSION";
    case ava6::Kind::SET_CHOOSE: return "SET_CHOOSE";
    case ava6::Kind::SET_IS_EMPTY: return "SET_IS_EMPTY";
    case ava6::Kind::SET_IS_SINGLETON: return "SET_IS_SINGLETON";
    case ava6::Kind::STRING_CONCAT: return "STRING_CONCAT";
    case ava6::Kind::STRING_IN_REGEXP: return "STRING_IN_REGEXP";
    case ava6::Kind::STRING_LENGTH: return "STRING_LENGTH";
    case ava6::Kind::STRING_SUBSTR: return "STRING_SUBSTR";
    case ava6::Kind::STRING_UPDATE: return "STRING_UPDATE";
    case ava6::Kind::STRING_CHARAT: return "STRING_CHARAT";
    case ava6::Kind::STRING_CONTAINS: return "STRING_CONTAINS";
    case ava6::Kind::STRING_INDEXOF: return "STRING_INDEXOF";
    case ava6::Kind::STRING_INDEXOF_RE: return "STRING_INDEXOF_RE";
    case ava6::Kind::STRING_REPLACE: return "STRING_REPLACE";
    case ava6::Kind::STRING_REPLACE_ALL: return "STRING_REPLACE_ALL";
    case ava6::Kind::STRING_REPLACE_RE: return "STRING_REPLACE_RE";
    case ava6::Kind::STRING_REPLACE_RE_ALL: return "STRING_REPLACE_RE_ALL";
    case ava6::Kind::STRING_TO_LOWER: return "STRING_TO_LOWER";
    case ava6::Kind::STRING_TO_UPPER: return "STRING_TO_UPPER";
    case ava6::Kind::STRING_REV: return "STRING_REV";
    case ava6::Kind::STRING_TO_CODE: return "STRING_TO_CODE";
    case ava6::Kind::STRING_FROM_CODE: return "STRING_FROM_CODE";
    case ava6::Kind::STRING_LT: return "STRING_LT";
    case ava6::Kind::STRING_LEQ: return "STRING_LEQ";
    case ava6::Kind::STRING_PREFIX: return "STRING_PREFIX";
    case ava6::Kind::STRING_SUFFIX: return "STRING_SUFFIX";
    case ava6::Kind::STRING_IS_DIGIT: return "STRING_IS_DIGIT";
    case ava6::Kind::STRING_FROM_INT: return "STRING_FROM_INT";
    case ava6::Kind::STRING_TO_INT: return "STRING_TO_INT";
    case ava6::Kind::CONST_STRING: return "CONST_STRING";
    case ava6::Kind::STRING_TO_REGEXP: return "STRING_TO_REGEXP";
    case ava6::Kind::REGEXP_CONCAT: return "REGEXP_CONCAT";
    case ava6::Kind::REGEXP_UNION: return "REGEXP_UNION";
    case ava6::Kind::REGEXP_INTER: return "REGEXP_INTER";
    case ava6::Kind::REGEXP_DIFF: return "REGEXP_DIFF";
    case ava6::Kind::REGEXP_STAR: return "REGEXP_STAR";
    case ava6::Kind::REGEXP_PLUS: return "REGEXP_PLUS";
    case ava6::Kind::REGEXP_OPT: return "REGEXP_OPT";
    case ava6::Kind::REGEXP_RANGE: return "REGEXP_RANGE";
    case ava6::Kind::REGEXP_REPEAT: return "REGEXP_REPEAT";
    case ava6::Kind::REGEXP_LOOP: return "REGEXP_LOOP";
    case ava6::Kind::REGEXP_NONE: return "REGEXP_NONE";
    case ava6::Kind::REGEXP_ALL: return "REGEXP_ALL";
    case ava6::Kind::REGEXP_ALLCHAR: return "REGEXP_ALLCHAR";
    case ava6::Kind::REGEXP_COMPLEMENT: return "REGEXP_COMPLEMENT";
    case ava6::Kind::SEQ_CONCAT: return "SEQ_CONCAT";
    case ava6::Kind::SEQ_LENGTH: return "SEQ_LENGTH";
    case ava6::Kind::SEQ_EXTRACT: return "SEQ_EXTRACT";
    case ava6::Kind::SEQ_UPDATE: return "SEQ_UPDATE";
    case ava6::Kind::SEQ_AT: return "SEQ_AT";
    case ava6::Kind::SEQ_CONTAINS: return "SEQ_CONTAINS";
    case ava6::Kind::SEQ_INDEXOF: return "SEQ_INDEXOF";
    case ava6::Kind::SEQ_REPLACE: return "SEQ_REPLACE";
    case ava6::Kind::SEQ_REPLACE_ALL: return "SEQ_REPLACE_ALL";
    case ava6::Kind::SEQ_REV: return "SEQ_REV";
    case ava6::Kind::SEQ_PREFIX: return "SEQ_PREFIX";
    case ava6::Kind::SEQ_SUFFIX: return "SEQ_SUFFIX";
    case ava6::Kind::CONST_SEQUENCE: return "CONST_SEQUENCE";
    case ava6::Kind::SEQ_UNIT: return "SEQ_UNIT";
    case ava6::Kind::SEQ_NTH: return "SEQ_NTH";
    case ava6::Kind::FORALL: return "FORALL";
    case ava6::Kind::EXISTS: return "EXISTS";
    case ava6::Kind::VARIABLE_LIST: return "VARIABLE_LIST";
    case ava6::Kind::INST_PATTERN: return "INST_PATTERN";
    case ava6::Kind::INST_NO_PATTERN: return "INST_NO_PATTERN";
    case ava6::Kind::INST_ATTRIBUTE: return "INST_ATTRIBUTE";
    case ava6::Kind::INST_PATTERN_LIST: return "INST_PATTERN_LIST";
    case ava6::Kind::LAST_KIND: return "LAST_KIND";
    default: return "UNDEFINED_KIND";
  }
}

size_t hash<ava6::SortKind>::operator()(ava6::SortKind k) const
{
  return static_cast<size_t>(k);
}

std::string to_string(ava6::SortKind k)
{
  switch (k)
  {
    case ava6::SortKind::INTERNAL_SORT_KIND: return "INTERNAL_SORT_KIND";
    case ava6::SortKind::UNDEFINED_SORT_KIND: return "UNDEFINED_SORT_KIND";
    case ava6::SortKind::NULL_SORT: return "NULL_SORT";
    case ava6::SortKind::ABSTRACT_SORT: return "ABSTRACT_SORT";
    case ava6::SortKind::ARRAY_SORT: return "ARRAY_SORT";
    case ava6::SortKind::BOOLEAN_SORT: return "BOOLEAN_SORT";
    case ava6::SortKind::BITVECTOR_SORT: return "BITVECTOR_SORT";
    case ava6::SortKind::DATATYPE_SORT: return "DATATYPE_SORT";
    case ava6::SortKind::FUNCTION_SORT: return "FUNCTION_SORT";
    case ava6::SortKind::INTEGER_SORT: return "INTEGER_SORT";
    case ava6::SortKind::REAL_SORT: return "REAL_SORT";
    case ava6::SortKind::REGLAN_SORT: return "REGLAN_SORT";
    case ava6::SortKind::SEQUENCE_SORT: return "SEQUENCE_SORT";
    case ava6::SortKind::SET_SORT: return "SET_SORT";
    case ava6::SortKind::STRING_SORT: return "STRING_SORT";
    case ava6::SortKind::TUPLE_SORT: return "TUPLE_SORT";
    case ava6::SortKind::UNINTERPRETED_SORT: return "UNINTERPRETED_SORT";
    case ava6::SortKind::LAST_SORT_KIND: return "LAST_SORT_KIND";
    default: return "UNDEFINED_SORT_KIND";
  }
}

size_t hash<ava6::Op>::operator()(const ava6::Op& t) const
{
  if (t.isIndexedHelper())
  {
    return std::hash<ava6::internal::Node>()(*t.d_node);
  }
  else
  {
    return std::hash<ava6::Kind>()(t.d_kind);
  }
}

size_t std::hash<ava6::Sort>::operator()(const ava6::Sort& s) const
{
  return std::hash<ava6::internal::TypeNode>()(*s.d_type);
}

size_t std::hash<ava6::Term>::operator()(const ava6::Term& t) const
{
  return std::hash<ava6::internal::Node>()(*t.d_node);
}

size_t std::hash<ava6::DatatypeConstructorDecl>::operator()(
    const ava6::DatatypeConstructorDecl& decl) const
{
  if (decl.isNull())
  {
    return 0;
  }
  return std::hash<ava6::internal::DTypeConstructor>()(*decl.d_ctor);
}

size_t std::hash<ava6::DatatypeDecl>::operator()(
    const ava6::DatatypeDecl& decl) const
{
  if (decl.isNull())
  {
    return 0;
  }
  return std::hash<ava6::internal::DType>()(*decl.d_dtype);
}

size_t std::hash<ava6::DatatypeSelector>::operator()(
    const ava6::DatatypeSelector& sel) const
{
  if (sel.isNull())
  {
    return 0;
  }
  return std::hash<ava6::internal::DTypeSelector>()(*sel.d_stor);
}

size_t std::hash<ava6::DatatypeConstructor>::operator()(
    const ava6::DatatypeConstructor& cons) const
{
  if (cons.isNull())
  {
    return 0;
  }
  return std::hash<ava6::internal::DTypeConstructor>()(*cons.d_ctor);
}

size_t hash<ava6::Datatype>::operator()(const ava6::Datatype& dt) const
{
  if (dt.isNull())
  {
    return 0;
  }
  return std::hash<ava6::internal::DType>()(*dt.d_dtype);
}

size_t hash<ava6::Proof>::operator()(const ava6::Proof& proof) const
{
  if (proof.isNull())
  {
    return 0;
  }
  return std::hash<ava6::internal::ProofNode>{}(*proof.d_proofNode);
}

}  // namespace std
