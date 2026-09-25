/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Inference enumeration.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__INFERENCE_ID_H
#define AVA6__THEORY__INFERENCE_ID_H

#include <iosfwd>

#include "expr/node.h"

namespace ava6::internal {
namespace theory {

/** Types of inferences used in the procedure
 *
 * Note: The order in this enum matters in certain cases (e.g. inferences
 * related to normal forms in strings), where inferences that come first are
 * generally preferred.
 *
 * Notice that an inference is intentionally distinct from ProofRule. An
 * inference captures *why* we performed a reasoning step, and a ProofRule
 * rule captures *what* reasoning step was used. For instance, the inference
 * LEN_SPLIT translates to ProofRule::SPLIT. The use of stats on inferences
 * allows us to know that we performed N splits (ProofRule::SPLIT) because we
 * wanted to split on lengths for string equalities (Inference::LEN_SPLIT).
 */
enum class InferenceId
{
  NONE,
  // ---------------------------------- core
  // formulas coming from input
  INPUT,
  // a conflict when two constants merge in the equality engine (of any theory)
  EQ_CONSTANT_MERGE,
  // a split from theory combination
  COMBINATION_SPLIT,
  // a conflict due to rewriting an asserted literal
  CONFLICT_REWRITE_LIT,
  // a skolem lemma introduced by the theory preprocessor
  THEORY_PP_SKOLEM_LEM,
  // ---------------------------------- ext theory
  // a simplification from the extended theory utility
  EXTT_SIMPLIFY,
  // ---------------------------------- arith theory
  //-------------------- linear core
  // black box conflicts. It's magic.
  ARITH_BLACK_BOX,
  // conflicting equality
  ARITH_CONF_EQ,
  // conflicting lower bound
  ARITH_CONF_LOWER,
  // conflict due to trichotomy
  ARITH_CONF_TRICHOTOMY,
  // conflicting upper bound
  ARITH_CONF_UPPER,
  // conflict from simplex
  ARITH_CONF_SIMPLEX,
  // conflict from sum-of-infeasibility simplex
  ARITH_CONF_SOI_SIMPLEX,
  // conflict when getting constraint from fact queue
  ARITH_CONF_FACT_QUEUE,
  // conflict from handleUnateProp
  ARITH_CONF_UNATE_PROP,
  // introduces split on a disequality
  ARITH_SPLIT_DEQ,
  // states the equivalence of two atoms that correspond to the same internal
  // constraint, e.g. (= (= x 0) (= (to_real x) 0.0)) for integer x
  ARITH_EQUIV_ATOM,
  // tighten integer inequalities to ceiling
  ARITH_TIGHTEN_CEIL,
  // tighten integer inequalities to floor
  ARITH_TIGHTEN_FLOOR,
  ARITH_BB_LEMMA,
  ARITH_DIO_CUT,
  ARITH_DIO_DECOMPOSITION,
  // unate lemma during presolve
  ARITH_UNATE,
  // row implication
  ARITH_ROW_IMPL,
  // a split that occurs when the non-linear solver changes values of arithmetic
  // variables in a model, but those variables are inconsistent with assignments
  // from another theory
  ARITH_SPLIT_FOR_NL_MODEL,
  // dummy lemma to demand a restart
  ARITH_DEMAND_RESTART,
  //-------------------- preprocessing
  // equivalence of term and its preprocessed form
  ARITH_PP_ELIM_OPERATORS,
  // a lemma from arithmetic preprocessing
  ARITH_PP_ELIM_OPERATORS_LEMMA,
  // for theory combination when NL model construction identifies shared terms
  ARITH_NL_SHARED_TERM_SPLIT,
  // for theory combination when NL has a multiplication term with factors that
  // are not preregistered.
  ARITH_NL_SHARED_TERM_FACTOR_SPLIT,
  // based on sign (NlSolver::checkMonomialSign)
  ARITH_NL_SIGN,
  // based on comparing (abs) model values (NlSolver::checkMonomialMagnitude)
  ARITH_NL_COMPARISON,
  // based on inferring bounds (NlSolver::checkMonomialInferBounds), for
  // inferences that introduce new terms
  ARITH_NL_INFER_BOUNDS_NT,
  // factoring (NlSolver::checkFactoring)
  ARITH_NL_FACTOR,
  // tangent planes (NlSolver::checkTangentPlanes)
  ARITH_NL_TANGENT_PLANE,
  // flatten monomials (NonlinearExtension::checkFlattenMonomials).
  ARITH_NL_FLATTEN_MON,
  //-------------------- ff inference
  // ---------------------------------- end arith theory

