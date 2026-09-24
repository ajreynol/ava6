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
  if (opts.write_##domain().optName##WasSetByUser                             \
      && opts.write_##domain().optName != value)                              \
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
  if (!opts.write_##domain().optName##WasSetByUser                          \
      && opts.write_##domain().optName != value)                            \
  {                                                                         \
    notifyModifyOption(options::domain::longName::optName, #value, reason); \
    opts.write_##domain().optName = value;                                  \
  }
/**
 * Set domain.optName to value due to reason if the option was not already set
 * by the user. Notify if value changes.
 */
#define SET_AND_NOTIFY_IF_NOT_USER_VAL_SYM(domain, optName, value, reason) \
  if (!opts.write_##domain().optName##WasSetByUser                         \
      && opts.write_##domain().optName != value)                           \
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
  // Complete proof checking and the proof-producing bit-vector backend are
  // the ordinary solver configuration.
  if (opts.smt.checkProofs && !opts.proof.checkProofsCompleteWasSetByUser
      && (!opts.proof.proofGranularityModeWasSetByUser
          || opts.proof.proofGranularityMode
                 >= options::ProofGranularityMode::DSL_REWRITE))
  {
    SET_AND_NOTIFY(proof, checkProofsComplete, true, "check-proofs");
  }
  
  // implied options
  if (opts.proof.checkProofsComplete)
  {
    SET_AND_NOTIFY(smt, checkProofs, true, "checkProofsComplete");
  }
  if (opts.smt.debugCheckModels)
  {
    SET_AND_NOTIFY(smt, checkModels, true, "debugCheckModels");
  }
  if (opts.smt.checkModels || opts.driver.dumpModels)
  {
    SET_AND_NOTIFY(smt, produceModels, true, "check or dump models");
  }
  if (opts.smt.checkModels)
  {
    SET_AND_NOTIFY(smt, produceAssignments, true, "checkModels");
  }
  // unsat cores and proofs shenanigans

  if (opts.smt.checkUnsatCores || opts.driver.dumpUnsatCores
      || opts.driver.dumpUnsatCoresLemmas || opts.smt.unsatAssumptions
      || false
      || opts.smt.unsatCoresMode != options::UnsatCoresMode::OFF)
  {
    SET_AND_NOTIFY(
        smt, produceUnsatCores, true, "option requiring unsat cores");
  }
  if (opts.smt.produceUnsatCores)
  {
    if (opts.smt.unsatCoresMode == options::UnsatCoresMode::OFF)
    {
      SET_AND_NOTIFY(smt,
                     unsatCoresMode,
                     options::UnsatCoresMode::ASSUMPTIONS,
                     "enabling unsat cores");
    }
  }
  
  if (opts.driver.dumpProofs)
  {
    // should not combine this with proof logging
    
  }
  // if check-proofs, dump-proofs, dump-unsat-cores-lemmas, or proof-mode=full,
  // then proofs being fully enabled is implied
  if (opts.smt.checkProofs || opts.driver.dumpProofs
      || opts.driver.dumpUnsatCoresLemmas
      || opts.smt.proofMode == options::ProofMode::FULL
      || opts.smt.proofMode == options::ProofMode::FULL_STRICT)
  {
    std::stringstream reasonNoProofs;
    if (incompatibleWithProofs(opts, reasonNoProofs))
    {
      std::stringstream ss;
      ss << reasonNoProofs.str() << " not supported with proofs or unsat cores";
      throw FatalOptionException(ss.str());
    }
    SET_AND_NOTIFY(smt, produceProofs, true, "option requiring proofs");
  }

  // this check assumes the user has requested *full* proofs
  if (opts.smt.produceProofs)
  {
    // if the user requested proofs, proof mode is (at least) full
    if (opts.smt.proofMode < options::ProofMode::FULL)
    {
      SET_AND_NOTIFY_IF_NOT_USER(
          smt, proofMode, options::ProofMode::FULL, "enabling proofs");
    }
    // Default granularity is DSL rewrite if we are intentionally using
    // proofs, otherwise it is MACRO (e.g. if produce unsat cores is true)
    if (!opts.proof.proofGranularityModeWasSetByUser
        && opts.proof.proofGranularityMode
               < options::ProofGranularityMode::DSL_REWRITE)
    {
      SET_AND_NOTIFY(proof,
                     proofGranularityMode,
                     options::ProofGranularityMode::DSL_REWRITE,
                     "enabling proofs");
    }
    // unsat cores are available due to proofs being enabled, as long as
    // SAT proofs are available
    if (opts.smt.unsatCoresMode != options::UnsatCoresMode::SAT_PROOF
        && opts.smt.proofMode != options::ProofMode::PP_ONLY)
    {
      SET_AND_NOTIFY(smt, produceUnsatCores, true, "enabling proofs");
      // if full proofs are available, use them for unsat cores
      SET_AND_NOTIFY(smt,
                     unsatCoresMode,
                     options::UnsatCoresMode::SAT_PROOF,
                     "enabling proofs");
    }
    // note that this test assumes that granularity modes are ordered and
    // THEORY_REWRITE is gonna be, in the enum, after the lower granularity
    // levels

  }
  if (!opts.smt.produceProofs)
  {
    if (opts.smt.proofMode != options::ProofMode::OFF)
    {
      // if (expert) user set proof mode to something other than off, enable
      // proofs
      SET_AND_NOTIFY(smt, produceProofs, true, "proof mode");
    }
    // if proofs weren't enabled by user, and we are producing difficulty

    
    // if proofs weren't enabled by user, and we are producing unsat cores
    if (opts.smt.produceUnsatCores)
    {
      SET_AND_NOTIFY(smt, produceProofs, true, "unsat cores");
      if (opts.smt.unsatCoresMode == options::UnsatCoresMode::SAT_PROOF)
      {
        // if requested to be based on proofs, we produce (preprocessing +) SAT
        // proofs
        SET_AND_NOTIFY_VAL_SYM(
            smt, proofMode, options::ProofMode::SAT, "unsat cores SAT proof");
      }
      else if (opts.smt.proofMode == options::ProofMode::OFF)
      {
        // otherwise, we always produce preprocessing proofs
        SET_AND_NOTIFY_VAL_SYM(
            smt, proofMode, options::ProofMode::PP_ONLY, "unsat cores");
      }
    }
  }
  if (opts.smt.produceProofs)
  {
    // Require complete proofs.
    if (opts.smt.proofMode == options::ProofMode::FULL)
    {
      SET_AND_NOTIFY_IF_NOT_USER(
          smt, proofMode, options::ProofMode::FULL_STRICT, "proof support");
    }
  }
  

  // if unsat cores are disabled, then unsat cores mode should be OFF. Similarly
  // for proof mode.
  Assert(opts.smt.produceUnsatCores
         == (opts.smt.unsatCoresMode != options::UnsatCoresMode::OFF));
  Assert(opts.smt.produceProofs
         == (opts.smt.proofMode != options::ProofMode::OFF));

  // if we require disabling options due to proofs, disable them now
  if (opts.smt.produceProofs)
  {
    std::stringstream reasonNoProofs;
    if (incompatibleWithProofs(opts, reasonNoProofs))
    {
      std::stringstream ss;
      ss << reasonNoProofs.str() << " not supported with proofs or unsat cores";
      throw FatalOptionException(ss.str());
    }
  }
  if (d_isInternalSubsolver)
  {
    // these options must be disabled on internal subsolvers, as they are
    // used by the user to rephrase the input.

    // deep restart does not work with internal subsolvers?
    
  }
}

void SetDefaults::finalizeLogic(LogicInfo& logic, Options& opts) const
{
  
  if (!false && logic.isQuantified()
           && (false
               || (logic.isPure(THEORY_ARITH) && !logic.isLinear()
                   && logic.areIntegersUsed()))
           && !opts.base.incrementalSolving)
  {

  }

  if (opts.bv.bitblastMode == options::BitblastMode::EAGER)
  {
    if (opts.smt.produceModels
        && (logic.isTheoryEnabled(THEORY_ARRAYS)
            || logic.isTheoryEnabled(THEORY_UF)))
    {
      if (opts.bv.bitblastModeWasSetByUser
          || opts.smt.produceModelsWasSetByUser)
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
      SET_AND_NOTIFY(smt, ackermann, true, "bit-blast eager");
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
  if (opts.smt.ackermann && opts.smt.produceModels
      && (logic.isTheoryEnabled(THEORY_ARRAYS)
          || logic.isTheoryEnabled(THEORY_UF)))
  {
    if (opts.smt.produceModelsWasSetByUser)
    {
      throw FatalOptionException(std::string(
          "Ackermannization currently does not support model generation."));
    }
    SET_AND_NOTIFY(smt, ackermann, false, "model generation");
    // we are not relying on ackermann to eliminate theories in this case
    Assert(opts.bv.bitblastMode != options::BitblastMode::EAGER);
  }

  if (opts.smt.ackermann)
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
  if (logic.isTheoryEnabled(THEORY_STRINGS)
      && !options().strings.stringExpWasSetByUser)
  {
    SET_AND_NOTIFY(strings, stringExp, true, "logic including strings");
  }
  // If strings-exp is enabled, we require quantifiers. We also enable them
  // if we are using eager string preprocessing or aggressive regular expression
  // elimination, which may introduce quantified formulas at preprocess time.
  if (opts.strings.stringExp || !true
      || false)
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
  if (d_env.hasSepHeap())
  {
    std::stringstream reasonNoSepLogic;
    if (incompatibleWithSeparationLogic(opts))
    {
      std::stringstream ss;
      ss << reasonNoSepLogic.str()
         << " not supported when using separation logic.";
      throw FatalOptionException(ss.str());
    }
  }
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
    std::stringstream reasonNoUc;
    if (incompatibleWithUnsatCores(opts, reasonNoUc))
    {
      std::stringstream ss;
      ss << reasonNoUc.str() << " not supported with unsat cores";
      throw FatalOptionException(ss.str());
    }
  }
  else
  {
    // Turn on unconstrained simplification for QF_AUFBV
    if (!opts.smt.unconstrainedSimpWasSetByUser
        && !opts.base.incrementalSolving)
    {
      // It is also currently incompatible with arithmetic, force the option
      // off.
      bool uncSimp = !opts.base.incrementalSolving && !logic.isQuantified()
                     && !opts.smt.produceModels && !opts.smt.produceAssignments
                     && !opts.smt.checkModels
                     && logic.isTheoryEnabled(THEORY_ARRAYS)
                     && logic.isTheoryEnabled(THEORY_BV)
                     && !logic.isTheoryEnabled(THEORY_ARITH);
      SET_AND_NOTIFY_VAL_SYM(
          smt, unconstrainedSimp, uncSimp, "logic and options");
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
  if (opts.smt.produceAssignments || false)
  {
    SET_AND_NOTIFY(smt, produceModels, true, "produce assignments or sygus");
  }

  // --ite-simp is an experimental option designed for QF_LIA/nec. This
  // technique is experimental. This benchmark set also requires removing ITEs
  // during preprocessing, before repeating simplification. Hence, we enable
  // this by default.


  // Set the options for the theoryOf
  if (!opts.theory.theoryOfModeWasSetByUser)
  {
    if (logic.isSharingEnabled() && !logic.isTheoryEnabled(THEORY_BV)
        && !logic.isTheoryEnabled(THEORY_STRINGS)
        && !logic.isTheoryEnabled(THEORY_SETS)
        && !false
        && !(logic.isTheoryEnabled(THEORY_ARITH) && !logic.isLinear()
             && !logic.isQuantified()))
    {
      SET_AND_NOTIFY_VAL_SYM(theory,
                             theoryOfMode,
                             options::TheoryOfMode::THEORY_OF_TERM_BASED,
                             "logic");
    }
  }

  // By default, symmetry breaker is on only for non-incremental QF_UF.
  // Note that if ufSymmetryBreaker is already set to false, we do not reenable
  // it.


  // If in arrays, set the UF handler to arrays
  if (logic.isTheoryEnabled(THEORY_ARRAYS) && !false
      && !false
      && (!logic.isQuantified()
          || (logic.isQuantified() && !logic.isTheoryEnabled(THEORY_UF))))
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
    if (!logic.isQuantified() && logic.isTheoryEnabled(THEORY_ARRAYS)
        && logic.isTheoryEnabled(THEORY_UF)
        && logic.isTheoryEnabled(THEORY_ARITH))
    {
      SET_AND_NOTIFY(arrays, arraysEagerIndexSplitting, false, "logic");
    }
  }
  // Turn on multiple-pass non-clausal simplification for QF_AUFBV
  if (!opts.smt.repeatSimpWasSetByUser)
  {
    bool repeatSimp = !logic.isQuantified()
                      && (logic.isTheoryEnabled(THEORY_ARRAYS)
                          && logic.isTheoryEnabled(THEORY_UF)
                          && logic.isTheoryEnabled(THEORY_BV))
                      && !safeUnsatCores(opts);
    SET_AND_NOTIFY_VAL_SYM(smt, repeatSimp, repeatSimp, "logic");
  }

  /* Disable bit-level propagation by default for the BITBLAST solver. */
  

  if (opts.bv.boolToBitvector == options::BoolToBVMode::ALL
      && !logic.isTheoryEnabled(THEORY_BV))
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
  if (!opts.arith.arithHeuristicPivotsWasSetByUser)
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
    SET_AND_NOTIFY_VAL_SYM(
        arith, arithHeuristicPivots, heuristicPivots, "logic");
  }
  if (!opts.arith.arithPivotThresholdWasSetByUser)
  {
    uint16_t pivotThreshold = 2;
    if (logic.isPure(THEORY_ARITH) && !logic.isQuantified())
    {
      if (logic.isDifferenceLogic())
      {
        pivotThreshold = 16;
      }
    }
    SET_AND_NOTIFY_VAL_SYM(arith, arithPivotThreshold, pivotThreshold, "logic");
  }
  if (!opts.arith.arithStandardCheckVarOrderPivotsWasSetByUser)
  {
    int16_t varOrderPivots = -1;
    if (logic.isPure(THEORY_ARITH) && !logic.isQuantified())
    {
      varOrderPivots = 200;
    }
    SET_AND_NOTIFY_VAL_SYM(
        arith, arithStandardCheckVarOrderPivots, varOrderPivots, "logic");
  }
  // DIO solver typically makes things worse for quantifier-free logics with
  // non-linear arithmetic.
  if (!logic.isQuantified() && logic.isTheoryEnabled(THEORY_ARITH)
      && !logic.isLinear() && !opts.arith.arithDioSolverWasSetByUser)
  {
    SET_AND_NOTIFY(
        arith, arithDioSolver, false, "quantifier-free non-linear logic");
  }
  if (logic.isPure(THEORY_ARITH) && !logic.areRealsUsed())
  {
    SET_AND_NOTIFY(
        arith, nlExtTangentPlanesInterleave, true, "pure integer logic");
  }
  if (!opts.arith.nlRlvAssertBoundsWasSetByUser)
  {
    bool val = !logic.isQuantified();
    // use bound inference to determine when bounds are irrelevant only when
    // the logic is quantifier-free
    SET_AND_NOTIFY_VAL_SYM(
        arith, nlRlvAssertBounds, val, "non-quantified logic");
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
  if (opts.smt.produceModels || opts.smt.produceAssignments
      || opts.smt.checkModels)
  {
    SET_AND_NOTIFY(arrays, arraysOptimizeLinear, false, "models");
  }


  // !!! All options that require disabling models go here
  std::stringstream reasonNoModel;
  if (incompatibleWithModels(opts, reasonNoModel))
  {
    std::string sOptNoModel = reasonNoModel.str();
    if (opts.smt.produceModels)
    {
      if (opts.smt.produceModelsWasSetByUser)
      {
        std::stringstream ss;
        ss << "Cannot use " << sOptNoModel << " with model generation.";
        throw FatalOptionException(ss.str());
      }
      SET_AND_NOTIFY(smt, produceModels, false, sOptNoModel);
    }
    if (opts.smt.produceAssignments)
    {
      if (opts.smt.produceAssignmentsWasSetByUser)
      {
        std::stringstream ss;
        ss << "Cannot use " << sOptNoModel
           << " with model generation (produce-assignments).";
        throw FatalOptionException(ss.str());
      }
      SET_AND_NOTIFY(smt, produceAssignments, false, sOptNoModel);
    }
    if (opts.smt.checkModels)
    {
      if (opts.smt.checkModelsWasSetByUser)
      {
        std::stringstream ss;
        ss << "Cannot use " << sOptNoModel
           << " with model generation (check-models).";
        throw FatalOptionException(ss.str());
      }
      SET_AND_NOTIFY(smt, checkModels, false, sOptNoModel);
    }
  }

  if (opts.bv.bitblastMode == options::BitblastMode::EAGER
      && !logic.isPure(THEORY_BV) && logic.getLogicString() != "QF_UFBV")
  {
    throw FatalOptionException(
        "Eager bit-blasting does not currently support theory combination with "
        "any theory other than UF. ");
  }

  // Note that if nlCov is already set to false, we do not reenable it.

  if (logic.isTheoryEnabled(theory::THEORY_ARITH)
      && logic.areTranscendentalsUsed())
  {
    SET_AND_NOTIFY_IF_NOT_USER_VAL_SYM(
        arith, nlExt, options::NlExtMode::FULL, "logic with transcendentals");
  }
  if (isOutputOn(OutputTag::NORMALIZE))
  {
    SET_AND_NOTIFY(base, preprocessOnly, true, "normalize output");
  }
  if (logic.isQuantified())
  {
    SET_AND_NOTIFY_IF_NOT_USER(
        arith,
        nlExtInitialSignLemmas,
        false,
        "Preemptive lemmas for incremental linearization are disabled "
        "when the logic has quantifiers");
  }
}


bool SetDefaults::usesInputConversion(const Options& opts,
                                      std::ostream& reason) const
{
  
  
  if (opts.smt.solveRealAsInt)
  {
    reason << "solveRealAsInt";
    return true;
  }
  return false;
}

bool SetDefaults::incompatibleWithProofs(Options& opts,
                                         std::ostream& reason) const
{

  
  bool isFullPf = (opts.smt.proofMode == options::ProofMode::FULL
                   || opts.smt.proofMode == options::ProofMode::FULL_STRICT);

  // options that are automatically set to support proofs
  
  // If proofs are required and the user did not specify a specific BV solver,
  // we make sure to use the proof producing BITBLAST_INTERNAL solver.
  if (isFullPf)
  {
    // this is always set by safe options, ok to silently change
    
  }


  // specific to SAT solver
  
  
  if (opts.smt.proofMode == options::ProofMode::FULL_STRICT)
  {
    // these are always disabled by safe options, ok to silently change
    // symmetry breaking does not have proof support

    // CEGQI with deltas and infinities is not supported
    
    
    
    // this is an expert option, ok to silently change
    // shared selectors are not supported
    
  }
  return false;
}

bool SetDefaults::incompatibleWithModels(const Options& opts,
                                         std::ostream& reason) const
{
  if (opts.smt.unconstrainedSimpWasSetByUser && opts.smt.unconstrainedSimp)
  {
    reason << "unconstrained-simp";
    return true;
  }
  

  return false;
}

bool SetDefaults::incompatibleWithIncremental(const LogicInfo& logic,
                                              Options& opts,
                                              std::ostream& reason,
                                              std::ostream& suggest) const
{
  if (d_env.hasSepHeap())
  {
    reason << "separation logic";
    return true;
  }
  if (opts.smt.ackermann)
  {
    reason << "ackermann";
    return true;
  }
  if (opts.smt.unconstrainedSimp)
  {
    if (opts.smt.unconstrainedSimpWasSetByUser)
    {
      reason << "unconstrained simplification";
      return true;
    }
    SET_AND_NOTIFY(smt, unconstrainedSimp, false, "incremental solving");
  }
  if (opts.bv.bitblastMode == options::BitblastMode::EAGER
      && !logic.isPure(THEORY_BV))
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

bool SetDefaults::incompatibleWithUnsatCores(Options& opts,
                                             std::ostream& reason) const
{
  // All techniques that are incompatible with unsat cores are listed here.
  // A preprocessing pass is incompatible with unsat cores if
  // (A) its reasoning is not local, i.e. it may replace an assertion A by A'
  // where A does not imply A', or if it adds new assertions B that are not
  // tautologies, AND
  // (B) it does not track proofs.

  

  

  


  return false;
}

bool SetDefaults::safeUnsatCores(const Options& opts) const
{
  // whether we want to force safe unsat cores, i.e., if we are in the default
  // ASSUMPTIONS mode, since other ones are experimental
  return opts.smt.unsatCoresMode == options::UnsatCoresMode::ASSUMPTIONS;
}

bool SetDefaults::incompatibleWithSygus(const Options& opts,
                                        std::ostream& reason) const
{
  // sygus should not be combined with preprocessing passes that convert the
  // input
  if (usesInputConversion(opts, reason))
  {
    return true;
  }

  
  return false;
}

bool SetDefaults::incompatibleWithQuantifiers(const Options& opts,
                                              std::ostream& reason) const
{
  if (opts.smt.ackermann)
  {
    reason << "ackermann";
    return true;
  }
  
  return false;
}

bool SetDefaults::incompatibleWithSeparationLogic(Options& opts) const
{
  // Spatial formulas in separation logic have a semantics that depends on
  // their position in the AST (e.g. their nesting beneath separation
  // conjunctions). Thus, we cannot apply BCP as a substitution for spatial
  // predicates to the input formula. We disable this option altogether to
  // ensure this is the case
  
  return false;
}

void SetDefaults::widenLogic(LogicInfo& logic, const Options& opts) const
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
  
  if (opts.quantifiers.preSkolemQuantNested
      && opts.quantifiers.preSkolemQuantNestedWasSetByUser)
  {
    // if pre-skolem nested is explictly set, then we require UF. If it is
    // not explicitly set, it is disabled below if UF is not present.
    verbose(1) << "Enabling UF because preSkolemQuantNested requires it."
               << std::endl;
    needsUf = true;
  }
  if (needsUf
      // Arrays, datatypes and sets permit Boolean terms and thus require UF
      || logic.isTheoryEnabled(THEORY_ARRAYS)
      || logic.isTheoryEnabled(THEORY_DATATYPES)
      || logic.isTheoryEnabled(THEORY_SETS)
      || false
      // Non-linear arithmetic requires UF to deal with division/mod because
      // their expansion introduces UFs for the division/mod-by-zero case.
      // If we are eliminating non-linear arithmetic via solve-int-as-bv,
      // then this is not required, since non-linear arithmetic will be
      // eliminated altogether (or otherwise fail at preprocessing).
      || (logic.isTheoryEnabled(THEORY_ARITH) && !logic.isLinear()
          && true)
      // If arithmetic and bv are enabled, it is possible to use bv2nat and
      // int2bv, which require the UF theory.
      || (logic.isTheoryEnabled(THEORY_ARITH)
          && logic.isTheoryEnabled(THEORY_BV))
      // FP requires UF since there are multiple operators that are partially
      // defined (see http://smt-lib.org/papers/BTRW15.pdf for more
      // details).
      || false)
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
    SET_AND_NOTIFY_IF_NOT_USER_VAL_SYM(
        quantifiers, fmfMbqiMode, options::FmfMbqiMode::NONE, "fmfBound");
    SET_AND_NOTIFY_IF_NOT_USER_VAL_SYM(
        quantifiers, prenexQuant, options::PrenexQuantMode::NONE, "fmfBound");
  }

  


  // now, have determined whether finite model find is on/off
  // apply finite model finding options


  // apply sygus options
  // if we are attempting to rewrite everything to SyGuS, use sygus()

  // counterexample-guided instantiation for non-sygus
  // enable if any possible quantifiers with arithmetic, datatypes or bitvectors
  if ((logic.isQuantified()
       && (logic.isTheoryEnabled(THEORY_ARITH)
           || logic.isTheoryEnabled(THEORY_DATATYPES)
           || logic.isTheoryEnabled(THEORY_BV)
           || false))
      || false)
  {
    SET_AND_NOTIFY_IF_NOT_USER(quantifiers, cegqi, true, "logic");
    // check whether we should apply full cbqi
    if (logic.isPure(THEORY_BV))
    {
      SET_AND_NOTIFY_IF_NOT_USER(
          quantifiers, cegqiFullEffort, true, "pure BV logic");
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
  if (opts.quantifiers.cbqiModeWasSetByUser || false)
  {
    SET_AND_NOTIFY(quantifiers, conflictBasedInst, true, "cbqi option");
  }
  // for induction techniques

  
  

  // can't pre-skolemize nested quantifiers without UF theory
  if (!logic.isTheoryEnabled(THEORY_UF)
      && opts.quantifiers.preSkolemQuant != options::PreSkolemQuantMode::OFF)
  {
    SET_AND_NOTIFY_IF_NOT_USER(
        quantifiers, preSkolemQuantNested, false, "preSkolemQuant");
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
