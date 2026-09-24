# Ava6 design

Ava6 is an experimental fork built by reducing cvc5 to a fixed SMT core.
Its initial algorithms and proof infrastructure are inherited from cvc5;
the smaller feature boundary provides a baseline for subsequent experiments.
The import revision and attribution are recorded in [README.md](README.md)
and [AUTHORS](AUTHORS).

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
Remaining constructors for excluded theories reject calls. The SyGuS API,
including `Grammar`, `SynthResult`, and synthesis queries, is deleted.
Quantifier elimination, abduction, interpolation, oracle, and separation-logic
queries are also deleted from the public solver API. Their parser commands and
internal solver entry points are removed together. Ordinary quantified SMT
solving remains supported. Future API experiments can remove the remaining
compatibility metadata for excluded theories.
Term and sort construction belongs to `TermManager`. Every `Solver` requires
an explicit term manager; the deprecated forwarding methods on `Solver` and
its implicit thread-local term manager are removed.

CPC printing uses the existing `proof/eo` implementation. Alethe and DOT printers
and the expert CPC signature are removed. Internal proof Nodes and debug traces
remain available to solver code; these are not selectable proof output formats.

Ordinary inductive datatypes remain, including their constructor, selector,
tester, and matching operations. Codatatype declarations, cyclic values,
bisimilarity, and their API construction flags are removed.

Sets retain ordinary finite-set operations. Relational operators, cardinality,
universe sets, and complement are removed. UF cardinality constraints,
`--finite-model-find`, and strings FMF are removed. Bounded-integer quantification
(`--fmf-bound`) and its model-checking machinery remain, including dependent
integer bounds, fixed finite lists of terms, and small interpreted finite types.
String operations use this machinery internally. Set-membership bounds that
require set-cardinality machinery are removed. General quantified SMT solving
and model construction remain. Mathematical type cardinality is still needed
for bit-vectors, datatypes, arrays, and sequences; it is separate from the
removed user-level cardinality constraints. Modern MBQI remains available.

Quantifier-free solving uses the general theory model builder; quantified
solving extends it with bounded-quantifier model construction. Its assignment exclusion-set interface and assigners are removed:
the retained theories had no callers that populated these sets. Ordinary
datatype model skeletons and fresh-value enumeration remain. Codatatype-specific
value exclusion and FMF domain restrictions disappear with their solvers.

Pool declarations/annotations and difficulty, timeout-core, learned-literal,
and model-blocking queries have no API, parser commands, or solver-engine entry
points. Internal learned-literal classification used by lemma preprocessing
remains private to the propositional solver.

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