  // ---------------------------------- finite field theory
  // a catch-all, for now
  // ---------------------------------- end finite field theory

  // ---------------------------------- arrays theory
  ARRAYS_EXT,
  ARRAYS_READ_OVER_WRITE,
  ARRAYS_READ_OVER_WRITE_1,
  ARRAYS_READ_OVER_WRITE_CONTRA,
  // (= (select (as const (Array T1 T2) x) y) x)
  ARRAYS_CONST_ARRAY_DEFAULT,
  // an internally inferred tautological equality
  ARRAYS_EQ_TAUTOLOGY,
  // ---------------------------------- end arrays theory

  // ---------------------------------- bitvector theory
  BV_BITBLAST_INTERNAL_EAGER_LEMMA,
  BV_BITBLAST_INTERNAL_BITBLAST_LEMMA,
  // ---------------------------------- end bitvector theory

  // ---------------------------------- datatypes theory
  // (= k t) for fresh k
  DATATYPES_PURIFY,
  // (= (C t1 ... tn) (C s1 .. sn)) => (= ti si)
  DATATYPES_UNIF,
  // ((_ is Ci) t) => (= t (Ci (sel_1 t) ... (sel_n t)))
  DATATYPES_INST,
  // (or ((_ is C1) t) V ... V ((_ is Cn) t))
  DATATYPES_SPLIT,
  // (not ((_ is C1) t)) ^ ... [j] ... ^ (not ((_ is Cn) t)) => ((_ is Cj) t)
  DATATYPES_LABEL_EXH,
  // (= t (Ci t1 ... tn)) => (= (sel_j t) rewrite((sel_j (Ci t1 ... tn))))
  DATATYPES_COLLAPSE_SEL,
  // (= (Ci t1...tn) (Cj t1...tn)) => false
  DATATYPES_CLASH_CONFLICT,
  // ((_ is Ci) t) ^ (= t (Cj t1 ... tn)) => false
  DATATYPES_TESTER_CONFLICT,
  // ((_ is Ci) t) ^ ((_ is Cj) s) ^ (= t s) => false
  DATATYPES_TESTER_MERGE_CONFLICT,
  // bisimilarity for codatatypes
  // corecursive singleton equality
  // corecursive singleton equality (not (= k1 k2)) for fresh k1, k2
  // cycle conflict for datatypes
  DATATYPES_CYCLE,
  // ---------------------------------- end datatypes theory

