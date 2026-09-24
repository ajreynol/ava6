/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Abstract management of models for TheoryEngine.
 */

#include "ava6_private.h"

#ifndef AVA6__THEORY__MODEL_MANAGER__H
#define AVA6__THEORY__MODEL_MANAGER__H

#include <memory>

#include "smt/env_obj.h"
#include "theory/uf/equality_engine.h"
#include "theory/logic_info.h"

namespace ava6::internal {

class TheoryEngine;
class Env;

namespace theory {

class TheoryEngineModelBuilder;
class TheoryModel;

/** Owns the model and its equality engine, and coordinates model construction. */
class ModelManager : protected EnvObj
{
 public:
  ModelManager(Env& env, TheoryEngine& te);
  ~ModelManager();
  /** Allocate the model builder and the model's equality engine. */
  void finishInit();
  /** Reset model, called during full effort check before the model is built */
  void resetModel();
  /**
   * Build the model. If we have yet to build the model on this round, this
   * method calls prepareModel and then calls
   * finishBuildModel.
   *
   * @return true if model building was successful.
   */
  bool buildModel();
  /**
   * Have we called buildModel this round? Note this returns true whether or
   * not the model building was successful.
   */
  bool isModelBuilt() const;
  /**
   * Post process model, which is used as a way of each theory adding additional
   * information to the model after successfully building a model.
   */
  void postProcessModel(bool incomplete);
  /** Get a pointer to model object maintained by this class. */
  TheoryModel* getModel();
  //------------------------ finer grained control over model building
  /**
   * Prepare the model by setting up the
   * equality engine of the model. This should assert all relevant information
   * about the model into the equality engine of d_model.
   *
   * @return true if we are in conflict (i.e. the equality engine of the model
   * equality engine is inconsistent).
   */
  bool prepareModel();
  /**
   * Finish build model, which calls the theory model builder to assign values
   * to all equivalence classes. This should be run after prepareModel.
   *
   * @return true if model building was successful.
   */
  bool finishBuildModel() const;
  //------------------------ end finer grained control over model building
 private:
  /**
   * Collect model Boolean variables.
   * This asserts the values of all boolean variables to the equality engine of
   * the model, based on their value in the prop engine.
   *
   * @return true if we are in conflict.
   */
  bool collectModelBooleanVariables();

  /** Reference to the theory engine */
  TheoryEngine& d_te;
  /**
   * A dummy context for the model equality engine, so we can clear it
   * independently of search context.
   */
  context::Context d_modelEeContext;
  /** Independently resettable equality engine used for model construction. */
  std::unique_ptr<eq::EqualityEngine> d_modelEqualityEngine;
  /** The model object we have allocated (if one exists) */
  std::unique_ptr<TheoryModel> d_model;
  /** The model builder object we are using */
  TheoryEngineModelBuilder* d_modelBuilder;
  /** The model builder object we have allocated (if one exists) */
  std::unique_ptr<TheoryEngineModelBuilder> d_alocModelBuilder;
  /** whether we have tried to build this model in the current context */
  bool d_modelBuilt;
  /** whether this model has been built successfully */
  bool d_modelBuiltSuccess;
};

}  // namespace theory
}  // namespace ava6::internal

#endif /* AVA6__THEORY__MODEL_MANAGER__H */
