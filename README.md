# Ava6

Ava6 is an independent experimental fork of cvc5. It starts with cvc5's solver
architecture and strips the implementation down to a fixed SMT core, providing
a smaller baseline for experiments in solving, proofs, parsing, and APIs.
The code and proof infrastructure come from cvc5 and its contributors, credited
in [AUTHORS](AUTHORS). Ava6 develops its own feature set and may change its APIs
as experiments progress.

It retains the SMT-LIB text interface and C++ API, models, incremental solving,
quantifiers, proof construction/checking, and **CPC as the only proof output**.
The eight theories are Booleans, uninterpreted functions, arithmetic,
bit-vectors, arrays, datatypes, sets, and strings/sequences. Builtin terms and
quantifier infrastructure support these theories.

Floating points, finite fields, separation logic, bags, SyGuS solving,
quantifier-elimination queries, abduction, interpolation, oracles,
nonlinear coverings (`--nl-cov`), and libpoly
are excluded. The C, Java, and Python APIs and documentation publishing setup
are excluded. Expert command-line and SMT-LIB options are removed.
The retained language is the default, with no safe/unrestricted modes or
separate illegal-input checker.

See [DESIGN.md](DESIGN.md) for the boundary between retained core policies and
removed features, and [test/README.md](test/README.md) for test selection.

## Origin

The initial import uses the tracked contents of the local `~/cvc5-pr-ajr`
checkout at `02594aba46d28c583d16f8f03a972a64a36742e9` (September 24, 2026).
That checkout had no tracked modifications. Build products, dependencies,
untracked experiments, and upstream Git history were excluded from the import.
Subsequent changes are recorded in this repository's Git history.

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
Create terms and sorts with an explicit `ava6::TermManager`, then construct
`ava6::Solver solver(tm)`. The deprecated cvc5 `Solver` construction methods
and implicit term manager have been removed.
SyGuS, quantifier-elimination, abduction, and interpolation commands are removed
from both the API and parser.

Upstream licensing and attribution are preserved in `COPYING`, `AUTHORS`,
`licenses/`, and individual source files.

CI consists of one debug build and two compact integration test entries (SMT-LIB
and C++ API), run with `ctest --test-dir build -L core`. Build the
`core-api-smoke` target first. There are no checker-subproject or publishing jobs.

## Code size

Run `./count_loc` for C++ line counts grouped by `src`, `include`, and `test`.
It uses Python 3's standard library and needs no `cloc` installation or downloads.
Pass paths to narrow the scope, for example `./count_loc src/theory`, or use
`--by-file` or `--json` for more detail. Build directories and downloaded
dependencies are excluded; bundled source such as MiniSAT is included.
Counts are physical lines: mixed code/comment lines count as code, and blank
lines remain blank even inside multiline comments. Generated build outputs
are excluded; their checked-in C++ templates are counted.
