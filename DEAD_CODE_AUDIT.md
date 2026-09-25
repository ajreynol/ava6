# Ava6 dead-code audit and cleanup

The coverage experiment below measured solver revision `b963d89` before the
cleanup. Deletions were established by checking callers, constant guards,
fixed strategy schedules, supported options, and proof/API uses. Zero coverage
alone was not treated as evidence of dead code.

## Completed deletions

| Removed code | Evidence and retained behavior |
| --- | --- |
| Old regex derivative/intersection implementation | `derivativeS` was only recursive; the other derivative/intersection methods formed an unused cluster. Removed 1,215 lines of bodies plus declarations, caches, and cached constants. All retained regex operation bodies are unchanged, including membership reduction, `delta`, inclusion checking, and trace printing. Regex intersection remains supported. |
| BV CEGQI normalization pipeline | `rewriteAssertionForSolvePv` had no caller. Removed it, `rewriteTermForSolvePv`, and their sole utility `BvInstantiatorUtil`: 681 baseline lines plus declarations/member. Removed the 471-line utility-specific unit test. Live BV inversion and instantiation remain. |
| Isolated helpers | Deleted `SubstitutionMinimize` (580 lines), `IAndUtils` (446), `BetaReduceNodeConverter` (76), and the old `MatchTrie`. Preserved its shared `NotifyMatch` interface in [notify_match.h](src/expr/notify_match.h) for the live n-ary trie and proof reconstruction. |
| Variable-substitution trigger matcher | Its sole factory branch checked an always-null variable. Removed the branch, `var_match_generator.h/.cpp`, and `PatternTermSelector::getInversion`. Ordinary and relational matching remain. |
| Arithmetic expert rewrites | The only `ArithRewriter` owner always disabled these paths. Removed the flag, expert dispatch, transcendental/IAND rewrites, and guarded PI rewrite. Preserved `expandPowConst`, which is used by proof rewrite reconstruction. |
| Oracle infrastructure | Removed oracle callbacks from `NodeManager`, the oracle header, Node kinds/type rule/attributes, unused parser token, and printer methods. Also removed the unused `AbsTypeFunDefAttribute`. |
| CEGQI single-instantiation state | Removed constant-false multi-instantiation/revert controls, never-populated slack state, and write-only bookkeeping. BV instantiation still chooses the first candidate from the same shuffled list and restores solved-state bookkeeping on failure. |
| Nonlinear strategy machinery | Replaced weighted interleaving with one fixed step sequence and an initialization flag, preserving the empty-sequence case. Removed never-scheduled resolution-bounds and split-zero checks and their dependent identifiers/state. |
| QE metadata and recording | Removed the remaining `quant-elim` / `quant-elim-partial` attribute interpretation, CEGQI branches, explicit instantiation recording, and associated incompleteness/recheck special cases. The public QE API/parser commands were already removed. Ordinary quantifier solving, witness elimination, and `mkAttrPreserveStructure` remain. |
| Context helper | Removed unused `CDRaised`; `CDMaybe` remains used by the Diophantine solver. |

The six options below had no solver effect and are now removed. The existing
option-initialization regression uses the live `preregister-mode` option instead
of `sat-random-seed`. The compact integration checks verify that all six are
rejected.

- `--nl-cov-linear-model`
- `--cegis-sample`
- `--sub-cbqi-timeout`
- `--elim-taut-quant`
- `--sat-random-seed`
- `--nl-ext-tf-tplanes`

Removed **145 stale identifiers** from inference, trust, incompleteness,
internal-skolem, and parser-token enums and their printing/dispatch code:
105 inference identifiers, 28 trust identifiers, 3 incompleteness identifiers,
4 internal-skolem identifiers, and 5 parser tokens. This includes identifiers
whose only remaining uses were dispatch cases for deleted producers. The
nonlinear scheduler's removed step identifiers are additional to those counts.

All **12 retained preprocessing passes** executed their `applyInternal` method
in the coverage run: Ackermann, ApplySubsts, BoolToBV, BvEagerAtoms, BVToBool,
NonClausalSimp, QuantifiersPreprocess, RealToInt, Rewrite, StaticLearning,
StaticRewrite, and TheoryPreprocess. None was deleted.

Bounded-integer quantifier reasoning, datatypes, models, debug/error paths,
first-order lambda handling, and CPC proofs remain supported.

## Cleanup validation

- Debug solver, C++ API integration executable, and unit tests build with
  assertions and tracing enabled and `-Werror`.
- Production solver builds with `-O3 -Werror`; its compact integration suite
  passes, including models, incremental solving, proofs, and rejection of
  removed features/options.
- Ethos checks all **12 CPC smoke proofs**, with no incomplete checks.
- All **2,405 tests pass**: 2,290 regressions, 89 unit tests, 24 C++ API
  tests, and 2 core integration tests. Wall time: 117.64 seconds, with a
  120-second per-process regression timeout. No regressions were disabled.

