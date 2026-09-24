# Baseline design

The experiment starts with the existing solver architecture. The parser still
constructs public `Term` objects through the C++ API. Moving it to internal
`Node` construction is a future experiment, not part of initialization.

## Supported boundary

The solver has no safety profiles, runtime mode switches, or illegal-input
checking pass. The parser and C++ API expose the retained language directly. The Node kind generators,
theory constructors, rewriters, and enumerators include only the eight retained
theories plus builtin and quantifier infrastructure. Removed theories have no
solver directories or generated internal kinds.

The C++ API remains useful for SMT solving and parser integration. Some legacy
public declarations and enums are retained to avoid an unrelated API redesign.
Their removed solving/construction entry points reject calls; compatibility
metadata is not an implementation of the excluded theories or synthesis solver.
Shared datatype/grammar representations are still part of this inherited API
boundary. Future API experiments can remove that compatibility surface.

CPC printing uses the existing `proof/eo` implementation. Alethe and DOT printers
and the expert CPC signature are removed. Internal proof Nodes and debug traces
remain available to solver code; these are not selectable proof output formats.

## Options and internal policies

`[[option]]` entries define the configurable interface. There are no expert
entries. `[[setting]]` entries describe private policies used by retained core
algorithms, and are never registered in command-line parsing, `set-option`,
`get-option`, or public option enumeration. Some policies are adjusted internally
when enabling models/proofs or selecting algorithms for a logic; preserving
these decisions avoids changing the behavior of core solving during import.
Unused policy entries are removed with their features. The settings are not a
mechanism for restoring expert command-line options.

External optional arithmetic and SAT backends have been removed from the build;
GMP and CaDiCaL, along with the inherited internal SAT machinery, are retained.
Nonlinear arithmetic uses the retained extension solver and is still incomplete
on some inputs. The rational implementation of real algebraic number storage is
retained without libpoly.

## Changes for later experiments

Keep the language boundary and test exclusions explicit. When deliberately
extending the boundary, update the Node generators, parser/API surface, option
vocabulary, proof signature, and regression selection together. A runtime flag
alone should not silently reintroduce an excluded feature.

## Project identity

The implementation uses namespace `ava6` (`ava6::internal` for internals),
public headers under `ava6/`, the `ava6` executable, and libraries `ava6` and
`ava6parser`. CMake packages and exported targets use `ava6`; mixed-case
identifiers use `Ava6`/`Ava` (for example, `Ava6ApiException`), and
preprocessor names use `AVA6`. Upstream
copyright notices, authorship, and external source links remain intact.
