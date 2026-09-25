# Ava6 kind audit

This cleanup started from revision `9d46406`. All 254 entries in the ten
`src/theory/*/kinds.toml` files were checked against constructors, parser/API
mappings, type rules, solver consumers, and proof/rewrite uses. There are now
228 internal kinds. Reference counts alone are insufficient: generated constants
and sorts, indexed operators, and rewrite DSL operators can have very few direct
C++ references.

## Removed kinds

| Group | Removed internal kinds | Reason |
| --- | --- | --- |
| Transcendentals | `EXPONENTIAL`, `SINE`, `COSINE`, `TANGENT`, `COSECANT`, `SECANT`, `COTANGENT`, `ARCSINE`, `ARCCOSINE`, `ARCTANGENT`, `ARCCOSECANT`, `ARCSECANT`, `ARCCOTANGENT`, `SQRT`, `PI` | Their solvers and parser support had already been removed; remaining uses were unreachable rewriting, API construction, typing, and printing. |
| Integer AND extensions | `IAND`, `IAND_OP`, `PIAND` | No supported solver or parser producer. |
| Libpoly values/predicates | `REAL_ALGEBRAIC_NUMBER`, `REAL_ALGEBRAIC_NUMBER_OP`, `INDEXED_ROOT_PREDICATE`, `INDEXED_ROOT_PREDICATE_OP` | Libpoly is absent. The algebraic-number wrapper always held a rational; irrational constructors were unreachable. |
| Arrays | `ARRAY_LAMBDA` | Only its type rule remained; no constructor or solver use. |
| Bit-vectors | `BITVECTOR_ACKERMANNIZE_UDIV`, `BITVECTOR_ACKERMANNIZE_UREM` | No producer; only obsolete model-evaluation registrations remained. |
| Regular expressions | `REGEXP_RV` | No producer after deletion of the unused regex intersection/derivative implementation. |

The cleanup also removes the associated type rules, payload headers, API
mappings/accessors, printer cases, and rewriter branches. It removes 20 public
kind entries, three transcendental skolem IDs, two bound-variable IDs, and 25
obsolete transcendental proof/rewrite-rule entries. `TermManager::mkPi` and the
four algebraic-number `Term` accessors are gone. Transcendental logic names such
as `QF_NRAT` and `QF_NIRAT` are rejected.

Live arithmetic coefficient operations now use `Rational` directly. Construction
still chooses integer type for integral coefficients, preserving the former
wrapper's behavior. Existing rational arithmetic unit tests are enabled without
the obsolete libpoly guard.

`NlModel::addBound` had no callers, so its bound map was always empty. Removed
that method, interval-only checks and repair loops, unused approximation flags,
and unused precision/lemma parameters. Exact substitutions and model repair
remain. Assertions must evaluate to true under those exact substitutions.

## Intentionally retained

- Internal `POW`: polynomial proof reconstruction and `ARITH_POW_ELIM`.
- Internal `POW2`, `INTS_LOG2`, and `INTS_ISPOW2`: live bit-vector rewrite DSL
  rules and their proof reconstruction. The unsupported public `POW`, `POW2`,
  and `LOG2` entries were removed.
- `BITVECTOR_BIT` and `BITVECTOR_FROM_BOOLS`: bit-blasting and proof rules.
- Generated constant/sort kinds and indexed-operator payloads: construction
  through generated metadata does not necessarily mention `Kind::...` directly.
- Ordinary datatypes, bounded-integer quantifier reasoning, and first-order
  function/model/proof support.

## Validation

- Debug and production builds pass with `-Werror`.
- All **2,405 tests pass**: 2,290 regressions, 89 unit entries, 24 C++ API
  entries, and two core checks. No regressions were disabled for this cleanup.
- All 12 core CPC proof cases pass external Ethos checking in both builds.
- Parser checks reject the removed arithmetic symbols and transcendental logics.
- A final source scan found no remaining references to the 26 removed kinds.
- Fixed CMake dependencies on the rewrite compiler's Python modules. Removing
  the six remaining transcendental DSL operators leaves all 28 regenerated
  C++/header outputs across the two builds byte-for-byte identical to those
  already built and checked.

Logs and the original inventory are under
[build-ava6/kind-audit](build-ava6/kind-audit).

The net reduction is **3,239 production C++ lines** and **3,597 tracked physical
lines overall**, including tests, metadata, and design documentation; these
figures exclude this new audit document.
