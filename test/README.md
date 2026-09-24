# Tests for the core baseline

The imported regression selection excludes benchmarks marked
`REQUIRES: unrestricted-mode` or `REQUIRES: no-safe-mode`, SyGuS inputs,
benchmarks using removed expert options, and benchmarks for removed theories
and solving commands. Exclusion removes the benchmark files as well as their
CMake registrations. C++ unit/API suites retain applicable test cases; the
other language API suites are absent.

`cmake --build build --target build-tests` builds the registered tests.
`ctest --test-dir build --output-on-failure -L unit` runs unit tests.
The regression driver supports normal solving, model/proof checks, and CPC
checking with an external Ethos installation. The expert CPC signature and
Alethe tester are not part of this repository.

`python3 tools/check_core.py build/bin/ava6` checks all eight theory families,
quantifiers, CPC proof generation, models, incremental use, rejection of removed
features, and the absence of unrestricted benchmarks. Pass `--ethos PATH` to
check the generated proofs against the retained CPC signature.

Upstream disabled regressions are deleted, and `regression_disabled_tests` is
empty. Only regression levels 0, 1, and 2 are retained; levels 3 and 4 are omitted.
