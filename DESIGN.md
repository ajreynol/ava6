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

The C++ API remains useful for SMT solving and parser integration. Public kinds, sort kinds, constructors, value queries, proof identifiers, and
skolem identifiers for removed theories are deleted. The SyGuS API,
including `Grammar`, `SynthResult`, and synthesis queries, is deleted.
Quantifier elimination, abduction, interpolation, oracle, and separation-logic
queries are also deleted from the public solver API. Their parser commands and
internal solver entry points are removed together. Ordinary quantified SMT
solving remains supported.
Term and sort construction belongs to `TermManager`. Every `Solver` requires
an explicit term manager; the deprecated forwarding methods on `Solver` and
its implicit thread-local term manager are removed.
The plugin API and its internal adapters, SAT callbacks, and theory-engine
module infrastructure are removed. The retained theories communicate directly
with the theory engine. Quantifier reasoning modules remain part of the
quantifiers engine.

CPC printing uses the existing `proof/eo` implementation. Alethe and DOT printers
and the expert CPC signature are removed. Internal proof Nodes and debug traces
remain available to solver code; these are not selectable proof output formats.

Ordinary inductive datatypes remain, including their constructor, selector,
tester, and matching operations. Shared-selector encodings are removed;
selectors retain their original constructor signatures. Codatatype declarations, cyclic values,
bisimilarity, and their API construction flags are removed. Nullable datatypes
and SyGuS datatype encodings (`DT_*` kinds, evaluation, size/height bounds,
grammar attributes, and constructor weights) are removed.

Sets retain ordinary finite-set operations. Relational operators, cardinality,
universe sets, complement, and higher-order set operations are removed. UF cardinality constraints,
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
points. Internal learned-literal classification remains for the diagnostic
`-o learned-lits` output. Model-core computation and its API query are removed.

Higher-order solving, function-valued inputs, partial applications, `HO_APPLY`,
and higher-order elimination/instantiation are removed. Internal lambda terms
still represent ordinary first-order function definitions and function models.
`HO_CONG` remains as a proof rule for rewriting an `APPLY_UF` operator when
expanding a first-order function definition. It does not enable higher-order
input or solving.

## Options and internal policies

`[[option]]` entries define the configurable interface. The category-summary
command and no-support classifications are removed; all retained options are
supported. Expert options and
`[[setting]]` entries, including generator support, are removed. Fixed algorithm
choices live in their implementations, with unused branches deleted.
[solver_config.h](src/options/solver_config.h) holds only configuration derived
from the logic and supported options, plus stream/parser state. For example,
proof and unsat-core bookkeeping follows the requested output and checking;
these are not independent configurable modes.

Benchmark normalization and unconstrained simplification passes are deleted.
There is no interactive shell or Editline dependency; stdin and file input
use the ordinary SMT-LIB parser.

External optional arithmetic and SAT backends have been removed from the build;
GMP and CaDiCaL are retained. Linear arithmetic uses exact simplex; the
disabled GLPK approximation backend, cut-log replay, and approximate-solution
import are deleted. Exact integer branching and its proof generation remain.
CaDiCaL is the only SAT backend; Minisat and
backend selection are removed. The central equality engine is the sole
architecture: applicable theories share its facts and receive its notifications.
There are no per-theory solving equality engines or master-engine forwarding.
Model construction still has its own equality engine. The benchmark
`regress1/nl/nl_uf_lalt.smt2` currently exceeds a 120-second timeout with this
architecture, both in Ava6 and in the upstream checkout with central equality
selected. Its input is retained for manual investigation, but it is omitted
from automatic regression runs.
The fixed algorithms use concrete implementations: BV solving owns its
bitblaster directly, and propositional solving constructs CaDiCaL without a
solver factory or interchangeable SAT interface. CNF conversion keeps a small
clause-sink interface for its unit tests. Linear arithmetic retains dual and
sum-of-infeasibilities simplex; the unused feasibility-correction simplex is
deleted. A single SMT driver manages preprocessing and incremental queries,
without retry strategies or global-negation result handling. Care-graph
combination lives in `CombinationEngine` without a strategy subclass. The
central equality engine is assigned directly to participating theories, and
the model manager owns its separate, independently resettable equality engine.
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
