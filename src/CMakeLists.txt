###############################################################################
# This file is part of the cvc5 project.
#
# Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
# in the top-level source directory and their institutional affiliations.
# All rights reserved.  See the file COPYING in the top-level source
# directory for licensing information.
# #############################################################################
#
# The build system configuration.
##

# Collect libava6 source files
libava6_add_sources(
  api/cpp/ava6.cpp
  api/cpp/ava6_checks.h
  api/cpp/ava6_types.cpp
  api/cpp/ava6_skolem_id.cpp
  decision/assertion_list.cpp
  decision/assertion_list.h
  decision/decision_engine.cpp
  decision/decision_engine.h
  decision/justification_strategy.cpp
  decision/justification_strategy.h
  decision/justify_cache.cpp
  decision/justify_cache.h
  decision/justify_info.cpp
  decision/justify_info.h
  decision/justify_stack.cpp
  decision/justify_stack.h
  decision/justify_stats.cpp
  decision/justify_stats.h
  lib/clock_gettime.c
  lib/clock_gettime.h
  lib/ffs.c
  lib/ffs.h
  lib/replacements.h
  lib/strtok_r.c
  lib/strtok_r.h
  options/language.cpp
  options/language.h
  options/managed_streams.cpp
  options/managed_streams.h
  options/option_exception.cpp
  options/option_exception.h
  options/options_handler.cpp
  options/options_handler.h
  options/options_listener.h
  options/options_public.h
  preprocessing/assertion_pipeline.cpp
  preprocessing/assertion_pipeline.h
  preprocessing/learned_literal_manager.cpp
  preprocessing/learned_literal_manager.h
  preprocessing/passes/ackermann.cpp
  preprocessing/passes/ackermann.h
  preprocessing/passes/apply_substs.cpp
  preprocessing/passes/apply_substs.h
  preprocessing/passes/bool_to_bv.cpp
  preprocessing/passes/bool_to_bv.h
  preprocessing/passes/bv_eager_atoms.cpp
  preprocessing/passes/bv_eager_atoms.h
  preprocessing/passes/bv_intro_pow2.cpp
  preprocessing/passes/bv_intro_pow2.h
  preprocessing/passes/bv_to_bool.cpp
  preprocessing/passes/bv_to_bool.h
  preprocessing/passes/bv_to_int.cpp
  preprocessing/passes/bv_to_int.h
  preprocessing/passes/distinct_elim.cpp
  preprocessing/passes/distinct_elim.h
  preprocessing/passes/extended_rewriter_pass.cpp
  preprocessing/passes/extended_rewriter_pass.h
  preprocessing/passes/foreign_theory_rewrite.cpp
  preprocessing/passes/foreign_theory_rewrite.h
  preprocessing/passes/global_negate.cpp
  preprocessing/passes/global_negate.h
  preprocessing/passes/ho_elim.cpp
  preprocessing/passes/ho_elim.h
  preprocessing/passes/int_to_bv.cpp
  preprocessing/passes/int_to_bv.h
  preprocessing/passes/ite_removal.cpp
  preprocessing/passes/ite_removal.h
  preprocessing/passes/ite_simp.cpp
  preprocessing/passes/ite_simp.h
  preprocessing/passes/learned_rewrite.cpp
  preprocessing/passes/learned_rewrite.h
  preprocessing/passes/miplib_trick.cpp
  preprocessing/passes/miplib_trick.h
  preprocessing/passes/nl_ext_purify.cpp
  preprocessing/passes/nl_ext_purify.h
  preprocessing/passes/non_clausal_simp.cpp
  preprocessing/passes/non_clausal_simp.h
  preprocessing/passes/pseudo_boolean_processor.cpp
  preprocessing/passes/pseudo_boolean_processor.h
  preprocessing/passes/quantifiers_preprocess.cpp
  preprocessing/passes/quantifiers_preprocess.h
  preprocessing/passes/real_to_int.cpp
  preprocessing/passes/real_to_int.h
  preprocessing/passes/rewrite.cpp
  preprocessing/passes/rewrite.h
  preprocessing/passes/sort_infer.cpp
  preprocessing/passes/sort_infer.h
  preprocessing/passes/static_learning.cpp
  preprocessing/passes/static_learning.h
  preprocessing/passes/static_rewrite.cpp
  preprocessing/passes/static_rewrite.h
  preprocessing/passes/strings_eager_pp.cpp
  preprocessing/passes/strings_eager_pp.h
  preprocessing/passes/theory_preprocess.cpp
  preprocessing/passes/theory_preprocess.h
  preprocessing/passes/unconstrained_simplifier.cpp
  preprocessing/passes/unconstrained_simplifier.h
  preprocessing/passes/normalize.cpp
  preprocessing/passes/normalize.h
  preprocessing/preprocessing_pass.cpp
  preprocessing/preprocessing_pass.h
  preprocessing/preprocessing_pass_context.cpp
  preprocessing/preprocessing_pass_context.h
  preprocessing/preprocessing_pass_registry.cpp
  preprocessing/preprocessing_pass_registry.h
  preprocessing/util/boolean_simplification.cpp
  preprocessing/util/boolean_simplification.h
  preprocessing/util/ite_utilities.cpp
  preprocessing/util/ite_utilities.h
  printer/ast/ast_printer.cpp
  printer/ast/ast_printer.h
  printer/enum_to_string.cpp
  printer/enum_to_string.h
  printer/let_binding.cpp
  printer/let_binding.h
  printer/printer.cpp
  printer/printer.h
  printer/smt2/smt2_printer.cpp
  printer/smt2/smt2_printer.h
  proof/eo/eo_dependent_type_converter.cpp
  proof/eo/eo_dependent_type_converter.h
  proof/eo/eo_list_node_converter.cpp
  proof/eo/eo_list_node_converter.h
  proof/eo/eo_node_converter.cpp
  proof/eo/eo_node_converter.h
  proof/eo/eo_print_channel.cpp
  proof/eo/eo_print_channel.h
  proof/eo/eo_printer.cpp
  proof/eo/eo_printer.h
  proof/assumption_proof_generator.cpp
  proof/assumption_proof_generator.h
  proof/buffered_proof_generator.cpp
  proof/buffered_proof_generator.h
  proof/conv_proof_generator.cpp
  proof/conv_proof_generator.h
  proof/conv_seq_proof_generator.cpp
  proof/conv_seq_proof_generator.h
  proof/clause_id.h
  proof/eager_proof_generator.cpp
  proof/eager_proof_generator.h
  proof/lazy_proof.cpp
  proof/lazy_proof.h
  proof/lazy_proof_chain.cpp
  proof/lazy_proof_chain.h
  proof/lazy_tree_proof_generator.cpp
  proof/lazy_tree_proof_generator.h
  proof/method_id.cpp
  proof/method_id.h
  proof/print_expr.cpp
  proof/print_expr.h
  proof/proof.cpp
  proof/proof.h
  proof/proof_checker.cpp
  proof/proof_checker.h
  proof/proof_ensure_closed.cpp
  proof/proof_ensure_closed.h
  proof/proof_generator.cpp
  proof/proof_generator.h
  proof/proof_letify.cpp
  proof/proof_letify.h
  proof/proof_node.cpp
  proof/proof_node.h
  proof/proof_node_algorithm.cpp
  proof/proof_node_algorithm.h
  proof/proof_node_converter.cpp
  proof/proof_node_converter.h
  proof/proof_node_to_sexpr.cpp
  proof/proof_node_to_sexpr.h
  proof/proof_node_manager.cpp
  proof/proof_node_manager.h
  proof/proof_node_updater.cpp
  proof/proof_node_updater.h
  proof/proof_rule_checker.cpp
  proof/proof_rule_checker.h
  proof/proof_set.h
  proof/proof_step_buffer.cpp
  proof/proof_step_buffer.h
  proof/resolution_proofs_util.cpp
  proof/resolution_proofs_util.h
  proof/rewrite_proof_generator.cpp
  proof/rewrite_proof_generator.h
  proof/subtype_elim_proof_converter.cpp
  proof/subtype_elim_proof_converter.h
  proof/trust_id.cpp
  proof/trust_id.h
  proof/trust_node.cpp
  proof/trust_node.h
  proof/trust_proof_generator.cpp
  proof/trust_proof_generator.h
  proof/theory_proof_step_buffer.cpp
  proof/theory_proof_step_buffer.h
  proof/unsat_core.cpp
  proof/unsat_core.h
  proof/valid_witness_proof_generator.cpp
  proof/valid_witness_proof_generator.h
  prop/cadical/cadical.cpp
  prop/cadical/cadical.h
  prop/cadical/cdclt_propagator.cpp
  prop/cadical/cdclt_propagator.h
  prop/cadical/proof_tracer.cpp
  prop/cadical/proof_tracer.h
  prop/cadical/util.cpp
  prop/cadical/util.h
  prop/cnf_stream.cpp
  prop/cnf_stream.h
  prop/learned_db.cpp
  prop/learned_db.h
  prop/learned_literal_type.h
  prop/lemma_inprocess.cpp
  prop/lemma_inprocess.h
  prop/minisat/core/Dimacs.h
  prop/minisat/core/Solver.cc
  prop/minisat/core/Solver.h
  prop/minisat/core/SolverTypes.h
  prop/minisat/minisat.cpp
  prop/minisat/minisat.h
  prop/minisat/mtl/Alg.h
  prop/minisat/mtl/Alloc.h
  prop/minisat/mtl/Heap.h
  prop/minisat/mtl/IntTypes.h
  prop/minisat/mtl/Map.h
  prop/minisat/mtl/Queue.h
  prop/minisat/mtl/Sort.h
  prop/minisat/mtl/Vec.h
  prop/minisat/mtl/XAlloc.h
  prop/minisat/opt_clauses_manager.cpp
  prop/minisat/opt_clauses_manager.h
  prop/minisat/sat_proof_manager.cpp
  prop/minisat/sat_proof_manager.h
  prop/minisat/simp/SimpSolver.cc
  prop/minisat/simp/SimpSolver.h
  prop/minisat/utils/Options.h
  prop/proof_cnf_stream.cpp
  prop/proof_cnf_stream.h
  prop/proof_post_processor.cpp
  prop/proof_post_processor.h
  prop/prop_engine.cpp
  prop/prop_engine.h
  prop/prop_proof_manager.cpp
  prop/prop_proof_manager.h
  prop/registrar.h
  prop/sat_solver.h
  prop/sat_solver_factory.cpp
  prop/sat_solver_factory.h
  prop/sat_solver_types.cpp
  prop/sat_solver_types.h
  prop/skolem_def_manager.cpp
  prop/skolem_def_manager.h
  prop/theory_preregistrar.cpp
  prop/theory_preregistrar.h
  prop/theory_proxy.cpp
  prop/theory_proxy.h
  prop/zero_level_learner.cpp
  prop/zero_level_learner.h
  smt/assertions.cpp
  smt/assertions.h
  smt/check_models.cpp
  smt/check_models.h
  smt/context_manager.cpp
  smt/context_manager.h
  smt/env.cpp
  smt/env.h
  smt/env_obj.cpp
  smt/env_obj.h
  smt/expand_definitions.cpp
  smt/expand_definitions.h
  smt/listeners.cpp
  smt/listeners.h
  smt/logic_exception.h
  smt/model.cpp
  smt/model.h
  smt/model_core_builder.cpp
  smt/model_core_builder.h
  smt/preprocessor.cpp
  smt/preprocessor.h
  smt/preprocess_proof_generator.cpp
  smt/preprocess_proof_generator.h
  smt/print_benchmark.cpp
  smt/print_benchmark.h
  smt/process_assertions.cpp
  smt/process_assertions.h
  smt/proof_manager.cpp
  smt/proof_manager.h
  smt/proof_final_callback.cpp
  smt/proof_final_callback.h
  smt/proof_logger.cpp
  smt/proof_logger.h
  smt/proof_post_processor.cpp
  smt/proof_post_processor.h
  smt/proof_post_processor_dsl.cpp
  smt/proof_post_processor_dsl.h
  smt/set_defaults.cpp
  smt/set_defaults.h
  smt/solver_engine.cpp
  smt/solver_engine.h
  smt/solver_engine_state.cpp
  smt/solver_engine_state.h
  smt/solver_engine_stats.cpp
  smt/solver_engine_stats.h
  smt/smt_driver.cpp
  smt/smt_driver.h
  smt/smt_mode.cpp
  smt/smt_mode.h
  smt/smt_solver.cpp
  smt/smt_solver.h
  smt/term_formula_removal.cpp
  smt/term_formula_removal.h
  smt/unsat_core_manager.cpp
  smt/unsat_core_manager.h
  smt/witness_form.cpp
  smt/witness_form.h
  theory/arith/arith_evaluator.cpp
  theory/arith/arith_evaluator.h
  theory/arith/arith_ite_utils.cpp
  theory/arith/arith_ite_utils.h
  theory/arith/arith_msum.cpp
  theory/arith/arith_msum.h
  theory/arith/arith_poly_norm.cpp
  theory/arith/arith_poly_norm.h
  theory/arith/arith_preprocess.cpp
  theory/arith/arith_preprocess.h
  theory/arith/arith_proof_rcons.cpp
  theory/arith/arith_proof_rcons.h
  theory/arith/arith_proof_utilities.cpp
  theory/arith/arith_proof_utilities.h
  theory/arith/arith_rewriter.cpp
  theory/arith/arith_rewriter.h
  theory/arith/arith_subs.cpp
  theory/arith/arith_subs.h
  theory/arith/arith_utilities.cpp
  theory/arith/arith_utilities.h
  theory/arith/bound_inference.cpp
  theory/arith/bound_inference.h
  theory/arith/branch_and_bound.cpp
  theory/arith/branch_and_bound.h
  theory/arith/delta_rational.cpp
  theory/arith/delta_rational.h
  theory/arith/equality_solver.cpp
  theory/arith/equality_solver.h
  theory/arith/inference_manager.cpp
  theory/arith/inference_manager.h
  theory/arith/linear/approx_simplex.cpp
  theory/arith/linear/approx_simplex.h
  theory/arith/linear/arith_static_learner.cpp
  theory/arith/linear/arith_static_learner.h
  theory/arith/linear/arithvar.cpp
  theory/arith/linear/arithvar.h
  theory/arith/linear/attempt_solution_simplex.cpp
  theory/arith/linear/attempt_solution_simplex.h
  theory/arith/linear/bound_counts.h
  theory/arith/linear/callbacks.cpp
  theory/arith/linear/callbacks.h
  theory/arith/linear/congruence_manager.cpp
  theory/arith/linear/congruence_manager.h
  theory/arith/linear/constraint.cpp
  theory/arith/linear/constraint.h
  theory/arith/linear/constraint_forward.h
  theory/arith/linear/cut_log.cpp
  theory/arith/linear/cut_log.h
  theory/arith/linear/dio_solver.cpp
  theory/arith/linear/dio_solver.h
  theory/arith/linear/dual_simplex.cpp
  theory/arith/linear/dual_simplex.h
  theory/arith/linear/error_set.cpp
  theory/arith/linear/error_set.h
  theory/arith/linear/fc_simplex.cpp
  theory/arith/linear/fc_simplex.h
  theory/arith/linear/infer_bounds.cpp
  theory/arith/linear/infer_bounds.h
  theory/arith/linear/linear_solver.cpp
  theory/arith/linear/linear_solver.h
  theory/arith/linear/linear_equality.cpp
  theory/arith/linear/linear_equality.h
  theory/arith/linear/matrix.cpp
  theory/arith/linear/matrix.h
  theory/arith/linear/normal_form.cpp
  theory/arith/linear/normal_form.h
  theory/arith/linear/partial_model.cpp
  theory/arith/linear/partial_model.h
  theory/arith/linear/simplex.cpp
  theory/arith/linear/simplex.h
  theory/arith/linear/simplex_update.cpp
  theory/arith/linear/simplex_update.h
  theory/arith/linear/soi_simplex.cpp
  theory/arith/linear/soi_simplex.h
  theory/arith/linear/tableau.cpp
  theory/arith/linear/tableau.h
  theory/arith/linear/tableau_sizes.cpp
  theory/arith/linear/tableau_sizes.h
  theory/arith/linear/theory_arith_private.cpp
  theory/arith/linear/theory_arith_private.h
  theory/arith/nl/equality_substitution.cpp
  theory/arith/nl/equality_substitution.h
  theory/arith/nl/ext/arith_nl_compare_proof_gen.cpp
  theory/arith/nl/ext/arith_nl_compare_proof_gen.h
  theory/arith/nl/ext/constraint.cpp
  theory/arith/nl/ext/constraint.h
  theory/arith/nl/ext/factoring_check.cpp
  theory/arith/nl/ext/factoring_check.h
  theory/arith/nl/ext/flatten_monomial_check.cpp
  theory/arith/nl/ext/flatten_monomial_check.h
  theory/arith/nl/ext/monomial.cpp
  theory/arith/nl/ext/monomial.h
  theory/arith/nl/ext/monomial_bounds_check.cpp
  theory/arith/nl/ext/monomial_bounds_check.h
  theory/arith/nl/ext/monomial_check.cpp
  theory/arith/nl/ext/monomial_check.h
  theory/arith/nl/ext/ext_state.cpp
  theory/arith/nl/ext/ext_state.h
  theory/arith/nl/ext/proof_checker.cpp
  theory/arith/nl/ext/proof_checker.h
  theory/arith/nl/ext/split_zero_check.cpp
  theory/arith/nl/ext/split_zero_check.h
  theory/arith/nl/ext/tangent_plane_check.cpp
  theory/arith/nl/ext/tangent_plane_check.h
  theory/arith/nl/ext_theory_callback.cpp
  theory/arith/nl/ext_theory_callback.h
  theory/arith/nl/iand_utils.cpp
  theory/arith/nl/iand_utils.h
  theory/arith/nl/nl_lemma_utils.cpp
  theory/arith/nl/nl_lemma_utils.h
  theory/arith/nl/nl_model.cpp
  theory/arith/nl/nl_model.h
  theory/arith/nl/nonlinear_extension.cpp
  theory/arith/nl/nonlinear_extension.h
  theory/arith/nl/stats.cpp
  theory/arith/nl/stats.h
  theory/arith/nl/strategy.cpp
  theory/arith/nl/strategy.h
  theory/arith/operator_elim.cpp
  theory/arith/operator_elim.h
  theory/arith/pp_rewrite_eq.cpp
  theory/arith/pp_rewrite_eq.h
  theory/arith/proof_checker.cpp
  theory/arith/proof_checker.h
  theory/arith/rewriter/addition.cpp
  theory/arith/rewriter/addition.h
  theory/arith/rewriter/node_utils.cpp
  theory/arith/rewriter/node_utils.h
  theory/arith/rewriter/ordering.h
  theory/arith/rewriter/rewrite_atom.cpp
  theory/arith/rewriter/rewrite_atom.h
  theory/arith/rewrites.cpp
  theory/arith/rewrites.h
  theory/arith/theory_arith.cpp
  theory/arith/theory_arith.h
  theory/arith/theory_arith_type_rules.cpp
  theory/arith/theory_arith_type_rules.h
  theory/arith/type_enumerator.h
  theory/arrays/array_info.cpp
  theory/arrays/array_info.h
  theory/arrays/inference_manager.cpp
  theory/arrays/inference_manager.h
  theory/arrays/proof_checker.cpp
  theory/arrays/proof_checker.h
  theory/arrays/skolem_cache.cpp
  theory/arrays/skolem_cache.h
  theory/arrays/theory_arrays.cpp
  theory/arrays/theory_arrays.h
  theory/arrays/theory_arrays_rewriter.cpp
  theory/arrays/theory_arrays_rewriter.h
  theory/arrays/theory_arrays_type_rules.cpp
  theory/arrays/theory_arrays_type_rules.h
  theory/arrays/type_enumerator.h
  theory/arrays/type_enumerator.cpp
  theory/assertion.cpp
  theory/assertion.h
  theory/atom_requests.cpp
  theory/atom_requests.h
  theory/booleans/circuit_propagator.cpp
  theory/booleans/circuit_propagator.h
  theory/booleans/proof_circuit_propagator.cpp
  theory/booleans/proof_circuit_propagator.h
  theory/booleans/proof_checker.cpp
  theory/booleans/proof_checker.h
  theory/booleans/theory_bool.cpp
  theory/booleans/theory_bool.h
  theory/booleans/theory_bool_rewriter.cpp
  theory/booleans/theory_bool_rewriter.h
  theory/booleans/theory_bool_type_rules.cpp
  theory/booleans/theory_bool_type_rules.h
  theory/booleans/type_enumerator.h
  theory/builtin/abstract_type.cpp
  theory/builtin/abstract_type.h
  theory/builtin/generic_op.cpp
  theory/builtin/generic_op.h
  theory/builtin/proof_checker.cpp
  theory/builtin/proof_checker.h
  theory/builtin/theory_builtin.cpp
  theory/builtin/theory_builtin.h
  theory/builtin/theory_builtin_rewriter.cpp
  theory/builtin/theory_builtin_rewriter.h
  theory/builtin/theory_builtin_type_rules.cpp
  theory/builtin/theory_builtin_type_rules.h
  theory/builtin/type_enumerator.cpp
  theory/builtin/type_enumerator.h
  theory/bv/bitblast/bitblast_proof_generator.cpp
  theory/bv/bitblast/bitblast_proof_generator.h
  theory/bv/bitblast/bitblast_strategies_template.h
  theory/bv/bitblast/bitblast_utils.h
  theory/bv/bitblast/bitblaster.h
  theory/bv/bitblast/node_bitblaster.cpp
  theory/bv/bitblast/node_bitblaster.h
  theory/bv/bitblast/proof_bitblaster.cpp
  theory/bv/bitblast/proof_bitblaster.h
  theory/bv/bv_pp_assert.cpp
  theory/bv/bv_pp_assert.h
  theory/bv/bv_solver.h
  theory/bv/bv_solver_bitblast_internal.cpp
  theory/bv/bv_solver_bitblast_internal.h
  theory/bv/int_blaster.cpp
  theory/bv/int_blaster.h
  theory/bv/macro_rewrite_elaborator.cpp
  theory/bv/macro_rewrite_elaborator.h
  theory/bv/proof_checker.cpp
  theory/bv/proof_checker.h
  theory/bv/theory_bv.cpp
  theory/bv/theory_bv.h
  theory/bv/theory_bv_rewrite_rules.h
  theory/bv/theory_bv_rewrite_rules_constant_evaluation.h
  theory/bv/theory_bv_rewrite_rules_core.h
  theory/bv/theory_bv_rewrite_rules_normalization.h
  theory/bv/theory_bv_rewrite_rules_operator_elimination.h
  theory/bv/theory_bv_rewrite_rules_simplification.h
  theory/bv/theory_bv_rewriter.cpp
  theory/bv/theory_bv_rewriter.h
  theory/bv/theory_bv_type_rules.cpp
  theory/bv/theory_bv_type_rules.h
  theory/bv/theory_bv_utils.cpp
  theory/bv/theory_bv_utils.h
  theory/bv/type_enumerator.h
  theory/care_graph.h
  theory/care_pair_argument_callback.cpp
  theory/care_pair_argument_callback.cpp
  theory/combination_care_graph.cpp
  theory/combination_care_graph.h
  theory/combination_engine.cpp
  theory/combination_engine.h
  theory/conflict_processor.cpp
  theory/conflict_processor.h
  theory/datatypes/datatypes_rewriter.cpp
  theory/datatypes/datatypes_rewriter.h
  theory/datatypes/inference.cpp
  theory/datatypes/inference.h
  theory/datatypes/inference_manager.cpp
  theory/datatypes/inference_manager.h
  theory/datatypes/infer_proof_cons.cpp
  theory/datatypes/infer_proof_cons.h
  theory/datatypes/proof_checker.cpp
  theory/datatypes/proof_checker.h
  theory/datatypes/sygus_datatype_utils.cpp
  theory/datatypes/sygus_datatype_utils.h
  theory/datatypes/theory_datatypes.cpp
  theory/datatypes/theory_datatypes.h
  theory/datatypes/theory_datatypes_type_rules.cpp
  theory/datatypes/theory_datatypes_type_rules.h
  theory/datatypes/theory_datatypes_utils.cpp
  theory/datatypes/theory_datatypes_utils.h
  theory/datatypes/project_op.cpp
  theory/datatypes/project_op.h
  theory/datatypes/tuple_utils.cpp
  theory/datatypes/tuple_utils.h
  theory/datatypes/type_enumerator.cpp
  theory/datatypes/type_enumerator.h
  theory/decision_manager.cpp
  theory/decision_manager.h
  theory/decision_strategy.cpp
  theory/decision_strategy.h
  theory/ee_manager.cpp
  theory/ee_manager.h
  theory/ee_manager_central.cpp
  theory/ee_manager_central.h
  theory/ee_manager_distributed.cpp
  theory/ee_manager_distributed.h
  theory/ee_setup_info.h
  theory/evaluator.cpp
  theory/evaluator.h
  theory/ext_theory.cpp
  theory/ext_theory.h
  theory/interrupted.h
  theory/incomplete_id.cpp
  theory/incomplete_id.h
  theory/inference_id.cpp
  theory/inference_id.h
  theory/inference_manager_buffered.cpp
  theory/inference_manager_buffered.h
  theory/lemma_property.cpp
  theory/lemma_property.h
  theory/logic_info.cpp
  theory/logic_info.h
  theory/model_manager.cpp
  theory/model_manager.h
  theory/model_manager_distributed.cpp
  theory/model_manager_distributed.h
  theory/output_channel.cpp
  theory/output_channel.h
  theory/quantifiers/alpha_equivalence.cpp
  theory/quantifiers/alpha_equivalence.h
  theory/quantifiers/bv_inverter.cpp
  theory/quantifiers/bv_inverter.h
  theory/quantifiers/bv_inverter_utils.cpp
  theory/quantifiers/bv_inverter_utils.h
  theory/quantifiers/cegqi/ceg_arith_instantiator.cpp
  theory/quantifiers/cegqi/ceg_arith_instantiator.h
  theory/quantifiers/cegqi/ceg_bv_instantiator.cpp
  theory/quantifiers/cegqi/ceg_bv_instantiator.h
  theory/quantifiers/cegqi/ceg_bv_instantiator_utils.cpp
  theory/quantifiers/cegqi/ceg_bv_instantiator_utils.h
  theory/quantifiers/cegqi/ceg_dt_instantiator.cpp
  theory/quantifiers/cegqi/ceg_dt_instantiator.h
  theory/quantifiers/cegqi/ceg_instantiator.cpp
  theory/quantifiers/cegqi/ceg_instantiator.h
  theory/quantifiers/cegqi/ceg_utils.cpp
  theory/quantifiers/cegqi/ceg_utils.h
  theory/quantifiers/cegqi/inst_strategy_cegqi.cpp
  theory/quantifiers/cegqi/inst_strategy_cegqi.h
  theory/quantifiers/cegqi/instantiator.cpp
  theory/quantifiers/cegqi/instantiator.h
  theory/quantifiers/cegqi/vts_term_cache.cpp
  theory/quantifiers/cegqi/vts_term_cache.h
  theory/quantifiers/dynamic_rewrite.cpp
  theory/quantifiers/dynamic_rewrite.h
  theory/quantifiers/ematching/candidate_generator.cpp
  theory/quantifiers/ematching/candidate_generator.h
  theory/quantifiers/ematching/ho_trigger.cpp
  theory/quantifiers/ematching/ho_trigger.h
  theory/quantifiers/ematching/im_generator.cpp
  theory/quantifiers/ematching/im_generator.h
  theory/quantifiers/ematching/inst_match_generator.cpp
  theory/quantifiers/ematching/inst_match_generator.h
  theory/quantifiers/ematching/inst_match_generator_multi.cpp
  theory/quantifiers/ematching/inst_match_generator_multi.h
  theory/quantifiers/ematching/inst_match_generator_multi_linear.cpp
  theory/quantifiers/ematching/inst_match_generator_multi_linear.h
  theory/quantifiers/ematching/inst_match_generator_simple.cpp
  theory/quantifiers/ematching/inst_match_generator_simple.h
  theory/quantifiers/ematching/inst_strategy.cpp
  theory/quantifiers/ematching/inst_strategy.h
  theory/quantifiers/ematching/inst_strategy_e_matching.cpp
  theory/quantifiers/ematching/inst_strategy_e_matching.h
  theory/quantifiers/ematching/inst_strategy_e_matching_user.cpp
  theory/quantifiers/ematching/inst_strategy_e_matching_user.h
  theory/quantifiers/ematching/instantiation_engine.cpp
  theory/quantifiers/ematching/instantiation_engine.h
  theory/quantifiers/ematching/pattern_term_selector.cpp
  theory/quantifiers/ematching/pattern_term_selector.h
  theory/quantifiers/ematching/relational_match_generator.cpp
  theory/quantifiers/ematching/relational_match_generator.h
  theory/quantifiers/ematching/trigger.cpp
  theory/quantifiers/ematching/trigger.h
  theory/quantifiers/ematching/trigger_database.cpp
  theory/quantifiers/ematching/trigger_database.h
  theory/quantifiers/ematching/trigger_term_info.cpp
  theory/quantifiers/ematching/trigger_term_info.h
  theory/quantifiers/ematching/trigger_trie.cpp
  theory/quantifiers/ematching/trigger_trie.h
  theory/quantifiers/ematching/var_match_generator.cpp
  theory/quantifiers/ematching/var_match_generator.h
  theory/quantifiers/entailment_check.cpp
  theory/quantifiers/entailment_check.h
  theory/quantifiers/equality_query.cpp
  theory/quantifiers/equality_query.h
  theory/quantifiers/extended_rewrite.cpp
  theory/quantifiers/extended_rewrite.h
  theory/quantifiers/fmf/bounded_integers.cpp
  theory/quantifiers/fmf/bounded_integers.h
  theory/quantifiers/fmf/first_order_model_fmc.cpp
  theory/quantifiers/fmf/first_order_model_fmc.h
  theory/quantifiers/fmf/full_model_check.cpp
  theory/quantifiers/fmf/full_model_check.h
  theory/quantifiers/fmf/model_builder.cpp
  theory/quantifiers/fmf/model_builder.h
  theory/quantifiers/fmf/model_engine.cpp
  theory/quantifiers/fmf/model_engine.h
  theory/quantifiers/quant_rep_bound_ext.cpp
  theory/quantifiers/quant_rep_bound_ext.h
  theory/quantifiers/first_order_model.cpp
  theory/quantifiers/first_order_model.h
  theory/quantifiers/fun_def_evaluator.cpp
  theory/quantifiers/fun_def_evaluator.h
  theory/quantifiers/ho_term_database.cpp
  theory/quantifiers/ho_term_database.h
  theory/quantifiers/ieval/free_var_info.cpp
  theory/quantifiers/ieval/free_var_info.h
  theory/quantifiers/ieval/inst_evaluator.cpp
  theory/quantifiers/ieval/inst_evaluator.h
  theory/quantifiers/ieval/inst_evaluator_manager.cpp
  theory/quantifiers/ieval/inst_evaluator_manager.h
  theory/quantifiers/ieval/pattern_term_info.cpp
  theory/quantifiers/ieval/pattern_term_info.h
  theory/quantifiers/ieval/quant_info.cpp
  theory/quantifiers/ieval/quant_info.h
  theory/quantifiers/ieval/state.cpp
  theory/quantifiers/ieval/state.h
  theory/quantifiers/ieval/term_evaluator.cpp
  theory/quantifiers/ieval/term_evaluator.h
  theory/quantifiers/index_trie.cpp
  theory/quantifiers/index_trie.h
  theory/quantifiers/inst_match.cpp
  theory/quantifiers/inst_match.h
  theory/quantifiers/inst_match_trie.cpp
  theory/quantifiers/inst_match_trie.h
  theory/quantifiers/inst_strategy_enumerative.cpp
  theory/quantifiers/inst_strategy_enumerative.h
  theory/quantifiers/inst_strategy_mbqi.cpp
  theory/quantifiers/inst_strategy_mbqi.h
  theory/quantifiers/instantiate.cpp
  theory/quantifiers/instantiate.h
  theory/quantifiers/instantiation_list.cpp
  theory/quantifiers/instantiation_list.h
  theory/quantifiers/lazy_trie.cpp
  theory/quantifiers/lazy_trie.h
  theory/quantifiers/master_eq_notify.cpp
  theory/quantifiers/master_eq_notify.h
  theory/quantifiers/proof_checker.cpp
  theory/quantifiers/proof_checker.h
  theory/quantifiers/quant_bound_inference.cpp
  theory/quantifiers/quant_bound_inference.h
  theory/quantifiers/quant_conflict_find.cpp
  theory/quantifiers/quant_conflict_find.h
  theory/quantifiers/quant_relevance.cpp
  theory/quantifiers/quant_relevance.h
  theory/quantifiers/quant_split.cpp
  theory/quantifiers/quant_split.h
  theory/quantifiers/quant_util.cpp
  theory/quantifiers/quant_util.h
  theory/quantifiers/quant_module.cpp
  theory/quantifiers/quant_module.h
  theory/quantifiers/quantifiers_attributes.cpp
  theory/quantifiers/quantifiers_attributes.h
  theory/quantifiers/quantifiers_inference_manager.cpp
  theory/quantifiers/quantifiers_inference_manager.h
  theory/quantifiers/quantifiers_macros.cpp
  theory/quantifiers/quantifiers_macros.h
  theory/quantifiers/quantifiers_modules.cpp
  theory/quantifiers/quantifiers_modules.h
  theory/quantifiers/quantifiers_preprocess.cpp
  theory/quantifiers/quantifiers_preprocess.h
  theory/quantifiers/quantifiers_registry.cpp
  theory/quantifiers/quantifiers_registry.h
  theory/quantifiers/quantifiers_rewriter.cpp
  theory/quantifiers/quantifiers_rewriter.h
  theory/quantifiers/quantifiers_state.cpp
  theory/quantifiers/quantifiers_state.h
  theory/quantifiers/quantifiers_statistics.cpp
  theory/quantifiers/quantifiers_statistics.h
  theory/quantifiers/relevant_domain.cpp
  theory/quantifiers/relevant_domain.h
  theory/quantifiers/single_inv_partition.cpp
  theory/quantifiers/single_inv_partition.h
  theory/quantifiers/skolemize.cpp
  theory/quantifiers/skolemize.h
  theory/quantifiers/term_tuple_enumerator.cpp
  theory/quantifiers/term_tuple_enumerator.h
  theory/quantifiers/term_database.cpp
  theory/quantifiers/term_database.h
  theory/quantifiers/term_enumeration.cpp
  theory/quantifiers/term_enumeration.h
  theory/quantifiers/term_registry.cpp
  theory/quantifiers/term_registry.h
  theory/quantifiers/term_util.cpp
  theory/quantifiers/term_util.h
  theory/quantifiers/theory_quantifiers.cpp
  theory/quantifiers/theory_quantifiers.h
  theory/quantifiers/theory_quantifiers_type_rules.cpp
  theory/quantifiers/theory_quantifiers_type_rules.h
  theory/quantifiers_engine.cpp
  theory/quantifiers_engine.h
  theory/relevance_manager.cpp
  theory/relevance_manager.h
  theory/rep_set.cpp
  theory/rep_set.h
  theory/rep_set_iterator.cpp
  theory/rep_set_iterator.h
  theory/rewriter.cpp
  theory/rewriter.h
  theory/rewriter_attributes.h
  theory/sets/infer_proof_cons.cpp
  theory/sets/infer_proof_cons.h
  theory/sets/inference_manager.cpp
  theory/sets/inference_manager.h
  theory/sets/normal_form.h
  theory/sets/proof_checker.cpp
  theory/sets/proof_checker.h
  theory/sets/set_reduction.cpp
  theory/sets/set_reduction.h
  theory/sets/skolem_cache.cpp
  theory/sets/skolem_cache.h
  theory/sets/solver_state.cpp
  theory/sets/solver_state.h
  theory/sets/strategy.cpp
  theory/sets/strategy.h
  theory/sets/term_registry.cpp
  theory/sets/term_registry.h
  theory/sets/theory_sets.cpp
  theory/sets/theory_sets.h
  theory/sets/theory_sets_private.cpp
  theory/sets/theory_sets_private.h
  theory/sets/theory_sets_rewriter.cpp
  theory/sets/theory_sets_rewriter.h
  theory/sets/theory_sets_type_enumerator.cpp
  theory/sets/theory_sets_type_enumerator.h
  theory/sets/theory_sets_type_rules.cpp
  theory/sets/theory_sets_type_rules.h
  theory/shared_solver.cpp
  theory/shared_solver.h
  theory/shared_solver_distributed.cpp
  theory/shared_solver_distributed.h
  theory/shared_terms_database.cpp
  theory/shared_terms_database.h
  theory/skolem_lemma.cpp
  theory/skolem_lemma.h
  theory/smt_engine_subsolver.cpp
  theory/smt_engine_subsolver.h
  theory/sort_inference.cpp
  theory/sort_inference.h
  theory/plugin_module.cpp
  theory/plugin_module.h
  theory/step.cpp
  theory/step.h
  theory/strategy.cpp
  theory/strategy.h
  theory/strings/array_solver.cpp
  theory/strings/array_solver.h
  theory/strings/array_core_solver.cpp
  theory/strings/array_core_solver.h
  theory/strings/arith_entail.cpp
  theory/strings/arith_entail.h
  theory/strings/base_solver.cpp
  theory/strings/base_solver.h
  theory/strings/code_point_solver.cpp
  theory/strings/code_point_solver.h
  theory/strings/core_solver.cpp
  theory/strings/core_solver.h
  theory/strings/eager_solver.cpp
  theory/strings/eager_solver.h
  theory/strings/extf_solver.cpp
  theory/strings/extf_solver.h
  theory/strings/eqc_info.cpp
  theory/strings/eqc_info.h
  theory/strings/infer_info.cpp
  theory/strings/infer_info.h
  theory/strings/infer_proof_cons.cpp
  theory/strings/infer_proof_cons.h
  theory/strings/inference_manager.cpp
  theory/strings/inference_manager.h
  theory/strings/macro_rewrite_elaborator.cpp
  theory/strings/macro_rewrite_elaborator.h
  theory/strings/model_cons.h
  theory/strings/model_cons_default.cpp
  theory/strings/model_cons_default.h
  theory/strings/normal_form.cpp
  theory/strings/normal_form.h
  theory/strings/proof_checker.cpp
  theory/strings/proof_checker.h
  theory/strings/regexp_enumerator.cpp
  theory/strings/regexp_enumerator.h
  theory/strings/regexp_elim.cpp
  theory/strings/regexp_elim.h
  theory/strings/regexp_entail.cpp
  theory/strings/regexp_entail.h
  theory/strings/regexp_eval.cpp
  theory/strings/regexp_eval.h
  theory/strings/regexp_operation.cpp
  theory/strings/regexp_operation.h
  theory/strings/regexp_solver.cpp
  theory/strings/regexp_solver.h
  theory/strings/rewrites.cpp
  theory/strings/rewrites.h
  theory/strings/sequences_rewriter.cpp
  theory/strings/sequences_rewriter.h
  theory/strings/sequences_stats.cpp
  theory/strings/sequences_stats.h
  theory/strings/skolem_cache.cpp
  theory/strings/skolem_cache.h
  theory/strings/solver_state.cpp
  theory/strings/solver_state.h
  theory/strings/strategy.cpp
  theory/strings/strategy.h
  theory/strings/strings_entail.cpp
  theory/strings/strings_entail.h
  theory/strings/strings_rewriter.cpp
  theory/strings/strings_rewriter.h
  theory/strings/theory_strings.cpp
  theory/strings/theory_strings.h
  theory/strings/theory_strings_preprocess.cpp
  theory/strings/theory_strings_preprocess.h
  theory/strings/theory_strings_type_rules.cpp
  theory/strings/theory_strings_type_rules.h
  theory/strings/theory_strings_utils.cpp
  theory/strings/theory_strings_utils.h
  theory/strings/term_registry.cpp
  theory/strings/term_registry.h
  theory/strings/type_enumerator.cpp
  theory/strings/type_enumerator.h
  theory/strings/word.cpp
  theory/strings/word.h
  theory/subs_minimize.cpp
  theory/subs_minimize.h
  theory/substitutions.cpp
  theory/substitutions.h
  theory/term_registration_visitor.cpp
  theory/term_registration_visitor.h
  theory/theory.cpp
  theory/theory.h
  theory/theory_engine.cpp
  theory/theory_engine.h
  theory/theory_engine_module.cpp
  theory/theory_engine_module.h
  theory/theory_engine_proof_generator.cpp
  theory/theory_engine_proof_generator.h
  theory/theory_id.cpp
  theory/theory_id.h
  theory/theory_engine_statistics.cpp
  theory/theory_engine_statistics.h
  theory/theory_eq_notify.h
  theory/theory_inference.cpp
  theory/theory_inference.h
  theory/theory_inference_manager.cpp
  theory/theory_inference_manager.h
  theory/theory_model.cpp
  theory/theory_model.h
  theory/theory_model_builder.cpp
  theory/theory_model_builder.h
  theory/theory_preprocessor.cpp
  theory/theory_preprocessor.h
  theory/theory_rewriter.cpp
  theory/theory_rewriter.h
  theory/theory_state.cpp
  theory/theory_state.h
  theory/trust_substitutions.cpp
  theory/trust_substitutions.h
  theory/type_enumerator.h
  theory/type_set.cpp
  theory/type_set.h
  theory/uf/conversions_solver.cpp
  theory/uf/conversions_solver.h
  theory/uf/diamonds_proof_generator.cpp
  theory/uf/diamonds_proof_generator.h
  theory/uf/distinct_extension.cpp
  theory/uf/distinct_extension.h
  theory/uf/equality_engine.cpp
  theory/uf/equality_engine.h
  theory/uf/equality_engine_iterator.cpp
  theory/uf/equality_engine_iterator.h
  theory/uf/equality_engine_notify.h
  theory/uf/equality_engine_types.h
  theory/uf/eq_proof.cpp
  theory/uf/eq_proof.h
  theory/uf/function_const.cpp
  theory/uf/function_const.h
  theory/uf/lambda_lift.cpp
  theory/uf/lambda_lift.h
  theory/uf/proof_checker.cpp
  theory/uf/proof_checker.h
  theory/uf/proof_equality_engine.cpp
  theory/uf/proof_equality_engine.h
  theory/uf/ho_extension.cpp
  theory/uf/ho_extension.h
  theory/uf/lambda_lift.cpp
  theory/uf/lambda_lift.h
  theory/uf/theory_uf.cpp
  theory/uf/theory_uf.h
  theory/uf/theory_uf_model.cpp
  theory/uf/theory_uf_model.h
  theory/uf/theory_uf_rewriter.cpp
  theory/uf/theory_uf_rewriter.h
  theory/uf/theory_uf_type_rules.cpp
  theory/uf/theory_uf_type_rules.h
  theory/uf/type_enumerator.cpp
  theory/uf/type_enumerator.h
  theory/valuation.cpp
  theory/valuation.h
)

