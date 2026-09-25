/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of inference enumeration.
 */

#include "theory/inference_id.h"

#include <iostream>

#include "proof/proof_checker.h"
#include "util/rational.h"

using namespace ava6::internal::kind;

namespace ava6::internal {
namespace theory {

const char* toString(InferenceId i)
{
  switch (i)
  {
    case InferenceId::NONE: return "NONE";
    case InferenceId::INPUT: return "INPUT";
    case InferenceId::EQ_CONSTANT_MERGE: return "EQ_CONSTANT_MERGE";
    case InferenceId::COMBINATION_SPLIT: return "COMBINATION_SPLIT";
    case InferenceId::CONFLICT_REWRITE_LIT: return "CONFLICT_REWRITE_LIT";
    case InferenceId::THEORY_PP_SKOLEM_LEM: return "THEORY_PP_SKOLEM_LEM";
    case InferenceId::EXTT_SIMPLIFY: return "EXTT_SIMPLIFY";
    case InferenceId::ARITH_BLACK_BOX: return "ARITH_BLACK_BOX";
    case InferenceId::ARITH_CONF_EQ: return "ARITH_CONF_EQ";
    case InferenceId::ARITH_CONF_LOWER: return "ARITH_CONF_LOWER";
    case InferenceId::ARITH_CONF_TRICHOTOMY: return "ARITH_CONF_TRICHOTOMY";
    case InferenceId::ARITH_CONF_UPPER: return "ARITH_CONF_UPPER";
    case InferenceId::ARITH_CONF_SIMPLEX: return "ARITH_CONF_SIMPLEX";
    case InferenceId::ARITH_CONF_SOI_SIMPLEX: return "ARITH_CONF_SOI_SIMPLEX";
    case InferenceId::ARITH_CONF_FACT_QUEUE: return "ARITH_CONF_FACT_QUEUE";
    case InferenceId::ARITH_CONF_UNATE_PROP: return "ARITH_CONF_UNATE_PROP";
    case InferenceId::ARITH_SPLIT_DEQ: return "ARITH_SPLIT_DEQ";
    case InferenceId::ARITH_EQUIV_ATOM: return "ARITH_EQUIV_ATOM";
    case InferenceId::ARITH_TIGHTEN_CEIL: return "ARITH_TIGHTEN_CEIL";
    case InferenceId::ARITH_TIGHTEN_FLOOR: return "ARITH_TIGHTEN_FLOOR";
    case InferenceId::ARITH_BB_LEMMA: return "ARITH_BB_LEMMA";
    case InferenceId::ARITH_DIO_CUT: return "ARITH_DIO_CUT";
    case InferenceId::ARITH_DIO_DECOMPOSITION: return "ARITH_DIO_DECOMPOSITION";
    case InferenceId::ARITH_UNATE: return "ARITH_UNATE";
    case InferenceId::ARITH_ROW_IMPL: return "ARITH_ROW_IMPL";
    case InferenceId::ARITH_SPLIT_FOR_NL_MODEL:
      return "ARITH_SPLIT_FOR_NL_MODEL";
    case InferenceId::ARITH_DEMAND_RESTART: return "ARITH_DEMAND_RESTART";
    case InferenceId::ARITH_PP_ELIM_OPERATORS: return "ARITH_PP_ELIM_OPERATORS";
    case InferenceId::ARITH_PP_ELIM_OPERATORS_LEMMA:
      return "ARITH_PP_ELIM_OPERATORS_LEMMA";
    case InferenceId::ARITH_NL_SHARED_TERM_SPLIT:
      return "ARITH_NL_SHARED_TERM_SPLIT";
    case InferenceId::ARITH_NL_SHARED_TERM_FACTOR_SPLIT:
      return "ARITH_NL_SHARED_TERM_FACTOR_SPLIT";
    case InferenceId::ARITH_NL_SIGN: return "ARITH_NL_SIGN";
    case InferenceId::ARITH_NL_COMPARISON: return "ARITH_NL_COMPARISON";
    case InferenceId::ARITH_NL_INFER_BOUNDS_NT:
      return "ARITH_NL_INFER_BOUNDS_NT";
    case InferenceId::ARITH_NL_FACTOR: return "ARITH_NL_FACTOR";
    case InferenceId::ARITH_NL_TANGENT_PLANE: return "ARITH_NL_TANGENT_PLANE";
    case InferenceId::ARITH_NL_FLATTEN_MON: return "ARITH_NL_FLATTEN_MON";
    case InferenceId::ARRAYS_EXT: return "ARRAYS_EXT";
    case InferenceId::ARRAYS_READ_OVER_WRITE: return "ARRAYS_READ_OVER_WRITE";
    case InferenceId::ARRAYS_READ_OVER_WRITE_1:
      return "ARRAYS_READ_OVER_WRITE_1";
    case InferenceId::ARRAYS_READ_OVER_WRITE_CONTRA:
      return "ARRAYS_READ_OVER_WRITE_CONTRA";
    case InferenceId::ARRAYS_CONST_ARRAY_DEFAULT:
      return "ARRAYS_CONST_ARRAY_DEFAULT";
    case InferenceId::ARRAYS_EQ_TAUTOLOGY: return "ARRAYS_EQ_TAUTOLOGY";

    case InferenceId::BV_BITBLAST_INTERNAL_EAGER_LEMMA:
      return "BV_BITBLAST_EAGER_LEMMA";
    case InferenceId::BV_BITBLAST_INTERNAL_BITBLAST_LEMMA:
      return "BV_BITBLAST_INTERNAL_BITBLAST_LEMMA";

    case InferenceId::DATATYPES_PURIFY: return "DATATYPES_PURIFY";
    case InferenceId::DATATYPES_UNIF: return "DATATYPES_UNIF";
    case InferenceId::DATATYPES_INST: return "DATATYPES_INST";
    case InferenceId::DATATYPES_SPLIT: return "DATATYPES_SPLIT";
    case InferenceId::DATATYPES_LABEL_EXH: return "DATATYPES_LABEL_EXH";
    case InferenceId::DATATYPES_COLLAPSE_SEL: return "DATATYPES_COLLAPSE_SEL";
    case InferenceId::DATATYPES_CLASH_CONFLICT:
      return "DATATYPES_CLASH_CONFLICT";
    case InferenceId::DATATYPES_TESTER_CONFLICT:
      return "DATATYPES_TESTER_CONFLICT";
    case InferenceId::DATATYPES_TESTER_MERGE_CONFLICT:
      return "DATATYPES_TESTER_MERGE_CONFLICT";
    case InferenceId::DATATYPES_CYCLE: return "DATATYPES_CYCLE";
    case InferenceId::QUANTIFIERS_INST_E_MATCHING:
      return "QUANTIFIERS_INST_E_MATCHING";
    case InferenceId::QUANTIFIERS_INST_E_MATCHING_SIMPLE:
      return "QUANTIFIERS_INST_E_MATCHING_SIMPLE";
    case InferenceId::QUANTIFIERS_INST_E_MATCHING_MT:
      return "QUANTIFIERS_INST_E_MATCHING_MT";
    case InferenceId::QUANTIFIERS_INST_E_MATCHING_MTL:
      return "QUANTIFIERS_INST_E_MATCHING_MTL";
    case InferenceId::QUANTIFIERS_INST_E_MATCHING_RELATIONAL:
      return "QUANTIFIERS_INST_E_MATCHING_RELATIONAL";
    case InferenceId::QUANTIFIERS_INST_CBQI_CONFLICT:
      return "QUANTIFIERS_INST_CBQI_CONFLICT";
    case InferenceId::QUANTIFIERS_INST_CBQI_PROP:
      return "QUANTIFIERS_INST_CBQI_PROP";
    case InferenceId::QUANTIFIERS_INST_FMF_EXH:
      return "QUANTIFIERS_INST_FMF_EXH";
    case InferenceId::QUANTIFIERS_INST_FMF_FMC:
      return "QUANTIFIERS_INST_FMF_FMC";
    case InferenceId::QUANTIFIERS_INST_FMF_FMC_EXH:
      return "QUANTIFIERS_INST_FMF_FMC_EXH";
    case InferenceId::QUANTIFIERS_INST_CEGQI: return "QUANTIFIERS_INST_CEGQI";
    case InferenceId::QUANTIFIERS_INST_MBQI: return "QUANTIFIERS_INST_MBQI";
    case InferenceId::QUANTIFIERS_INST_ENUM: return "QUANTIFIERS_INST_ENUM";
    case InferenceId::QUANTIFIERS_BINT_PROXY: return "QUANTIFIERS_BINT_PROXY";
    case InferenceId::QUANTIFIERS_BINT_MIN_NG: return "QUANTIFIERS_BINT_MIN_NG";
    case InferenceId::QUANTIFIERS_CEGQI_CEX: return "QUANTIFIERS_CEGQI_CEX";
    case InferenceId::QUANTIFIERS_CEGQI_CEX_AUX:
      return "QUANTIFIERS_CEGQI_CEX_AUX";
    case InferenceId::QUANTIFIERS_CEGQI_CEX_DEP:
      return "QUANTIFIERS_CEGQI_CEX_DEP";
    case InferenceId::QUANTIFIERS_CEGQI_VTS_LB_DELTA:
      return "QUANTIFIERS_CEGQI_VTS_LB_DELTA";
    case InferenceId::QUANTIFIERS_CEGQI_VTS_UB_DELTA:
      return "QUANTIFIERS_CEGQI_VTS_UB_DELTA";
    case InferenceId::QUANTIFIERS_CEGQI_VTS_LB_INF:
      return "QUANTIFIERS_CEGQI_VTS_LB_INF";
    case InferenceId::QUANTIFIERS_DSPLIT: return "QUANTIFIERS_DSPLIT";
    case InferenceId::QUANTIFIERS_SKOLEMIZE: return "QUANTIFIERS_SKOLEMIZE";
    case InferenceId::QUANTIFIERS_REDUCE_ALPHA_EQ:
      return "QUANTIFIERS_REDUCE_ALPHA_EQ";
    case InferenceId::QUANTIFIERS_PARTIAL_TRIGGER_REDUCE:
      return "QUANTIFIERS_PARTIAL_TRIGGER_REDUCE";
    case InferenceId::QUANTIFIERS_GT_PURIFY: return "QUANTIFIERS_GT_PURIFY";
    case InferenceId::QUANTIFIERS_TDB_DEQ_CONG:
      return "QUANTIFIERS_TDB_DEQ_CONG";
    case InferenceId::QUANTIFIERS_CEGQI_WITNESS:
      return "QUANTIFIERS_CEGQI_WITNESS";

    case InferenceId::SETS_SKOLEM: return "SETS_SKOLEM";
    case InferenceId::SETS_CG_SPLIT: return "SETS_CG_SPLIT";
    case InferenceId::SETS_COMPREHENSION: return "SETS_COMPREHENSION";
    case InferenceId::SETS_DEQ: return "SETS_DEQ";
    case InferenceId::SETS_DOWN_CLOSURE: return "SETS_DOWN_CLOSURE";
    case InferenceId::SETS_EQ_CONFLICT: return "SETS_EQ_CONFLICT";
    case InferenceId::SETS_EQ_MEM: return "SETS_EQ_MEM";
    case InferenceId::SETS_EQ_MEM_CONFLICT: return "SETS_EQ_MEM_CONFLICT";
    case InferenceId::SETS_MEM_EQ: return "SETS_MEM_EQ";
    case InferenceId::SETS_MEM_EQ_CONFLICT: return "SETS_MEM_EQ_CONFLICT";
    case InferenceId::SETS_PROXY: return "SETS_PROXY";
    case InferenceId::SETS_PROXY_SINGLETON: return "SETS_PROXY_SINGLETON";
    case InferenceId::SETS_SINGLETON_EQ: return "SETS_SINGLETON_EQ";
    case InferenceId::SETS_UP_CLOSURE: return "SETS_UP_CLOSURE";
    case InferenceId::SETS_UP_CLOSURE_2: return "SETS_UP_CLOSURE_2";
    case InferenceId::STRINGS_I_NORM_S: return "STRINGS_I_NORM_S";
    case InferenceId::STRINGS_I_CONST_MERGE: return "STRINGS_I_CONST_MERGE";
    case InferenceId::STRINGS_I_CONST_CONFLICT:
      return "STRINGS_I_CONST_CONFLICT";
    case InferenceId::STRINGS_I_CYCLE_CONFLICT:
      return "STRINGS_I_CYCLE_CONFLICT";
    case InferenceId::STRINGS_I_NORM: return "STRINGS_I_NORM";
    case InferenceId::STRINGS_UNIT_SPLIT: return "STRINGS_UNIT_SPLIT";
    case InferenceId::STRINGS_UNIT_INJ_OOB: return "STRINGS_UNIT_INJ_OOB";
    case InferenceId::STRINGS_UNIT_INJ: return "STRINGS_UNIT_INJ";
    case InferenceId::STRINGS_UNIT_CONST_CONFLICT:
      return "STRINGS_UNIT_CONST_CONFLICT";
    case InferenceId::STRINGS_UNIT_INJ_DEQ: return "STRINGS_UNIT_INJ_DEQ";
    case InferenceId::STRINGS_CARD_SP: return "STRINGS_CARD_SP";
    case InferenceId::STRINGS_CARDINALITY: return "STRINGS_CARDINALITY";
    case InferenceId::STRINGS_I_CYCLE_E: return "STRINGS_I_CYCLE_E";
    case InferenceId::STRINGS_I_CYCLE: return "STRINGS_I_CYCLE";
    case InferenceId::STRINGS_F_CONST: return "STRINGS_F_CONST";
    case InferenceId::STRINGS_F_UNIFY: return "STRINGS_F_UNIFY";
    case InferenceId::STRINGS_F_ENDPOINT_EMP: return "STRINGS_F_ENDPOINT_EMP";
    case InferenceId::STRINGS_F_ENDPOINT_EQ: return "STRINGS_F_ENDPOINT_EQ";
    case InferenceId::STRINGS_F_NCTN: return "STRINGS_F_NCTN";
    case InferenceId::STRINGS_N_EQ_CONF: return "STRINGS_N_EQ_CONF";
    case InferenceId::STRINGS_N_ENDPOINT_EMP: return "STRINGS_N_ENDPOINT_EMP";
    case InferenceId::STRINGS_N_UNIFY: return "STRINGS_N_UNIFY";
    case InferenceId::STRINGS_N_ENDPOINT_EQ: return "STRINGS_N_ENDPOINT_EQ";
    case InferenceId::STRINGS_N_CONST: return "STRINGS_N_CONST";
    case InferenceId::STRINGS_INFER_EMP: return "STRINGS_INFER_EMP";
    case InferenceId::STRINGS_SSPLIT_CST_PROP: return "STRINGS_SSPLIT_CST_PROP";
    case InferenceId::STRINGS_SSPLIT_VAR_PROP: return "STRINGS_SSPLIT_VAR_PROP";
    case InferenceId::STRINGS_LEN_SPLIT: return "STRINGS_LEN_SPLIT";
    case InferenceId::STRINGS_LEN_SPLIT_EMP: return "STRINGS_LEN_SPLIT_EMP";
    case InferenceId::STRINGS_SSPLIT_CST: return "STRINGS_SSPLIT_CST";
    case InferenceId::STRINGS_SSPLIT_VAR: return "STRINGS_SSPLIT_VAR";
    case InferenceId::STRINGS_FLOOP: return "STRINGS_FLOOP";
    case InferenceId::STRINGS_FLOOP_CONFLICT: return "STRINGS_FLOOP_CONFLICT";
    case InferenceId::STRINGS_NORMAL_FORM: return "STRINGS_NORMAL_FORM";
    case InferenceId::STRINGS_N_NCTN: return "STRINGS_N_NCTN";
    case InferenceId::STRINGS_LEN_NORM: return "STRINGS_LEN_NORM";
    case InferenceId::STRINGS_DEQ_DISL_EMP_SPLIT:
      return "STRINGS_DEQ_DISL_EMP_SPLIT";
    case InferenceId::STRINGS_DEQ_DISL_FIRST_CHAR_EQ_SPLIT:
      return "STRINGS_DEQ_DISL_FIRST_CHAR_EQ_SPLIT";
    case InferenceId::STRINGS_DEQ_DISL_FIRST_CHAR_STRING_SPLIT:
      return "STRINGS_DEQ_DISL_FIRST_CHAR_STRING_SPLIT";
    case InferenceId::STRINGS_DEQ_STRINGS_EQ: return "STRINGS_DEQ_STRINGS_EQ";
    case InferenceId::STRINGS_DEQ_DISL_STRINGS_SPLIT:
      return "STRINGS_DEQ_DISL_STRINGS_SPLIT";
    case InferenceId::STRINGS_DEQ_LENS_EQ: return "STRINGS_DEQ_LENS_EQ";
    case InferenceId::STRINGS_DEQ_NORM_EMP: return "STRINGS_DEQ_NORM_EMP";
    case InferenceId::STRINGS_DEQ_LENGTH_SP: return "STRINGS_DEQ_LENGTH_SP";
    case InferenceId::STRINGS_DEQ_EXTENSIONALITY:
      return "STRINGS_DEQ_EXTENSIONALITY";
    case InferenceId::STRINGS_CODE_INJ: return "STRINGS_CODE_INJ";
    case InferenceId::STRINGS_RE_NF_CONFLICT: return "STRINGS_RE_NF_CONFLICT";
    case InferenceId::STRINGS_RE_UNFOLD_POS: return "STRINGS_RE_UNFOLD_POS";
    case InferenceId::STRINGS_RE_UNFOLD_NEG: return "STRINGS_RE_UNFOLD_NEG";
    case InferenceId::STRINGS_RE_INTER_INCLUDE:
      return "STRINGS_RE_INTER_INCLUDE";
    case InferenceId::STRINGS_RE_INTER_CONF: return "STRINGS_RE_INTER_CONF";
    case InferenceId::STRINGS_RE_INTER_INFER: return "STRINGS_RE_INTER_INFER";
    case InferenceId::STRINGS_RE_DELTA: return "STRINGS_RE_DELTA";
    case InferenceId::STRINGS_RE_DELTA_CONF: return "STRINGS_RE_DELTA_CONF";
    case InferenceId::STRINGS_RE_DERIVE: return "STRINGS_RE_DERIVE";
    case InferenceId::STRINGS_EXTF: return "STRINGS_EXTF";
    case InferenceId::STRINGS_EXTF_N: return "STRINGS_EXTF_N";
    case InferenceId::STRINGS_EXTF_D: return "STRINGS_EXTF_D";
    case InferenceId::STRINGS_EXTF_D_N: return "STRINGS_EXTF_D_N";
    case InferenceId::STRINGS_EXTF_EQ_REW: return "STRINGS_EXTF_EQ_REW";
    case InferenceId::STRINGS_EXTF_REW_SAME: return "STRINGS_EXTF_REW_SAME";
    case InferenceId::STRINGS_CTN_TRANS: return "STRINGS_CTN_TRANS";
    case InferenceId::STRINGS_CTN_DECOMPOSE: return "STRINGS_CTN_DECOMPOSE";
    case InferenceId::STRINGS_CTN_NEG_EQUAL: return "STRINGS_CTN_NEG_EQUAL";
    case InferenceId::STRINGS_CTN_POS: return "STRINGS_CTN_POS";
    case InferenceId::STRINGS_REDUCTION: return "STRINGS_REDUCTION";
    case InferenceId::STRINGS_PREFIX_CONFLICT: return "STRINGS_PREFIX_CONFLICT";
    case InferenceId::STRINGS_PREFIX_CONFLICT_MIN:
      return "STRINGS_PREFIX_CONFLICT_MIN";
    case InferenceId::STRINGS_ARITH_BOUND_CONFLICT:
      return "STRINGS_ARITH_BOUND_CONFLICT";
    case InferenceId::STRINGS_REGISTER_TERM_ATOMIC:
      return "STRINGS_REGISTER_TERM_ATOMIC";
    case InferenceId::STRINGS_REGISTER_TERM: return "STRINGS_REGISTER_TERM";
    case InferenceId::STRINGS_CMI_SPLIT: return "STRINGS_CMI_SPLIT";
    case InferenceId::STRINGS_CONST_SEQ_PURIFY:
      return "STRINGS_CONST_SEQ_PURIFY";
    case InferenceId::STRINGS_RE_EQ_ELIM_EQUIV:
      return "STRINGS_RE_EQ_ELIM_EQUIV";

    case InferenceId::UF_NOT_DISTINCT_ELIM: return "UF_NOT_DISTINCT_ELIM";
    case InferenceId::UF_DISTINCT_DEQ: return "UF_DISTINCT_DEQ";
    case InferenceId::UF_DISTINCT_DEQ_MODEL: return "UF_DISTINCT_DEQ_MODEL";
    case InferenceId::UF_ARITH_BV_CONV_REDUCTION:
      return "UF_ARITH_BV_CONV_REDUCTION";
    case InferenceId::UNKNOWN: return "?";

    default:
      DebugUnhandled() << "No print for inference id "
                       << static_cast<size_t>(i);
      return "?Unhandled";
  }
}

std::ostream& operator<<(std::ostream& out, InferenceId i)
{
  out << toString(i);
  return out;
}

Node mkInferenceIdNode(NodeManager* nm, InferenceId i)
{
  return nm->mkConstInt(Rational(static_cast<uint32_t>(i)));
}

bool getInferenceId(TNode n, InferenceId& i)
{
  uint32_t index;
  if (!ProofRuleChecker::getUInt32(n, index))
  {
    return false;
  }
  i = static_cast<InferenceId>(index);
  return true;
}

}  // namespace theory
}  // namespace ava6::internal