  //-------------------------------------- quantifiers theory
  //-------------------- types of instantiations.
  // Notice the identifiers in this section cover all the techniques used for
  // quantifier instantiation. The subcategories below are for specific lemmas
  // that are not instantiation lemmas added, per technique.
  // instantiation from E-matching
  QUANTIFIERS_INST_E_MATCHING,
  // E-matching using simple trigger implementation
  QUANTIFIERS_INST_E_MATCHING_SIMPLE,
  // E-matching using multi-triggers
  QUANTIFIERS_INST_E_MATCHING_MT,
  // E-matching using linear implementation of multi-triggers
  QUANTIFIERS_INST_E_MATCHING_MTL,
  // E-matching based on relational triggers
  QUANTIFIERS_INST_E_MATCHING_RELATIONAL,
  // conflicting instantiation from conflict-based instantiation
  QUANTIFIERS_INST_CBQI_CONFLICT,
  // propagating instantiation from conflict-based instantiation
  QUANTIFIERS_INST_CBQI_PROP,
  // instantiation from naive exhaustive instantiation in finite model finding
  // instantiation from finite model finding based on its model-based algorithm
  // instantiation from running exhaustive instantiation on a subdomain of
  // the quantified formula in finite model finding based on its model-based
  // algorithm
  // instantiations from counterexample-guided instantiation
  QUANTIFIERS_INST_FMF_EXH,
  QUANTIFIERS_INST_FMF_FMC,
  QUANTIFIERS_INST_FMF_FMC_EXH,
  QUANTIFIERS_INST_CEGQI,
  // instantiations from model-based instantiation
  QUANTIFIERS_INST_MBQI,
  // instantiations from enumerative instantiation
  QUANTIFIERS_INST_ENUM,
  //-------------------- bounded integers
  // a proxy lemma from bounded integers, used to control bounds on ground terms
  QUANTIFIERS_BINT_PROXY,
  // a proxy lemma to minimize an instantiation of non-ground terms
  QUANTIFIERS_BINT_MIN_NG,
  //-------------------- counterexample-guided instantiation
  // a counterexample lemma
  QUANTIFIERS_CEGQI_CEX,
  // an auxiliary lemma from counterexample lemma
  QUANTIFIERS_CEGQI_CEX_AUX,
  // G2 => G1 where G2 is a counterexample literal for a nested quantifier whose
  // counterexample literal is G1.
  QUANTIFIERS_CEGQI_CEX_DEP,
  // 0 < delta
  QUANTIFIERS_CEGQI_VTS_LB_DELTA,
  // delta < c, for positive c
  QUANTIFIERS_CEGQI_VTS_UB_DELTA,
  // infinity > c
  QUANTIFIERS_CEGQI_VTS_LB_INF,
  //-------------------- dynamic splitting
  // a dynamic split from quantifiers
  QUANTIFIERS_DSPLIT,
  //-------------------- miscellaneous
  // skolemization
  QUANTIFIERS_SKOLEMIZE,
  // Q1 <=> Q2, where Q1 and Q2 are alpha equivalent
  QUANTIFIERS_REDUCE_ALPHA_EQ,
  // a higher-order match predicate lemma
  // purification of non-variable higher-order function
  // reduction of quantifiers that don't have triggers that cover all variables
  QUANTIFIERS_PARTIAL_TRIGGER_REDUCE,
  // a purification lemma for a ground term appearing in a quantified formula,
  // used to ensure E-matching has equality information for that term
  QUANTIFIERS_GT_PURIFY,
  // when term indexing discovers disequal congruent terms in the master
  // equality engine
  QUANTIFIERS_TDB_DEQ_CONG,
  // An existential corresponding to a witness term generated based on BV
  // invertibility conditions.
  QUANTIFIERS_CEGQI_WITNESS,
  //-------------------------------------- end quantifiers theory

  // ---------------------------------- sep theory
  // ensures that pto is a function: (pto x y) ^ ~(pto z w) ^ x = z => y != w
  // enforces injectiveness of pto: (pto x y) ^ (pto y w) ^ x = y => y = w
  // introduces a label for a heap, of the form U => L, where U is an
  // unlabelled separation logic predicate and L is its labelled form
  // introduces the set constraints for a label
  // lemma for sep.emp
  // positive reduction for sep constraint
  // negative reduction for sep constraint
  // model-based refinement for negated star/wand
  // sep.nil is not in the heap
  // a symmetry breaking lemma
  // finite witness data lemma
  // element distinctness lemma
  // reference bound lemma
  // ---------------------------------- end sep theory

  // ---------------------------------- sets theory
  //-------------------- sets core solver
  // split when computing care graph
  SETS_SKOLEM,
  SETS_CG_SPLIT,
  SETS_COMPREHENSION,
  SETS_DEQ,
  SETS_DOWN_CLOSURE,
  // conflict when two singleton/emptyset terms merge
  SETS_EQ_CONFLICT,
  SETS_EQ_MEM,
  SETS_EQ_MEM_CONFLICT,
  SETS_MEM_EQ,
  SETS_MEM_EQ_CONFLICT,
  SETS_PROXY,
  SETS_PROXY_SINGLETON,
  SETS_SINGLETON_EQ,
  SETS_UP_CLOSURE,
  SETS_UP_CLOSURE_2,
  //-------------------------------------- end sets theory

