#!/usr/bin/env python3
"""Integration checks for the deliberately restricted solver baseline."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CASES = {
    'booleans': '(declare-const p Bool) (assert p) (assert (not p))',
    'uf': '''(declare-sort U 0) (declare-const a U) (declare-const b U)
      (declare-fun f (U) U) (assert (= a b)) (assert (distinct (f a) (f b)))''',
    'function-definitions': '''(define-fun f ((x Int)) Int (+ x 1))
      (declare-const a Int) (assert (distinct (f a) (+ a 1)))''',
    'combined-equality': '''(declare-fun f (Int) Int)
      (declare-const a (Array Int Int)) (declare-const x Int)
      (declare-const y Int) (assert (= (+ x 1) y))
      (assert (= (select a (f (+ x 1))) 0))
      (assert (= (select a (f y)) 1))''',
    'arithmetic': '(declare-const x Int) (assert (< x 0)) (assert (>= x 0))',
    'bitvectors': '''(declare-const x (_ BitVec 8))
      (assert (distinct (bvand #x03 x x #x07) (bvand #x03 x)))''',
    'arrays': '''(declare-const a (Array Int Int))
      (assert (distinct (select (store a 0 1) 0) 1))''',
    'datatypes': '''(declare-datatype D ((a) (b))) (declare-const x D)
      (assert (distinct x a)) (assert (distinct x b))''',
    'recursive-datatypes': '''(declare-datatype L ((nil) (cons (head Int) (tail L))))
      (declare-const x L) (assert (= x (cons 0 x)))''',
    'sets': '''(declare-const x Int) (declare-const s (Set Int))
      (assert (set.member x s)) (assert (not (set.member x s)))''',
    'strings': '''(declare-const s String) (assert (= s "abc"))
      (assert (distinct (str.len s) 3))''',
    'quantifiers': '''(declare-fun p (Int) Bool)
      (assert (forall ((x Int)) (p x))) (assert (not (p 0)))''',
}


def run(binary, text, *args):
    return subprocess.run([str(binary), '--lang=smt2', *args], input=text,
                          text=True, capture_output=True, timeout=60)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', type=Path)
    parser.add_argument('--ethos', type=Path)
    args = parser.parse_args()
    for name, body in CASES.items():
        query = '(set-logic ALL)\n' + body + '\n(check-sat)\n'
        result = run(args.binary, query, '--check-proofs', '--dump-proofs')
        assert result.returncode == 0, (name, result.stdout, result.stderr)
        assert result.stdout.startswith('unsat\n'), (name, result.stdout)
        proof = result.stdout[len('unsat\n'):].strip()
        assert proof.startswith('(') and proof.endswith(')'), (name, proof)
        proof = proof[1:-1]
        assert 'step' in proof or 'assume' in proof, (name, proof)
        if args.ethos:
            with tempfile.NamedTemporaryFile(mode='w', suffix='.eo') as f:
                f.write(f'(include "{ROOT}/proofs/eo/cpc/Cpc.eo")\n{proof}\n')
                f.flush()
                checked = subprocess.run(
                    [str(args.ethos), '--require-proof-of-false', f.name],
                    text=True, capture_output=True, timeout=60)
                assert checked.returncode == 0, (name, checked.stdout, checked.stderr)
                assert 'correct' in checked.stdout and 'incomplete' not in checked.stdout, (name, checked.stdout)
        print(f'PASS {name}: solving and CPC proof')
    result = run(args.binary, '''(set-logic QF_LIA)
      (set-option :produce-models true) (set-option :incremental true)
      (declare-const x Int) (assert (= x 7)) (check-sat) (get-value (x))
      (push 1) (assert (= x 8)) (check-sat) (pop 1) (check-sat)''')
    assert result.returncode == 0, result.stderr
    assert result.stdout.splitlines() == ['sat', '((x 7))', 'unsat', 'sat'], result.stdout
    print('PASS models and incremental solving')
    result = run(args.binary, '''(set-logic ALL)
      (set-option :produce-models true) (set-option :incremental true)
      (declare-datatype L ((nil) (cons (head Int) (tail L))))
      (declare-sort U 0) (declare-const a U) (declare-const b U)
      (declare-const x L) (declare-const s (Set L))
      (assert (distinct a b)) (assert (= x (cons 7 nil)))
      (assert (set.member x s)) (assert (not (set.member nil s)))
      (check-sat) (get-value (x))
      (push 1) (assert (= (tail x) x)) (check-sat) (pop 1) (check-sat)''',
      '--check-models', '--debug-check-models')
    assert result.returncode == 0, (result.stdout, result.stderr)
    assert result.stdout.splitlines() == [
        'sat', '((x (cons 7 nil)))', 'unsat', 'sat'], result.stdout
    print('PASS datatype, set and UF models without exclusion sets')
    for option in ['--safe-mode=unrestricted', '--nl-cov', '--sygus',
                   '--produce-abducts', '--produce-interpolants',
                   '--proof-format-mode=alethe', '--proof-format-mode=dot',
                   '--fp', '--ff', '--bags', '--sep', '--arith-exp',
                   '--finite-model-find', '--strings-fmf',
                   '--produce-difficulty', '--dump-difficulty',
                   '--produce-learned-literals', '--timeout-core-timeout=100',
                   '--sat-solver=minisat', '--sat-solver=cadical',
                   '--ee-mode=distributed', '--ee-mode=central',
                   '--dt-shared-sel', '--check-synth-sol', '--model-cores=simple',
                   '--interactive', '--help-option-categories', '--output=normalize',
                   '--nl-cov-linear-model=none', '--cegis-sample=none',
                   '--sub-cbqi-timeout=1', '--elim-taut-quant',
                   '--sat-random-seed=1', '--nl-ext-tf-tplanes']:
        result = run(args.binary, '(check-sat)', option)
        assert result.returncode != 0, (option, result.stdout)
    for text in ['(set-logic HO_ALL)',
                 '(set-logic ALL) (declare-fun f (Int) Int) (assert (= (@ f 0) 0))',
                 '(set-logic ALL) (define-fun f () (-> Int Int) (lambda ((x Int)) x))',
                 '(set-logic ALL) (declare-fun f (Int) Int) (assert (= (set.map f (set.singleton 0)) (set.singleton 0)))',
                 '(set-logic ALL) (declare-fun p (Int) Bool) (assert (= (set.filter p (set.singleton 0)) (set.singleton 0)))',
                 '(set-logic QF_FP)', '(set-logic QF_FF)', '(set-logic QF_UFC)',
                 '(set-logic ALL) (declare-const x (_ FloatingPoint 8 24))',
                 '(set-logic ALL) (declare-const x (Bag Int))',
                 '(set-option :nl-cov true)', '(set-option :sygus true)',
                 '(set-logic ALL) (assert (= (^ 2 3) 8))',
                 '(set-logic ALL) (assert (= (sin 0.0) 0.0))',
                 '(set-logic ALL) (assert (= (set.card (set.singleton 0)) 1))',
                 '(set-logic ALL) (declare-const s (Set Int)) (assert (= s (as set.universe (Set Int))))',
                 '(set-logic ALL) (declare-const s (Set Int)) (assert (= s (set.complement s)))',
                 '(set-logic ALL) (declare-const r (Set (Tuple Int Int))) (assert (= r (rel.transpose r)))',
                 '(set-logic ALL) (declare-const x (Nullable Int))',
                 '(set-logic ALL) (assert (= (select ((as const (Array Int Int)) 0) 1) 0))']:
        result = run(args.binary, text + '\n(check-sat)')
        assert result.returncode != 0 or '(error ' in result.stdout or 'unsupported' in result.stdout.splitlines(), (text, result.stdout)
    print('PASS removed features/options are rejected')
    for command in [
            '(declare-var x Int)', '(synth-fun f () Int)',
            '(synth-inv inv ((x Int)))', '(constraint true)', '(assume true)',
            '(inv-constraint inv pre trans post)', '(check-synth)',
            '(check-synth-next)', '(find-synth :enum)', '(find-synth-next)',
            '(get-qe (exists ((x Int)) (= x 0)))',
            '(get-qe-disjunct (exists ((x Int)) (= x 0)))',
            '(get-abduct A true)', '(get-abduct-next)',
            '(get-interpolant I true)', '(get-interpolant-next)',
            '(declare-pool p Int (0))', '(get-difficulty)',
            '(get-timeout-core)', '(get-timeout-core-assuming (true))',
            '(get-learned-literals)', '(block-model :values)',
            '(block-model-values (true))',
            '(declare-codatatype S ((cons (head Int) (tail S))))',
            '(declare-codatatypes ((S 0)) (((cons (head Int) (tail S)))))']:
        result = run(args.binary, '(set-logic ALL)\n' + command)
        assert result.returncode != 0, (command, result.stdout, result.stderr)
        assert 'command' in (result.stdout + result.stderr).lower(), (
            command, result.stdout, result.stderr)
    print('PASS removed commands are rejected by the parser')
    assert not any((ROOT / 'src/theory' / t).exists() for t in ('fp', 'ff', 'bags', 'sep'))
    assert not any((ROOT / 'test/regress/cli' / d).exists() for d in ('regress3', 'regress4'))
    manifest = (ROOT / 'test/regress/cli/CMakeLists.txt').read_text()
    assert re.search(r'set\(regression_disabled_tests\s*\)', manifest)
    assert (ROOT / 'include/ava6/ava6.h').exists()
    assert not (ROOT / 'include/cvc5').exists()
    for p in (ROOT / 'test').rglob('*.smt2'):
        assert not re.search(r'REQUIRES:[^\n]*(unrestricted-mode|no-safe-mode)', p.read_text()), p
    print('PASS repository scope')


if __name__ == '__main__':
    main()