Logs are under [build-ava6/dead-code-validation](build-ava6/dead-code-validation),
including [external CPC checks](build-ava6/dead-code-validation/cpc-smoke.log).
The net reduction is **6,057 physical lines**, including source, headers,
obsolete tests, and build registrations; this excludes the audit document and
coverage-reporting script. Of these, 5,492 are production C++ source/header
lines and 471 are the obsolete utility unit test.

## Uncovered code that still has supported callers

- `resolution_proofs_util.cpp` has zero coverage here, but proof postprocessing
  calls it when eliminating chain multiset resolution. The supported
  `--no-proof-chain-m-res` option can select that path.
- `inst_match_generator_multi.cpp` has zero coverage, but the retained
  `--multi-trigger-cache` option selects it.
- AST/debug printers, assertion handling, error output, and some CPC conversion
  helpers are uncovered. The regression runners do not exercise every output
  mode, trace, error, API method, or proof configuration.
- SOI simplex has very low coverage, but arithmetic still has an explicit
  fallback call into it. Removing it requires an algorithmic reachability
  argument beyond the regression coverage.

The first two cases are useful targets for additional regression coverage.

## Coverage experiment

| Measurement | Result |
| --- | ---: |
| Regressions | 2,290 |
| Passed under instrumentation | 2,288 |
| Instrumentation-run timeouts | 2 |
| Covered executable source lines | 79,969 / 111,256 (**71.88%**) |
| Source/header files represented | 911 |
| Uncalled emitted functions | 3,221 / 13,531 |
| Translation units without runtime counters | 26 / 542 |
| Regression wall time | 17 minutes |

The two timeouts were `regress1/quantifiers/inv-conds.smt2` (unsat-core
tester) and `regress1/quantifiers/smtlibe99bbe.smt2` (proof tester). Both passed
all their default testers when rerun with the normal debug binary and a
60-second process limit. These are instrumentation-run timeouts, not a reason
to disable either regression. Counters from forcibly terminated processes may
be missing.

Local results: [summary](build-dead-code-coverage/coverage-audit/summary.json),
[per-file coverage](build-dead-code-coverage/coverage-audit/files.csv),
[uncalled functions](build-dead-code-coverage/coverage-audit/uncalled-functions.csv),
and [regression log](build-dead-code-coverage/coverage-audit/regress.log).
The normal-debug control logs are
[inv-conds](build-dead-code-coverage/coverage-audit/inv-conds-normal-debug.log)
and [smtlibe99bbe](build-dead-code-coverage/coverage-audit/smtlibe99bbe-normal-debug.log).
Generated measurements stay in the ignored build directory.

The build is isolated in `build-dead-code-coverage`. It uses GCC coverage at
`-O0`, with assertions and tracing compiled in, and the default regression
runners: base, unsat cores, proofs, models, and dump. The 120-second per-process
limit accommodates instrumentation overhead. This is CLI regression coverage;
it does not include the separate C++ API/unit suite or a full CPC/Ethos run.

The line totals cover handwritten sources under `src/` and public headers under
`include/`. Generated C++ sources, dependencies, and system headers are excluded.
Header/template instances are merged: a line or function is covered if any
instance ran. Exception paths, assertions, debug printing, and API-only methods
are expected to contribute uncovered code.

## Reproducing the report

The installed GCC and its matching `gcov`, plus Python's standard library, are
sufficient. No `lcov`, `fastcov`, or Python packages are required. Configure a
separate build with the usual local dependency prefix and these extra flags:

```sh
./configure.sh debug --ninja --name=build-dead-code-coverage \
  -DENABLE_DEBUG_SYMBOLS=OFF -DENABLE_UNIT_TESTING=OFF \
  -DENABLE_AUTO_DOWNLOAD=OFF \
  -DCMAKE_PREFIX_PATH=/local/ajreynol/pr-ajr-safe/deps \
  -DBUILD_SHARED_LIBS=OFF -DSTATIC_BINARY=OFF \
  '-DCMAKE_CXX_FLAGS=--coverage -fprofile-update=atomic -g0' \
  '-DCMAKE_C_FLAGS=--coverage -fprofile-update=atomic -g0' \
  -DCMAKE_CXX_FLAGS_DEBUG=-O0 -DCMAKE_C_FLAGS_DEBUG=-O0
cmake --build build-dead-code-coverage --target ava6-bin -j12
TEST_TIMEOUT=120 ctest --test-dir build-dead-code-coverage/test/regress/cli \
  --output-on-failure -L 'regress[0-2]' -j12
python3 contrib/coverage_audit.py build-dead-code-coverage
```

Use a fresh build/profile dataset for a fresh measurement: GCC otherwise merges
new counters into previous runs. This audit redirected runtime counters to a
private temporary directory using `GCOV_PREFIX` to reduce filesystem overhead,
then copied them back to their corresponding build directories before reading.
The generated CSVs contain per-file coverage and uncalled function locations;
`coverage.json` retains merged line/function data for further analysis.