  //-------------------------------------- strings theory
  //-------------------- base solver
  // initial normalize singular
  //   x1 = "" ^ ... ^ x_{i-1} = "" ^ x_{i+1} = "" ^ ... ^ xn = "" =>
  //   x1 ++ ... ++ xn = xi
  STRINGS_I_NORM_S,
  // initial constant merge
  //   explain_constant(x, c) => x = c
  // Above, explain_constant(x,c) is a basic explanation of why x must be equal
  // to string constant c, which is computed by taking arguments of
  // concatenation terms that are entailed to be constants. For example:
  //  ( y = "AB" ^ z = "C" ) => y ++ z = "ABC"
  STRINGS_I_CONST_MERGE,
  // initial constant conflict
  //    ( explain_constant(x, c1) ^ explain_constant(x, c2) ^ x = y) => false
  // where c1 != c2.
  STRINGS_I_CONST_CONFLICT,
  // An initial cycle conflict, for instance
  //   ( x = x ++ y ) ^ y = "B" => false
  STRINGS_I_CYCLE_CONFLICT,
  // initial normalize
  // Given two concatenation terms, this is applied when we find that they are
  // equal after e.g. removing strings that are currently empty. For example:
  //   y = "" ^ z = "" => x ++ y = z ++ x
  STRINGS_I_NORM,
  // split between the argument of two equated str.unit terms
  STRINGS_UNIT_SPLIT,
  // a code point must be out of bounds due to (str.unit x) = (str.unit y) and
  // x != y.
  STRINGS_UNIT_INJ_OOB,
  // injectivity of seq.unit
  // (seq.unit x) = (seq.unit y) => x=y, or
  // (seq.unit x) = (seq.unit c) => x=c
  STRINGS_UNIT_INJ,
  // unit constant conflict
  // (seq.unit x) = C => false if |C| != 1.
  STRINGS_UNIT_CONST_CONFLICT,
  // injectivity of seq.unit for disequality
  // (seq.unit x) != (seq.unit y) => x != y, or
  // (seq.unit x) != (seq.unit c) => x != c
  STRINGS_UNIT_INJ_DEQ,
  // A split due to cardinality
  STRINGS_CARD_SP,
  // The cardinality inference for strings, see Liang et al CAV 2014.
  STRINGS_CARDINALITY,
  //-------------------- core solver
  // A cycle in the empty string equivalence class, e.g.:
  //   x ++ y = "" => x = ""
  // This is typically not applied due to length constraints implying emptiness.
  STRINGS_I_CYCLE_E,
  // A cycle in the containment ordering.
  //   x = y ++ x => y = "" or
  //   x = y ++ z ^ y = x ++ w => z = "" ^ w = ""
  // This is typically not applied due to length constraints implying emptiness.
  STRINGS_I_CYCLE,
  // Flat form constant
  //   x = y ^ x = z ++ c ... ^ y = z ++ d => false
  // where c and d are distinct constants.
  STRINGS_F_CONST,
  // Flat form unify
  //   x = y ^ x = z ++ x' ... ^ y = z ++ y' ^ len(x') = len(y') => x' = y'
  // Notice flat form instances are similar to normal form inferences but do
  // not involve recursive explanations.
  STRINGS_F_UNIFY,
  // Flat form endpoint empty
  //   x = y ^ x = z ^ y = z ++ y' => y' = ""
  STRINGS_F_ENDPOINT_EMP,
  // Flat form endpoint equal
  //   x = y ^ x = z ++ x' ^ y = z ++ y' => x' = y'
  STRINGS_F_ENDPOINT_EQ,
  // Flat form not contained
  // x = c ^ x = y => false when rewrite( contains( y, c ) ) = false
  STRINGS_F_NCTN,
  // Normal form equality conflict
  //   x = N[x] ^ y = N[y] ^ x=y => false
  // where Rewriter::rewrite(N[x]=N[y]) = false.
  STRINGS_N_EQ_CONF,
  // Given two normal forms, infers that the remainder one of them has to be
  // empty. For example:
  //    If x1 ++ x2 = y1 and x1 = y1, then x2 = ""
  STRINGS_N_ENDPOINT_EMP,
  // Given two normal forms, infers that two components have to be the same if
  // they have the same length. For example:
  //   If x1 ++ x2 = x3 ++ x4 and len(x1) = len(x3) then x1 = x3
  STRINGS_N_UNIFY,
  // Given two normal forms, infers that the endpoints have to be the same. For
  // example:
  //   If x1 ++ x2 = x3 ++ x4 ++ x5 and x1 = x3 then x2 = x4 ++ x5
  STRINGS_N_ENDPOINT_EQ,
  // Given two normal forms with constant endpoints, infers a conflict if the
  // endpoints do not agree. For example:
  //   If "abc" ++ ... = "bc" ++ ... then conflict
  STRINGS_N_CONST,
  // infer empty, for example:
  //     (~) x = ""
  // This is inferred when we encounter an x such that x = "" rewrites to a
  // constant. This inference is used for instance when we otherwise would have
  // split on the emptiness of x but the rewriter tells us the emptiness of x
  // can be inferred.
  STRINGS_INFER_EMP,
  // string split constant propagation, for example:
  //     x = y, x = "abc", y = y1 ++ "b" ++ y2
  //       implies y1 = "a" ++ y1'
  STRINGS_SSPLIT_CST_PROP,
  // string split variable propagation, for example:
  //     x = y, x = x1 ++ x2, y = y1 ++ y2, len( x1 ) >= len( y1 )
  //       implies x1 = y1 ++ x1'
  // This is inspired by Zheng et al CAV 2015.
  STRINGS_SSPLIT_VAR_PROP,
  // length split, for example:
  //     len( x1 ) = len( y1 ) V len( x1 ) != len( y1 )
  // This is inferred when e.g. x = y, x = x1 ++ x2, y = y1 ++ y2.
  STRINGS_LEN_SPLIT,
  // length split empty, for example:
  //     z = "" V z != ""
  // This is inferred when, e.g. x = y, x = z ++ x1, y = y1 ++ z
  STRINGS_LEN_SPLIT_EMP,
  // string split constant
  //    x = y, x = "c" ++ x2, y = y1 ++ y2, y1 != ""
  //      implies y1 = "c" ++ y1'
  // This is a special case of F-Split in Figure 5 of Liang et al CAV 2014.
  STRINGS_SSPLIT_CST,
  // string split variable, for example:
  //    x = y, x = x1 ++ x2, y = y1 ++ y2
  //      implies x1 = y1 ++ x1' V y1 = x1 ++ y1'
  // This is rule F-Split in Figure 5 of Liang et al CAV 2014.
  STRINGS_SSPLIT_VAR,
  // flat form loop, for example:
  //    x = y, x = x1 ++ z, y = z ++ y2
  //      implies z = u2 ++ u1, u in ( u1 ++ u2 )*, x1 = u2 ++ u, y2 = u ++ u1
  //        for fresh u, u1, u2.
  // This is the rule F-Loop from Figure 5 of Liang et al CAV 2014.
  STRINGS_FLOOP,
  // loop conflict ???
  STRINGS_FLOOP_CONFLICT,
  // Normal form inference
  // x = y ^ z = y => x = z
  // This is applied when y is the normal form of both x and z.
  STRINGS_NORMAL_FORM,
  // Normal form not contained, same as FFROM_NCTN but for normal forms
  STRINGS_N_NCTN,
  // Length normalization
  //   x = y => len( x ) = len( y )
  // Typically applied when y is the normal form of x.
  STRINGS_LEN_NORM,
  // When x ++ x' ++ ... != "abc" ++ y' ++ ... ^ len(x) != len(y), we apply the
  // inference:
  //   x = "" v x != ""
  STRINGS_DEQ_DISL_EMP_SPLIT,
  // When x ++ x' ++ ... != "abc" ++ y' ++ ... ^ len(x) = 1, we apply the
  // inference:
  //   x = "a" v x != "a"
  STRINGS_DEQ_DISL_FIRST_CHAR_EQ_SPLIT,
  // When x ++ x' ++ ... != "abc" ++ y' ++ ... ^ len(x) != "", we apply the
  // inference:
  //   ni = x ++ x' ++ ... ^ nj = "abc" ++ y' ++ ... ^ x != "" --->
  //     x = k1 ++ k2 ^ len(k1) = 1 ^ (k1 != "a" v x = "a" ++  k2)
  STRINGS_DEQ_DISL_FIRST_CHAR_STRING_SPLIT,
  // When x ++ x' ++ ... != y ++ y' ++ ... ^ len(x) != len(y), we apply the
  // inference:
  //   ni = x ++ x' ++ ... ^ nj = y ++ y' ++ ... ^ ni != nj ^ len(x) != len(y)
  //     --->
  //       len(k1) = len(x) ^ len(k2) = len(y) ^ (y = k1 ++ k3 v x = k1 ++ k2)
  STRINGS_DEQ_DISL_STRINGS_SPLIT,
  // When x ++ x' ++ ... != y ++ y' ++ ... ^ len(x) = len(y), we apply the
  // inference:
  //   x = y v x != y
  STRINGS_DEQ_STRINGS_EQ,
  // When x ++ x' ++ ... != y ++ y' ++ ... and we do not know how the lengths
  // of x and y compare, we apply the inference:
  //   len(x) = len(y) v len(x) != len(y)
  STRINGS_DEQ_LENS_EQ,
  // When px ++ x ++ ... != py ^ len(px ++ x ++ ...) = len(py), we apply the
  // following inference that infers that the remainder of the longer normal
  // form must be empty:
  //   ni = px ++ x ++ ... ^ nj = py ^ len(ni) = len(nj) --->
  //     x = "" ^ ...
  STRINGS_DEQ_NORM_EMP,
  // When two strings are disequal s != t and the comparison of their lengths
  // is unknown, we apply the inference:
  //   len(s) != len(t) V len(s) = len(t)
  STRINGS_DEQ_LENGTH_SP,
  // Disequality extensionality
  // x != y => ( seq.len(x) != seq.len(y) or
  //             ( seq.nth(x, d) != seq.nth(y, d) ^ 0 <= d < seq.len(x) ) )
  STRINGS_DEQ_EXTENSIONALITY,
  //-------------------- codes solver
  // str.code(x) = -1 V str.code(x) != str.code(y) V x = y
  STRINGS_CODE_INJ,
  //-------------------- regexp solver
  // regular expression normal form conflict
  //   ( x in R ^ x = y ^ rewrite((str.in_re y R)) = false ) => false
  // where y is the normal form computed for x.
  STRINGS_RE_NF_CONFLICT,
  // regular expression unfolding
  // This is a general class of inferences of the form:
  //   (x in R) => F
  // where F is formula expressing the next step of checking whether x is in
  // R.  For example:
  //   (x in (R)*) =>
  //   x = "" V x in R V ( x = x1 ++ x2 ++ x3 ^ x1 in R ^ x2 in (R)* ^ x3 in R)
  STRINGS_RE_UNFOLD_POS,
  // Same as above, for negative memberships
  STRINGS_RE_UNFOLD_NEG,
  // intersection inclusion conflict
  //   (x in R1 ^ ~ x in R2) => false  where [[includes(R2,R1)]]
  // Where includes(R2,R1) is a heuristic check for whether R2 includes R1.
  STRINGS_RE_INTER_INCLUDE,
  // intersection conflict, using regexp intersection computation
  //   (x in R1 ^ x in R2) => false   where [[intersect(R1, R2) = empty]]
  STRINGS_RE_INTER_CONF,
  // intersection inference
  //   (x in R1 ^ y in R2 ^ x = y) => (x in re.inter(R1,R2))
  STRINGS_RE_INTER_INFER,
  // regular expression delta
  //   (x = "" ^ x in R) => C
  // where "" in R holds if and only if C holds.
  STRINGS_RE_DELTA,
  // regular expression delta conflict
  //   (x = "" ^ x in R) => false
  // where R does not accept the empty string.
  STRINGS_RE_DELTA_CONF,
  // regular expression derive ???
  STRINGS_RE_DERIVE,
  //-------------------- extended function solver
  // Standard extended function inferences from context-dependent rewriting
  // produced by constant substitutions. See Reynolds et al CAV 2017. These are
  // inferences of the form:
  //   X = Y => f(X) = t   when   rewrite( f(Y) ) = t
  // where X = Y is a vector of equalities, where some of Y may be constants.
  STRINGS_EXTF,
  // Same as above, for normal form substitutions.
  STRINGS_EXTF_N,
  // Decompositions based on extended function inferences from context-dependent
  // rewriting produced by constant substitutions. This is like the above, but
  // handles cases where the inferred predicate is not necessarily an equality
  // involving f(X). For example:
  //   x = "A" ^ contains( y ++ x, "B" ) => contains( y, "B" )
  // This is generally only inferred if contains( y, "B" ) is a known term in
  // the current context.
  STRINGS_EXTF_D,
  // Same as above, for normal form substitutions.
  STRINGS_EXTF_D_N,
  // Extended function equality rewrite. This is an inference of the form:
  //   t = s => P
  // where P is a predicate implied by rewrite( t = s ).
  // Typically, t is an application of an extended function and s is a constant.
  // It is generally only inferred if P is a predicate over known terms.
  STRINGS_EXTF_EQ_REW,
  // two terms rewrite to the same thing
  // in particular this is of the form (E1 ^ E2) => t1 = t2
  // where E1 => t1 = tr and E2 => t2 = tr.
  STRINGS_EXTF_REW_SAME,
  // contain transitive
  //   ( str.contains( s, t ) ^ ~contains( s, r ) ) => ~contains( t, r ).
  STRINGS_CTN_TRANS,
  // contain decompose
  //  str.contains( x, str.++( y1, ..., yn ) ) => str.contains( x, yi ) or
  //  ~str.contains( str.++( x1, ..., xn ), y ) => ~str.contains( xi, y )
  STRINGS_CTN_DECOMPOSE,
  // contain neg equal
  //   ( len( x ) = len( s ) ^ ~contains( x, s ) ) => x != s
  STRINGS_CTN_NEG_EQUAL,
  // contain positive
  //   str.contains( x, y ) => x = w1 ++ y ++ w2
  // where w1 and w2 are skolem variables.
  STRINGS_CTN_POS,
  // All reduction inferences of the form:
  //   f(x1, .., xn) = y ^ P(x1, ..., xn, y)
  // where f is an extended function, y is the purification variable for
  // f(x1, .., xn) and P is the reduction predicate for f
  // (see theory_strings_preprocess).
  STRINGS_REDUCTION,
  //-------------------- merge conflicts
  // prefix conflict
  STRINGS_PREFIX_CONFLICT,
  // minimized prefix conflict
  STRINGS_PREFIX_CONFLICT_MIN,
  // arithmetic bound conflict
  STRINGS_ARITH_BOUND_CONFLICT,
  //-------------------- other
  // a lemma added during term registration for an atomic term
  STRINGS_REGISTER_TERM_ATOMIC,
  // a lemma added during term registration
  STRINGS_REGISTER_TERM,
  // a split during collect model info
  STRINGS_CMI_SPLIT,
  // constant sequence purification
  STRINGS_CONST_SEQ_PURIFY,
  // regular expression equality equivalence
  STRINGS_RE_EQ_ELIM_EQUIV,
  //-------------------------------------- end strings theory

