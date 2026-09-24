# ava6

This repository starts from cvc5, using the tracked contents of `~/cvc5-pr-ajr` at
`02594aba46d28c583d16f8f03a972a64a36742e9` (September 24, 2026). The source
checkout had no tracked modifications. Its build products, dependencies,
untracked experiments, and Git history were not imported.

The baseline is a solver for experiments, with a fixed core language
and feature set.
It retains the SMT-LIB text interface and C++ API, models, incremental solving,
quantifiers, proof construction/checking, and **CPC as the only proof output**.
The eight theories are Booleans, uninterpreted functions, arithmetic,
bit-vectors, arrays, datatypes, sets, and strings/sequences. Builtin terms and
quantifier infrastructure support these theories.

Floating points, finite fields, separation logic, bags, SyGuS solving,
abduction, interpolation, oracles, nonlinear coverings (`--nl-cov`), and libpoly
are excluded. The C, Java, and Python APIs and documentation publishing setup
are excluded. Expert command-line and SMT-LIB options are removed.

See [DESIGN.md](DESIGN.md) for the boundary between retained core policies and
removed features, and [test/README.md](test/README.md) for test selection.

## Build

Use CMake 3.16+, a C++17 compiler, Python 3 (3.11+ avoids the `tomli` dependency),
the Python `pyparsing` module, GMP, and CaDiCaL. GoogleTest is needed only for unit tests. Dependencies can be
provided through `CMAKE_PREFIX_PATH`; `ENABLE_AUTO_DOWNLOAD=ON` enables the
upstream dependency download mechanism. Dependency installations are not vendored.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_UNIT_TESTING=ON -DSTATIC_BINARY=OFF \
  -DCMAKE_PREFIX_PATH=/path/to/dependencies
cmake --build build -j
cmake --build build --target build-tests -j
ctest --test-dir build --output-on-failure
python3 tools/check_core.py build/bin/ava6
```

The thin `configure.sh` wrapper also accepts `debug`, `production`, `testing`,
`--unit-testing`, `--auto-download`, and `-DKEY=VALUE`. Unsupported optional libraries and language bindings are rejected at
configuration time. No documentation build or publication target is provided.

For external checking of the smoke-test CPC proofs, add
`--ethos /path/to/ethos` to `tools/check_core.py`. The signature is
[proofs/eo/cpc/Cpc.eo](proofs/eo/cpc/Cpc.eo).

Installed C++ clients can use `find_package(ava6 CONFIG REQUIRED)` and link
`ava6::ava6` and `ava6::ava6parser`. For static installations, include the
CaDiCaL and GMP installation prefixes in the client's `CMAKE_PREFIX_PATH`.

Upstream licensing and attribution are preserved in `COPYING`, `AUTHORS`,
`licenses/`, and individual source files.

CI consists of one debug build and two compact integration test entries (SMT-LIB
and C++ API), run with `ctest --test-dir build -L core`. Build the
`core-api-smoke` target first. There are no checker-subproject or publishing jobs.
