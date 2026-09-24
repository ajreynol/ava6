# SMT-LIB regressions

Only levels 0, 1, and 2 are retained. Benchmarks that require removed theories,
options, or commands are omitted, as are upstream disabled tests. Register new
`.smt2` files in [CMakeLists.txt](CMakeLists.txt).

Build the solver, then run from the repository root:

```sh
RUN_REGRESSION_ARGS='--tester base' ctest --test-dir build -L 'regress[0-2]' --output-on-failure
```

The driver also supports `model`, `proof`, `unsat-core`, `dump`, and `cpc`
testers. CPC checking requires Ethos and the signature in `proofs/eo/cpc`.
Use `TEST_TIMEOUT` to set the timeout in seconds per solver invocation.
`AVA6_REGRESSION_ARGS` adds solver options to each invocation.

SMT-LIB comments specify expected behavior:

```smt2
; COMMAND-LINE: --incremental
; EXPECT: sat
; EXPECT: unsat
(set-logic QF_UF)
(declare-const p Bool)
(assert p)
(check-sat)
(assert (not p))
(check-sat)
```

Repeat `COMMAND-LINE` to run with multiple option combinations. `EXPECT-ERROR`
checks stderr; `EXIT` specifies the exit status. `SCRUBBER` and `ERROR-SCRUBBER`
filter output before comparison. `DISABLE-TESTER` skips an inapplicable tester.
`REQUIRES` selects a build capability listed by `ava6 --show-config`, such as
`statistics`; prefix it with `no-` to exclude that capability.