  //-------------------------------------- uf theory
  // Clause from the uf symmetry breaker
  // Lemma of the form
  // (~distinct(t1...tn) => ~blastDistinct(distinct(t1...tn))
  UF_NOT_DISTINCT_ELIM,
  // Conflict of the form (distinct(t1...tn) ^ ti = tj)
  UF_DISTINCT_DEQ,
  // Lemma of the form (~distinct(t1...tn) or ti != tj) sent during last call
  UF_DISTINCT_DEQ_MODEL,
  //-------------------- UF arith/bv conversions solver
  // reductions of an arithmetic/bit-vector conversion term
  UF_ARITH_BV_CONV_REDUCTION,
  //-------------------------------------- end uf theory

  //-------------------------------------- unknown
  UNKNOWN
};

/**
 * Converts an inference to a string. Note: This function is also used in
 * `safe_print()`. Changing this functions name or signature will result in
 * `safe_print()` printing "<unsupported>" instead of the proper strings for
 * the enum values.
 *
 * @param i The inference
 * @return The name of the inference
 */
const char* toString(InferenceId i);

/**
 * Writes an inference name to a stream.
 *
 * @param out The stream to write to
 * @param i The inference to write to the stream
 * @return The stream
 */
std::ostream& operator<<(std::ostream& out, InferenceId i);

/** Make node from inference id */
Node mkInferenceIdNode(NodeManager* nm, InferenceId i);

/** get an inference identifier from a node, return false if we fail */
bool getInferenceId(TNode n, InferenceId& i);

}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__INFERENCE_H */
