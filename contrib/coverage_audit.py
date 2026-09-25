#!/usr/bin/env python3
"""Summarize GCC JSON coverage without third-party Python packages.

Run after an instrumented build and tests. Uncovered code is an audit lead,
not evidence that it can be deleted. Missing .gcda files count as zero coverage.
Generated sources and dependencies outside src/ and include/ are excluded.
"""

import argparse
from concurrent.futures import ThreadPoolExecutor
import csv
import json
from pathlib import Path
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('--source', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--output', type=Path)
    parser.add_argument('--gcov', default='gcov')
    parser.add_argument('--jobs', type=int, default=8)
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    root = args.source.resolve()
    build = args.build.resolve()
    output = (args.output or build / 'coverage-audit').resolve()
    output.mkdir(parents=True, exist_ok=True)
    notes = sorted((build / 'src').rglob('*.gcno'))
    if not notes:
        parser.error('no .gcno files found under build/src; compile with --coverage first')

    def collect(note):
        proc = subprocess.run([args.gcov, '--json-format', '--stdout', str(note)],
                              capture_output=True, text=True, check=True)
        data = json.loads(proc.stdout)
        selected = []
        for entry in data['files']:
            path = Path(entry['file'])
            if not path.is_absolute():
                path = Path(data['current_working_directory']) / path
            try:
                relative = path.resolve().relative_to(root)
            except ValueError:
                continue
            if relative.parts[0] in ('src', 'include'):
                selected.append((str(relative), entry))
        return selected

    lines = {}
    functions = {}
    # Merge header/template instantiations across translation units. A source
    # line/function is covered if ANY emitted instance executes.
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for i, entries in enumerate(pool.map(collect, notes), 1):
            for path, entry in entries:
                source_lines = lines.setdefault(path, {})
                for line in entry['lines']:
                    n = line['line_number']
                    source_lines[n] = source_lines.get(n, False) or line['count'] > 0
                for function in entry['functions']:
                    name = function.get('demangled_name', function['name'])
                    key = (path, function['start_line'], name)
                    executed = function['execution_count'] > 0
                    if key in functions:
                        functions[key]['executed'] |= executed
                    else:
                        functions[key] = dict(path=path, name=name,
                            start=function['start_line'], end=function['end_line'],
                            executed=executed)
            if i % 50 == 0:
                print(f'Read {i}/{len(notes)} translation units', flush=True)

    rows = []
    for path, covered in sorted(lines.items()):
        hit = sum(covered.values())
        total = len(covered)
        rows.append(dict(path=path, covered_lines=hit, executable_lines=total,
                         coverage_percent=round(100 * hit / total, 2) if total else 0))
    if not rows or not any(row['executable_lines'] for row in rows):
        parser.error('no executable src/ or include/ lines found; check --source')
    with (output / 'files.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    uncalled = [f for f in functions.values() if not f['executed']]
    uncalled.sort(key=lambda f: (-(f['end'] - f['start']), f['path'], f['start']))
    with (output / 'uncalled-functions.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=['path', 'start', 'end', 'name'])
        writer.writeheader()
        writer.writerows({k: f[k] for k in writer.fieldnames} for f in uncalled)
    total = sum(row['executable_lines'] for row in rows)
    hit = sum(row['covered_lines'] for row in rows)
    summary = dict(source=str(root), build=str(build), translation_units=len(notes),
                   units_without_runtime_data=sum(not p.with_suffix('.gcda').exists() for p in notes),
                   source_files=len(rows), executable_lines=total, covered_lines=hit,
                   line_coverage_percent=round(100 * hit / total, 2),
                   functions=len(functions), uncalled_functions=len(uncalled))
    (output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    (output / 'coverage.json').write_text(json.dumps(dict(lines=lines,
                                         functions=list(functions.values()))) + '\n')
    print(json.dumps(summary, indent=2))
    print(f'Reports: {output}')


if __name__ == '__main__':
    main()