#-----------------------------------------------------------------------------#
# Add required include paths for this and all subdirectories.

include_directories(include)
include_directories(. ${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------#
# Take care of options

if (Python_VERSION VERSION_LESS "3.11.0")
  check_python_module("tomli")
endif()

set(options_toml_files
  options/arith_options.toml
  options/arrays_options.toml
  options/base_options.toml
  options/booleans_options.toml
  options/builtin_options.toml
  options/bv_options.toml
  options/datatypes_options.toml
  options/decision_options.toml
  options/expr_options.toml
  options/main_options.toml
  options/parser_options.toml
  options/printer_options.toml
  options/proof_options.toml
  options/prop_options.toml
  options/quantifiers_options.toml
  options/sets_options.toml
  options/smt_options.toml
  options/strings_options.toml
  options/theory_options.toml
  options/uf_options.toml
)
string(REPLACE "toml" "cpp;" options_gen_cpp_files ${options_toml_files})
string(REPLACE "toml" "h;"   options_gen_h_files ${options_toml_files})
list(APPEND options_gen_cpp_files "options/io_utils.cpp" "options/options.cpp" "options/options_public.cpp" "main/options.cpp")
list(APPEND options_gen_h_files "options/io_utils.h" "options/options.h")

libava6_add_sources(GENERATED ${options_gen_cpp_files} ${options_gen_h_files})

list_prepend(options_toml_files "${CMAKE_CURRENT_LIST_DIR}/" abs_toml_files)

# The mkoptions.py script only updates its output files if their content would
# actually change. This mechanism makes sure that running the script does not
# automatically trigger a full rebuild, but only a rebuild of those parts that
# include files that did actually change.
# This approach also means that the output files (which gen-options used to
# depend on) may not actually be updated and thus cmake would keep re-running
# mkoptions.py over and over again.
# We thus have an artificial options.stamp file that mkoptions.py always writes
# to, and can thus be used to communicate that all options files have been
# properly updated.
add_custom_command(
  OUTPUT
    options/options.stamp
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/options
  COMMAND
    ${Python_EXECUTABLE}
    ${CMAKE_CURRENT_LIST_DIR}/options/mkoptions.py
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${abs_toml_files}
  BYPRODUCTS
    ${options_gen_cpp_files} ${options_gen_h_files}
  DEPENDS
    options/mkoptions.py
    ${options_toml_files}
    main/options_template.cpp
    options/io_utils_template.h
    options/io_utils_template.cpp
    options/module_template.h
    options/module_template.cpp
    options/options_public_template.cpp
    options/options_template.h
    options/options_template.cpp
)

add_custom_target(gen-options DEPENDS options/options.stamp)


#-----------------------------------------------------------------------------#

  set(KINDS_TOML_FILES
  ${PROJECT_SOURCE_DIR}/src/theory/builtin/kinds.toml
  ${PROJECT_SOURCE_DIR}/src/theory/booleans/kinds.toml
  ${PROJECT_SOURCE_DIR}/src/theory/uf/kinds.toml
  ${PROJECT_SOURCE_DIR}/src/theory/arith/kinds.toml
  ${PROJECT_SOURCE_DIR}/src/theory/bv/kinds.toml


  ${PROJECT_SOURCE_DIR}/src/theory/arrays/kinds.toml
  ${PROJECT_SOURCE_DIR}/src/theory/datatypes/kinds.toml

  ${PROJECT_SOURCE_DIR}/src/theory/sets/kinds.toml

  ${PROJECT_SOURCE_DIR}/src/theory/strings/kinds.toml
  ${PROJECT_SOURCE_DIR}/src/theory/quantifiers/kinds.toml)
#-----------------------------------------------------------------------------#

set(REWRITES_FILES
  ${PROJECT_SOURCE_DIR}/src/theory/arith/rewrites
  ${PROJECT_SOURCE_DIR}/src/theory/arrays/rewrites
  ${PROJECT_SOURCE_DIR}/src/theory/booleans/rewrites
  ${PROJECT_SOURCE_DIR}/src/theory/builtin/rewrites
  ${PROJECT_SOURCE_DIR}/src/theory/bv/rewrites
  ${PROJECT_SOURCE_DIR}/src/theory/bv/rewrites-elimination
  ${PROJECT_SOURCE_DIR}/src/theory/bv/rewrites-simplification
  ${PROJECT_SOURCE_DIR}/src/theory/sets/rewrites
  ${PROJECT_SOURCE_DIR}/src/theory/strings/rewrites
  ${PROJECT_SOURCE_DIR}/src/theory/strings/rewrites-regexp-membership
  ${PROJECT_SOURCE_DIR}/src/theory/uf/rewrites
)

#-----------------------------------------------------------------------------#
# Add subdirectories

add_subdirectory(base)
add_subdirectory(context)
add_subdirectory(expr)
add_subdirectory(parser)
add_subdirectory(rewriter)
add_subdirectory(theory)
add_subdirectory(util)

#-----------------------------------------------------------------------------#
# All sources for libava6 are now collected in LIBAVA6_SRCS and (if generated)
# LIBAVA6_GEN_SRCS (via libava6_add_sources). We can now build libava6.

set_source_files_properties(${LIBAVA6_GEN_SRCS} PROPERTIES GENERATED TRUE)

# With debug enabled, var tracking will exceed max-vartrack-size for some autogenerated files, this makes
# the compiler to start-over without var-tracking-assignments which increases build time.
# This option has no effect for non-debug build (builds without "-g")
add_check_cxx_flag("-fno-var-tracking-assignments" ${LIBAVA6_GEN_SRCS})

# First build the object library
add_library(ava6-obj OBJECT ${LIBAVA6_SRCS} ${LIBAVA6_GEN_SRCS})
target_compile_definitions(ava6-obj PRIVATE -D__BUILDING_AVA6LIB -Dava6_obj_EXPORTS)
set_target_properties(ava6-obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
# add_dependencies() is necessary for cmake versions before 3.21
add_dependencies(ava6-obj ava6base ava6context)
# Add libava6 dependencies for generated sources.
add_dependencies(ava6-obj gen-expr gen-versioninfo gen-options gen-rewrites gen-tags gen-theory)

add_library(ava6 $<TARGET_OBJECTS:ava6-obj> $<TARGET_OBJECTS:ava6base> $<TARGET_OBJECTS:ava6context>)

set_target_properties(ava6 PROPERTIES OUTPUT_NAME ava6)

# Link the shared library
target_include_directories(ava6
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
# On Windows, CMake's default install action places
# DLLs into the runtime path (by default "bin")
install(TARGETS ava6 EXPORT ava6-targets)

if(NOT SKIP_SET_RPATH AND BUILD_SHARED_LIBS AND APPLE)
  update_rpath_macos(libava6.dylib)
  update_rpath_macos(libava6.${AVA6_SOVERSION}.dylib)
endif()

if(BUILD_SHARED_LIBS)
  set_target_properties(ava6 PROPERTIES
    VERSION ${AVA6_VERSION_NUMBER} SOVERSION ${AVA6_SOVERSION})
endif()

include(GenerateExportHeader)
generate_export_header(ava6-obj
  BASE_NAME ava6
  EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/ava6/ava6_export.h
)
install(FILES
  ${PROJECT_BINARY_DIR}/include/ava6/ava6_export.h
  DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}/ava6/
)

# Add library/include dependencies

# Add rt library
# Note: For glibc < 2.17 we have to additionally link against rt (man clock_gettime).
#       RT_LIBRARIES should be empty for glibc >= 2.17
target_link_libraries(ava6 PRIVATE ${RT_LIBRARIES})

if(ENABLE_VALGRIND)
  target_include_directories(ava6-obj SYSTEM PUBLIC ${Valgrind_INCLUDE_DIR})
endif()

add_dependencies(ava6-obj CaDiCaL)
target_include_directories(ava6-obj SYSTEM PRIVATE ${CaDiCaL_INCLUDE_DIR})
target_link_libraries(ava6 PRIVATE $<BUILD_INTERFACE:CaDiCaL> $<INSTALL_INTERFACE:ava6::cadical>)









# GMP has to be included at the end because of linker ordering issues when
# generating a static library.
add_dependencies(ava6-obj GMP)
target_include_directories(ava6-obj PRIVATE ${GMP_INCLUDE_DIR})
if(BUILD_SHARED_LIBS)
  target_link_libraries(ava6 PRIVATE $<BUILD_INTERFACE:GMP> $<INSTALL_INTERFACE:ava6::gmp>)
else()
  target_link_libraries(ava6 PUBLIC $<BUILD_INTERFACE:GMP> $<INSTALL_INTERFACE:ava6::gmp>)
endif()

#-----------------------------------------------------------------------------#
# Visit main subdirectory after creating target ava6. For target main, we have
# to manually add library dependencies since we can't use
# target_link_libraries(...) with object libraries for cmake versions <= 3.12.
# Thus, we can only visit main as soon as all dependencies for ava6 are set up.

add_subdirectory(main)
