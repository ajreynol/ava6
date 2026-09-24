/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implementation of setting default options.
 */

#include "smt/set_defaults.h"

#include <sstream>

#include "base/output.h"
#include "options/arith_options.h"
#include "options/arrays_options.h"
#include "options/base_options.h"
#include "options/booleans_options.h"
#include "options/bv_options.h"
#include "options/datatypes_options.h"
#include "options/decision_options.h"
#include "options/language.h"
#include "options/main_options.h"
#include "options/option_exception.h"
#include "options/parser_options.h"
#include "options/printer_options.h"
#include "options/proof_options.h"
#include "options/prop_options.h"
#include "options/quantifiers_options.h"
#include "options/sets_options.h"
#include "options/smt_options.h"
#include "options/strings_options.h"
#include "options/theory_options.h"
#include "options/uf_options.h"
#include "smt/logic_exception.h"
#include "theory/theory.h"

using namespace ava6::internal::theory;

namespace ava6::internal {
namespace smt {

/**
 * Throw an option exception if domain.optName is set by the user and not the
 * given value. Give an error message where reason is given.
 * Note this macro should be used if the value is concrete.
 */
#define OPTION_EXCEPTION_IF_NOT(domain, optName, value, reason)               \
  if (opts.write_##domain().optName##WasSetByUser                             \ && opts.write_##domain().optName != value)                              \
  {                                                                           \
    std::stringstream ss;                                                     \
    ss << "Cannot use --" << options::domain::longName::optName << " due to " \
       << reason << ".";                                                      \
    throw FatalOptionException(ss.str());                                     \
  }
/**
 * Set domain.optName to value due to reason. Notify if value changes.
 * Note this macro should be used if the value is concrete.
 */
#define SET_AND_NOTIFY(domain, optName, value, reason)                      \
  if (opts.write_##domain().optName != value)                               \
  {                                                                         \
    notifyModifyOption(options::domain::longName::optName, #value, reason); \
    opts.write_##domain().optName = value;                                  \
  }
/**
 * Set domain.optName to value due to reason. Notify if value changes.
 *
 * Note this macro should be used if the value passed to the macro is not
 * concrete (i.e., stored in a variable).
 */
#define SET_AND_NOTIFY_VAL_SYM(domain, optName, value, reason)    \
  if (opts.write_##domain().optName != value)                     \
  {                                                               \
    std::stringstream sstmp;                                      \
    sstmp << value;                                               \
    notifyModifyOption(                                           \
        options::domain::longName::optName, sstmp.str(), reason); \
    opts.write_##domain().optName = value;                        \
  }
/**
 * Set domain.optName to value due to reason if the option was not already set
 * by the user. Notify if value changes.
 * Note this macro should be used if the value is concrete.
 */
#define SET_AND_NOTIFY_IF_NOT_USER(domain, optName, value, reason)          \
  if (!opts.write_##domain().optName##WasSetByUser                          \ && opts.write_##domain().optName != value)                            \
  {                                                                         \
    notifyModifyOption(options::domain::longName::optName, #value, reason); \
    opts.write_##domain().optName = value;                                  \
  }
/**
 * Set domain.optName to value due to reason if the option was not already set
 * by the user. Notify if value changes.
 */
#define SET_AND_NOTIFY_IF_NOT_USER_VAL_SYM(domain, optName, value, reason) \
  if (!opts.write_##domain().optName##WasSetByUser                         \ && opts.write_##domain().optName != value)                           \
  {                                                                        \
    std::stringstream sstmp;                                               \
    sstmp << value;                                                        \
    notifyModifyOption(                                                    \
        options::domain::longName::optName, sstmp.str(), reason);          \
    opts.write_##domain().optName = value;                                 \
  }

SetDefaults::SetDefaults(Env& env, bool isInternalSubsolver)
    : EnvObj(env), d_isInternalSubsolver(isInternalSubsolver)
{
}

void SetDefaults::setDefaults(LogicInfo& logic, Options& opts)
{
  // initial changes that are independent of logic, and may impact the logic
  setDefaultsPre(opts);
  // now, finalize the logic
  finalizeLogic(logic, opts);
  // further changes to options based on the logic
  setDefaultsPost(logic, opts);
}

void SetDefaults::setDefaultsPre(Options& opts)
{
  if (opts.smt.debugCheckModels)
  {
    SET_AND_NOTIFY(smt, checkModels, true, "debug-check-models");
  }
  if (opts.smt.checkModels || opts.driver.dumpModels || opts.smt.produceAssignments)
  {
    SET_AND_NOTIFY(smt, produceModels, true, "model output or checking");
  }
  if (opts.smt.checkModels)
  {
    SET_AND_NOTIFY(smt, produceAssignments, true, "check-models");
  }
  if (opts.smt.checkProofs || opts.driver.dumpProofs || opts.driver.dumpUnsatCoresLemmas)
  {
    SET_AND_NOTIFY(smt, produceProofs, true, "proof output or checking");
  }
  const bool fullProofs = opts.smt.produceProofs;
  if (fullProofs || opts.smt.checkUnsatCores || opts.driver.dumpUnsatCores || opts.driver.dumpUnsatCoresLemmas || opts.smt.unsatAssumptions)
  {
    SET_AND_NOTIFY(smt, produceUnsatCores, true, "proofs or unsat cores");
  }
  opts.solver.proofMode = fullProofs ? options::ProofMode::FULL_STRICT
      : opts.smt.produceUnsatCores ? options::ProofMode::PP_ONLY
                                  : options::ProofMode::OFF;
  opts.solver.unsatCoresMode = fullProofs ? options::UnsatCoresMode::SAT_PROOF
      : opts.smt.produceUnsatCores ? options::UnsatCoresMode::ASSUMPTIONS
                                  : options::UnsatCoresMode::OFF;
  if (fullProofs && !opts.proof.proofGranularityModeWasSetByUser)
  {
    SET_AND_NOTIFY(proof, proofGranularityMode,
                   options::ProofGranularityMode::DSL_REWRITE, "proofs");
  }
  opts.solver.checkProofsComplete = opts.smt.checkProofs
      && opts.proof.proofGranularityMode >= options::ProofGranularityMode::DSL_REWRITE;
  if (opts.smt.produceUnsatCores)
  {
    // Assumption cores still need proofs of preprocessing steps.
    SET_AND_NOTIFY(smt, produceProofs, true, "unsat cores");
  }
}

void SetDefaults::finalizeLogic(LogicInfo& logic, Options& opts) const
{

  if (opts.bv.bitblastMode == options::BitblastMode::EAGER)
  {
    if (opts.smt.produceModels && (logic.isTheoryEnabled(THEORY_ARRAYS) || logic.isTheoryEnabled(THEORY_UF)))
    {
      if (opts.bv.bitblastModeWasSetByUser || opts.smt.produceModelsWasSetByUser)
      {
        std::stringstream ss;
        ss << "Eager bit-blasting currently does not support model generation ";
        ss << "for the combination of bit-vectors with arrays or uinterpreted ";
        ss << "functions. Try --" << options::bv::longName::bitblastMode << "="
           << options::BitblastMode::LAZY << ".";
        throw FatalOptionException(ss.str());
      }
      SET_AND_NOTIFY(
          bv, bitblastMode, options::BitblastMode::LAZY, "model generation");
    }
    else if (!opts.base.incrementalSolving)
    {
      // if not incremental, we rely on ackermann to eliminate other theories.
      opts.solver.ackermann = true;
    }
    else if (logic.isQuantified() || !logic.isPure(THEORY_BV))
    {
      // requested bitblast=eager in incremental mode, must be QF_BV only.
      throw FatalOptionException(
          std::string("Eager bit-blasting is only support in incremental mode "
                      "if the logic is quantifier-free bit-vectors"));
    }
  }

  // set options about ackermannization
  if (opts.solver.ackermann && opts.smt.produceModels && (logic.isTheoryEnabled(THEORY_ARRAYS) || logic.isTheoryEnabled(THEORY_UF)))
  {
    if (opts.smt.produceModelsWasSetByUser)
    {
      throw FatalOptionException(std::string(
          "Ackermannization currently does not support model generation."));
    }
    opts.solver.ackermann = false;
    // we are not relying on ackermann to eliminate theories in this case
    Assert(opts.bv.bitblastMode != options::BitblastMode::EAGER);
  }

  if (opts.solver.ackermann)
  {
    if (logic.isTheoryEnabled(THEORY_UF))
    {
      logic = logic.getUnlockedCopy();
      logic.disableTheory(THEORY_UF);
      logic.lock();
    }
  }

  // Set default options associated with strings-exp, which is enabled by
  // default if the logic includes strings. Note that enabling stringExp
  // enables quantifiers in the logic, and enables the bounded integer
  // quantifiers module for processing *only* bounded quantifiers generated by
  // the strings theory. It should not have an impact otherwise.
  if (logic.isTheoryEnabled(THEORY_STRINGS) && !options().strings.stringExpWasSetByUser)
  {
    SET_AND_NOTIFY(strings, stringExp, true, "logic including strings");
  }
  // If strings-exp is enabled, we require quantifiers. We also enable them
  // if we are using eager string preprocessing or aggressive regular expression
  // elimination, which may introduce quantified formulas at preprocess time.
  if (opts.strings.stringExp)
  {
    // We require quantifiers since extended functions reduce using them.
    if (!logic.isQuantified())
    {
      logic = logic.getUnlockedCopy();
      logic.enableQuantifiers();
      logic.lock();
      Trace("smt") << "turning on quantifier logic, for strings-exp"
                   << std::endl;
    }
    // Note we allow E-matching by default to support combinations of sequences
    // and quantifiers. We also do not enable fmfBound here, which would
    // enable bounded integer instantiation for *all* quantifiers. Instead,
    // the bounded integers module will always process internally generated
    // quantifiers (those marked with InternalQuantAttribute).
  }

  // We now know whether the input uses sygus. Update the logic to incorporate
  // the theories we need internally for handling sygus problems.

  // widen the logic
  widenLogic(logic, opts);

  // check if we have any options that are not supported with quantified logics
  if (logic.isQuantified())
  {
    std::stringstream reasonNoQuant;
    if (incompatibleWithQuantifiers(opts, reasonNoQuant))
    {
      std::stringstream ss;
      ss << reasonNoQuant.str() << " not supported in quantified logics.";
      throw FatalOptionException(ss.str());
    }
  }
  // check if we have separation logic heap types
  
}

void SetDefaults::setDefaultsPost(const LogicInfo& logic, Options& opts) const
{
  SET_AND_NOTIFY(smt, produceAssertions, true, "always enabled");

  // Disable options incompatible with incremental solving, or output an error
  // if enabled explicitly.
  if (opts.base.incrementalSolving)
  {
    std::stringstream reasonNoInc;
    std::stringstream suggestNoInc;
    if (incompatibleWithIncremental(logic, opts, reasonNoInc, suggestNoInc))
    {
      std::stringstream ss;
      ss << reasonNoInc.str() << " not supported with incremental solving. "
         << suggestNoInc.str();
      throw FatalOptionException(ss.str());
    }
  }

  // Disable options incompatible with unsat cores or output an error if enabled
  // explicitly
  if (opts.smt.produceUnsatCores)
  {
    // check if the options are not compatible with unsat cores
    
  }
  else
  {
    // Turn on unconstrained simplification for QF_AUFBV
    if (!opts.base.incrementalSolving)
    {
      // It is also currently incompatible with arithmetic, force the option
      // off.
      bool uncSimp = !opts.base.incrementalSolving && !logic.isQuantified()
                     && !opts.smt.produceModels && !opts.smt.produceAssignments
                     && !opts.smt.checkModels
                     && logic.isTheoryEnabled(THEORY_ARRAYS)
                     && logic.isTheoryEnabled(THEORY_BV)
                     && !logic.isTheoryEnabled(THEORY_ARITH);
      opts.solver.unconstrainedSimp = uncSimp;
    }

    // by default, nonclausal simplification is off for QF_SAT
    if (!opts.smt.simplificationModeWasSetByUser)
    {
      bool qf_sat = logic.isPure(THEORY_BOOL) && !logic.isQuantified();
      // simplification=none works better for SMT LIB benchmarks with
      // quantifiers, not others
      if (qf_sat)
      {
        SET_AND_NOTIFY_VAL_SYM(smt,
                               simplificationMode,
                               options::SimplificationMode::NONE,
                               "logic");
      }
      else
      {
        SET_AND_NOTIFY_VAL_SYM(smt,
                               simplificationMode,
                               options::SimplificationMode::BATCH,
                               "logic");
      }
    }
  }

  // cases where we need produce models
  if (opts.smt.produceAssignments)
  {
    SET_AND_NOTIFY(smt, produceModels, true, "produce assignments or sygus");
  }

  // --ite-simp is an experimental option designed for QF_LIA/nec. This
  // technique is experimental. This benchmark set also requires removing ITEs
  // during preprocessing, before repeating simplification. Hence, we enable
  // this by default.

  // Set the options for the theoryOf
  {
    if (logic.isSharingEnabled() && !logic.isTheoryEnabled(THEORY_BV) && !logic.isTheoryEnabled(THEORY_STRINGS) && !logic.isTheoryEnabled(THEORY_SETS) && !(logic.isTheoryEnabled(THEORY_ARITH) && !logic.isLinear()
             && !logic.isQuantified()))
    {
      opts.solver.theoryOfMode = options::TheoryOfMode::THEORY_OF_TERM_BASED;
    }
  }

  // By default, symmetry breaker is on only for non-incremental QF_UF.
  // Note that if ufSymmetryBreaker is already set to false, we do not reenable
  // it.

  // If in arrays, set the UF handler to arrays
  if (logic.isTheoryEnabled(THEORY_ARRAYS) && (!logic.isQuantified() || (logic.isQuantified() && !logic.isTheoryEnabled(THEORY_UF))))
  {
    d_env.setUninterpretedSortOwner(THEORY_ARRAYS);
  }
  else
  {
    d_env.setUninterpretedSortOwner(THEORY_UF);
  }

  // Turn off array eager index splitting for QF_AUFLIA
  if (!opts.arrays.arraysEagerIndexSplittingWasSetByUser)
  {
    if (!logic.isQuantified() && logic.isTheoryEnabled(THEORY_ARRAYS) && logic.isTheoryEnabled(THEORY_UF) && logic.isTheoryEnabled(THEORY_ARITH))
    {
      SET_AND_NOTIFY(arrays, arraysEagerIndexSplitting, false, "logic");
    }
  }
  // Turn on multiple-pass non-clausal simplification for QF_AUFBV
  {
    bool repeatSimp = !logic.isQuantified()
                      && (logic.isTheoryEnabled(THEORY_ARRAYS)
                          && logic.isTheoryEnabled(THEORY_UF)
                          && logic.isTheoryEnabled(THEORY_BV))
                      && opts.solver.unsatCoresMode != options::UnsatCoresMode::ASSUMPTIONS;
    opts.solver.repeatSimp = repeatSimp;
  }

  /* Disable bit-level propagation by default for the BITBLAST solver. */

  if (opts.bv.boolToBitvector == options::BoolToBVMode::ALL && !logic.isTheoryEnabled(THEORY_BV))
  {
    if (opts.bv.boolToBitvectorWasSetByUser)
    {
      throw FatalOptionException(
          "bool-to-bv=all not supported for non-bitvector logics.");
    }
    SET_AND_NOTIFY_VAL_SYM(
        bv, boolToBitvector, options::BoolToBVMode::OFF, "non-BV logic");
  }

  // Turn on arith rewrite equalities only for pure arithmetic
  if (!opts.arith.arithRewriteEqWasSetByUser)
  {
    bool arithRewriteEq =
        logic.isPure(THEORY_ARITH) && logic.isLinear() && !logic.isQuantified();
    SET_AND_NOTIFY_VAL_SYM(arith, arithRewriteEq, arithRewriteEq, "logic");
  }
  {
    int16_t heuristicPivots = 5;
    if (logic.isPure(THEORY_ARITH) && !logic.isQuantified())
    {
      if (logic.isDifferenceLogic())
      {
        heuristicPivots = -1;
      }
      else if (!logic.areIntegersUsed())
      {
        heuristicPivots = 0;
      }
    }
    opts.solver.arithHeuristicPivots = heuristicPivots;
  }
  {
    uint16_t pivotThreshold = 2;
    if (logic.isPure(THEORY_ARITH) && !logic.isQuantified())
    {
      if (logic.isDifferenceLogic())
      {
        pivotThreshold = 16;
      }
    }
    opts.solver.arithPivotThreshold = pivotThreshold;
  }
  {
    int16_t varOrderPivots = -1;
    if (logic.isPure(THEORY_ARITH) && !logic.isQuantified())
    {
      varOrderPivots = 200;
    }
    opts.solver.arithStandardCheckVarOrderPivots = varOrderPivots;
  }
  // DIO solver typically makes things worse for quantifier-free logics with
  // non-linear arithmetic.
  if (!logic.isQuantified() && logic.isTheoryEnabled(THEORY_ARITH) && !logic.isLinear())
  {
    opts.solver.arithDioSolver = false;
  }
  if (logic.isPure(THEORY_ARITH) && !logic.areRealsUsed())
  {
    SET_AND_NOTIFY(
        arith, nlExtTangentPlanesInterleave, true, "pure integer logic");
  }
  {
    bool val = !logic.isQuantified();
    // use bound inference to determine when bounds are irrelevant only when
    // the logic is quantifier-free
    opts.solver.nlRlvAssertBounds = val;
  }

  // set the default decision mode
  setDefaultDecisionMode(logic, opts);

  // set up of central equality engine
  {
    // use the arithmetic equality solver by default
    
  }

  // set all defaults in the quantifiers theory, which includes sygus
  setDefaultsQuantifiers(logic, opts);

  // Shared selectors are generally not good to combine with standard
  // quantifier techniques e.g. E-matching.
  // We only enable them if SyGuS is enabled.

  // For now, these array theory optimizations do not support model-building
  if (opts.smt.produceModels || opts.smt.produceAssignments || opts.smt.checkModels)
  {
    opts.solver.arraysOptimizeLinear = false;
  }

  // !!! All options that require disabling models go here

  if (opts.bv.bitblastMode == options::BitblastMode::EAGER && !logic.isPure(THEORY_BV) && logic.getLogicString() !=)
  {
    throw FatalOptionException(
        "Eager bit-blasting does not currently support theory combination with "
        "any theory other than UF. ");
  }

  // Note that if nlCov is already set to false, we do not reenable it.

  if (logic.isTheoryEnabled(theory::THEORY_ARITH) && logic.areTranscendentalsUsed())
  {
    SET_AND_NOTIFY_IF_NOT_USER_VAL_SYM(
        arith, nlExt, options::NlExtMode::FULL, "logic with transcendentals");
  }
  if (isOutputOn(OutputTag::NORMALIZE))
  {
    SET_AND_NOTIFY(base, preprocessOnly, true, "normalize output");
  }
  
}

bool SetDefaults::incompatibleWithIncremental(const LogicInfo& logic,
                                              Options& opts,
                                              std::ostream& reason,
                                              std::ostream& suggest) const
{
  
  if (opts.solver.ackermann)
  {
    reason << "ackermann";
    return true;
  }
  if (opts.solver.unconstrainedSimp)
  {
    
    opts.solver.unconstrainedSimp = false;
  }
  if (opts.bv.bitblastMode == options::BitblastMode::EAGER && !logic.isPure(THEORY_BV))
  {
    reason << "eager bit-blasting in non-QF_BV logic";
    suggest << "Try --" << options::bv::longName::bitblastMode << "="
            << options::BitblastMode::LAZY << ".";
    return true;
  }

  // proof logging not yet supported in incremental mode, which requires
  // managing how new assertions are printed.

  // disable modes not supported by incremental

  return false;
}

bool SetDefaults::incompatibleWithQuantifiers(const Options& opts,
                                              std::ostream& reason) const
{
  if (opts.solver.ackermann)
  {
    reason << "ackermann";
    return true;
  }
  
  return false;
}

void SetDefaults::widenLogic(LogicInfo& logic, AVA6_UNUSED const Options& opts) const
{
  bool needsUf = false;
  // strings require LIA, UF; widen the logic
  if (logic.isTheoryEnabled(THEORY_STRINGS))
  {
    LogicInfo log(logic.getUnlockedCopy());
    // Strings requires arith for length constraints, and also UF
    needsUf = true;
    if (!logic.isTheoryEnabled(THEORY_ARITH) || logic.isDifferenceLogic())
    {
      verbose(1)
          << "Enabling linear integer arithmetic because strings are enabled"
          << std::endl;
      log.enableTheory(THEORY_ARITH);
      log.enableIntegers();
      log.arithOnlyLinear();
    }
    else if (!logic.areIntegersUsed())
    {
      verbose(1) << "Enabling integer arithmetic because strings are enabled"
                 << std::endl;
      log.enableIntegers();
    }
    logic = log;
    logic.lock();
  }

  if (needsUf || logic.isTheoryEnabled(THEORY_ARRAYS) || logic.isTheoryEnabled(THEORY_DATATYPES) || logic.isTheoryEnabled(THEORY_SETS) || (logic.isTheoryEnabled(THEORY_ARITH) && !logic.isLinear()) || (logic.isTheoryEnabled(THEORY_ARITH) && logic.isTheoryEnabled(THEORY_BV)))
  {
    if (!logic.isTheoryEnabled(THEORY_UF))
    {
      LogicInfo log(logic.getUnlockedCopy());
      if (!needsUf)
      {
        verbose(1) << "Enabling UF because " << logic << " requires it."
                   << std::endl;
      }
      log.enableTheory(THEORY_UF);
      logic = log;
      logic.lock();
    }
  }
  
}

void SetDefaults::setDefaultsQuantifiers(const LogicInfo& logic,
                                         Options& opts) const
{
  if (opts.quantifiers.fullSaturateQuant)
  {
    SET_AND_NOTIFY(quantifiers, enumInst, true, "full-saturate-quant");
  }

  // enable MBQI if --mbqi-enum is provided

  if (opts.quantifiers.mbqi)
  {
    // MBQI is an alternative to CEGQI/SyQI
    SET_AND_NOTIFY_IF_NOT_USER(quantifiers, cegqi, false, "mbqi");

  }

  // Configure bounded quantifier enumeration.
  if (opts.quantifiers.fmfBound)
  {
    // if bounded integers are set, use no MBQI by default
    opts.solver.fmfMbqiMode = options::FmfMbqiMode::NONE;
    opts.solver.prenexQuant = options::PrenexQuantMode::NONE;
  }

  // now, have determined whether finite model find is on/off
  // apply finite model finding options

  // apply sygus options
  // if we are attempting to rewrite everything to SyGuS, use sygus()

  // counterexample-guided instantiation for non-sygus
  // enable if any possible quantifiers with arithmetic, datatypes or bitvectors
  if ((logic.isQuantified() && (logic.isTheoryEnabled(THEORY_ARITH) || logic.isTheoryEnabled(THEORY_DATATYPES) || logic.isTheoryEnabled(THEORY_BV))))
  {
    SET_AND_NOTIFY_IF_NOT_USER(quantifiers, cegqi, true, "logic");
    // check whether we should apply full cbqi
    if (logic.isPure(THEORY_BV))
    {
      opts.solver.cegqiFullEffort = true;
    }
  }
  if (opts.quantifiers.cegqi)
  {
    if (logic.isPure(THEORY_ARITH) || logic.isPure(THEORY_BV))
    {
      SET_AND_NOTIFY_IF_NOT_USER(
          quantifiers, conflictBasedInst, false, "cegqi pure logic");
      SET_AND_NOTIFY_IF_NOT_USER(
          quantifiers, instNoEntail, false, "cegqi pure logic");
      // only instantiation should happen at last call when model is avaiable
      SET_AND_NOTIFY_IF_NOT_USER_VAL_SYM(quantifiers,
                                         instWhenMode,
                                         options::InstWhenMode::LAST_CALL,
                                         "cegqi pure logic");
    }
    
  }
  // implied options...
  if (opts.quantifiers.cbqiModeWasSetByUser)
  {
    SET_AND_NOTIFY(quantifiers, conflictBasedInst, true, "cbqi option");
  }
  // for induction techniques

  // can't pre-skolemize nested quantifiers without UF theory
  if (!logic.isTheoryEnabled(THEORY_UF) && opts.quantifiers.preSkolemQuant != options::PreSkolemQuantMode::OFF)
  {
    opts.solver.preSkolemQuantNested = false;
  }
  if (!logic.isTheoryEnabled(THEORY_DATATYPES))
  {
    SET_AND_NOTIFY_VAL_SYM(quantifiers,
                           quantDynamicSplit,
                           options::QuantDSplitMode::NONE,
                           "non-datatypes logic");
  }
  
}

void SetDefaults::setDefaultDecisionMode(const LogicInfo& logic,
                                         Options& opts) const
{
  // Set decision mode based on logic (if not set by user)
  if (opts.decision.decisionModeWasSetByUser)
  {
    return;
  }
  options::DecisionMode decMode =
      // anything that uses sygus uses internal

                      // ALL or its supersets
          logic.hasEverything()
          ? options::DecisionMode::JUSTIFICATION
          : (  // QF_BV without internal bit-blasting
                (!logic.isQuantified() && logic.isPure(THEORY_BV)
                 && false)
                        ||
                        // QF_AUFBV or QF_ABV or QF_UFBV
                        (!logic.isQuantified()
                         && (logic.isTheoryEnabled(THEORY_ARRAYS)
                             || logic.isTheoryEnabled(THEORY_UF))
                         && logic.isTheoryEnabled(THEORY_BV))
                        ||
                        // QF_AUFLIA (and may be ends up enabling
                        // QF_AUFLRA?)
                        (!logic.isQuantified()
                         && logic.isTheoryEnabled(THEORY_ARRAYS)
                         && logic.isTheoryEnabled(THEORY_UF)
                         && logic.isTheoryEnabled(THEORY_ARITH))
                        ||
                        // QF_LRA
                        (!logic.isQuantified() && logic.isPure(THEORY_ARITH)
                         && logic.isLinear() && !logic.isDifferenceLogic()
                         && !logic.areIntegersUsed())
                        ||
                        // Quantifiers
                        logic.isQuantified() ||
                        // Strings
                        logic.isTheoryEnabled(THEORY_STRINGS)
                    ? options::DecisionMode::JUSTIFICATION
                    : options::DecisionMode::INTERNAL);

  bool stoponly =
      // ALL or its supersets
      logic.hasEverything() || logic.isTheoryEnabled(THEORY_STRINGS)
          ? false
          : (  // QF_AUFLIA
                (!logic.isQuantified() && logic.isTheoryEnabled(THEORY_ARRAYS)
                 && logic.isTheoryEnabled(THEORY_UF)
                 && logic.isTheoryEnabled(THEORY_ARITH))
                        ||
                        // QF_LRA
                        (!logic.isQuantified() && logic.isPure(THEORY_ARITH)
                         && logic.isLinear() && !logic.isDifferenceLogic()
                         && !logic.areIntegersUsed())
                    ? true
                    : false);

  if (stoponly)
  {
    if (decMode == options::DecisionMode::JUSTIFICATION)
    {
      decMode = options::DecisionMode::STOPONLY;
    }
    else
    {
      Assert(decMode == options::DecisionMode::INTERNAL);
    }
  }
  SET_AND_NOTIFY_VAL_SYM(decision, decisionMode, decMode, "logic");
}

void SetDefaults::notifyModifyOption(const std::string& x,
                                     const std::string& val,
                                     const std::string& reason) const
{
  verbose(1) << "SetDefaults: setting " << x << " to " << val;
  if (!reason.empty())
  {
    verbose(1) << " due to " << reason;
  }
  verbose(1) << std::endl;
  // don't print -o options-auto for internal subsolvers
  if (!d_isInternalSubsolver)
  {
    if (isOutputOn(OutputTag::OPTIONS_AUTO))
    {
      output(OutputTag::OPTIONS_AUTO) << "(options-auto";
      output(OutputTag::OPTIONS_AUTO) << " " << x;
      output(OutputTag::OPTIONS_AUTO) << " " << val;
      if (!reason.empty())
      {
        output(OutputTag::OPTIONS_AUTO) << " :reason \"" << reason << "\"";
      }
      output(OutputTag::OPTIONS_AUTO) << ")" << std::endl;
    }
  }
}

void SetDefaults::disableChecking(Options& opts)
{
  opts.write_smt().checkUnsatCores = false;
  opts.write_smt().produceProofs = false;
  opts.write_smt().checkProofs = false;
  opts.write_smt().debugCheckModels = false;
  opts.write_smt().checkModels = false;

}

}  // namespace smt
}  // namespace ava6::internal
