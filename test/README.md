# Ava6 tests

These tests come from the cvc5 snapshot recorded in the root README, with
selection and updates for Ava6's retained SMT language and C++ API.

The imported regression selection excludes benchmarks marked
`REQUIRES: unrestricted-mode` or `REQUIRES: no-safe-mode`, SyGuS inputs,
benchmarks using removed expert options, and benchmarks for removed theories
and solving commands. Exclusion removes the benchmark files as well as their
CMake registrations. C++ unit/API suites retain applicable test cases; the
other language API suites are absent.
Inputs using quantifier-elimination queries are also omitted. The compact core
check verifies that the parser rejects synthesis, quantifier-elimination,
abduction, and interpolation commands.

Inputs using pools, codatatypes, `--finite-model-find`, strings FMF,
UF/set cardinality, relational sets,
universe sets/complement, difficulty, timeout cores, learned-literal queries,
or model blocking are also deleted. Ordinary datatype and bounded-integer
(`--fmf-bound`) tests remain; bounds based on set membership are omitted with
the removed set-cardinality machinery.

`cmake --build build --target build-tests` builds the registered tests.
`ctest --test-dir build --output-on-failure -L unit` runs unit tests.
The regression driver supports normal solving, model/proof checks, and CPC
checking with an external Ethos installation. The expert CPC signature and
Alethe tester are not part of this repository.

Run `./contrib/get-ethos-checker` from the repository to install the pinned
checker into `deps/bin/ethos`, then run
`cmake --build build --target regress-cpc` (or `make -C build regress-cpc`).
Set `ETHOS_EXECUTABLE` in CMake to use a checker installed elsewhere.

`python3 tools/check_core.py build/bin/ava6` checks all eight theory families,
quantifiers, CPC proof generation, models, incremental use, rejection of removed
features, and the absence of unrestricted benchmarks. Pass `--ethos PATH` to
check the generated proofs against the retained CPC signature.

Upstream disabled regressions are deleted, and `regression_disabled_tests` is
empty. Only regression levels 0, 1, and 2 are retained; levels 3 and 4 are omitted.
