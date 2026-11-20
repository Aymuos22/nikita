"""Create a compact Markdown report from recorded benchmark rows (stdlib only)."""
import csv
import pathlib
import platform
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[1]
source = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else root / 'results/benchmarks.csv'
rows = list(csv.DictReader(source.open()))
by_key = {(row['dataset'], row['algorithm']): row for row in rows}
compiler = subprocess.check_output(['c++', '--version'], text=True).splitlines()[0]
try:
    memory = subprocess.check_output(['sysctl', '-n', 'hw.memsize'], text=True).strip()
except (OSError, subprocess.CalledProcessError):
    memory = 'not collected'
report = [
    '# Recorded benchmark report', '',
    'Measurements recorded on 2026-11-21 (Asia/Kolkata task date).', '',
    f'- Host: {platform.system()} {platform.release()}, {platform.machine()}.',
    f'- Compiler: {compiler}.',
    '- Release flags: `-std=c++17 -O2 -Wall -Wextra -Wpedantic`.',
    f'- Physical memory: {memory} bytes.',
    '- Generator seed: 15. Each timed case runs three repetitions in one process.',
    '- Timer: `std::chrono::steady_clock`; CSV stores best and arithmetic mean microseconds.',
    f'- Recorded experiments: {len(rows)} rows, {len(set(r["algorithm"] for r in rows))} distinct algorithm variants.', '',
    '## Same-input comparisons', '',
    '| Dataset | Algorithm | Size n | Mean µs | Estimated workspace bytes | Sort comparisons |',
    '|---|---|---:|---:|---:|---:|',
]
for dataset in ('small', 'medium', 'large', 'adversarial'):
    for algorithm in ('merge_sort', 'quick_sort', 'dijkstra', 'bellman_ford', 'matcher_0', 'matcher_3'):
        row = by_key[dataset, algorithm]
        report.append(f'| {dataset} | {algorithm} | {row["n"]} | {float(row["mean_us"]):.3f} | '
                      f'{row["estimated_workspace_bytes"]} | {row["counted_sort_comparisons"] or "—"} |')
report += [
    '', '## Interpretation', '',
    'All route variants are compared on the same directed graph; sorting variants receive copies '
    'of the same sequence; text variants receive identical text/pattern pairs. Correctness is '
    'checked before recording comparisons. Matching variants 0/1/2/3 mean naive/Rabin-Karp/DFA/KMP. '
    'Campaign, matrix and spanning-tree alternatives are also checked for agreement where applicable.', '',
    'The adversarial graph is a reverse-edge-order directed chain, forcing Bellman-Ford to discover '
    'one more reachable vertex per pass. The random graph family can favor in-place Bellman-Ford '
    'because edge ordering propagates several relaxations in one pass; Dijkstra also pays adjacency '
    'construction and heap costs. Observed winners are instance-specific. The adversarial text '
    'repeats `a` and uses a long nearly-matching pattern ending in `b`, increasing naive comparisons. '
    'The duplicate-only adversarial sorting sequence favors three-way quicksort; this is not a '
    'quicksort worst-case pivot construction.', '',
    'Bounded families scale their own problem sizes, not graph sizes: matrix dimensions 4/6/8; '
    'knapsack 10/14/18 items; TSP and N-Queens 4/6/8; subset enumeration 10/12/14 values. '
    'Exact solvers are deliberately excluded from 10,000-vertex stress inputs. The raw table '
    'contains all mandatory variants and their bounded experiments.', '',
    '## Measurement limits', '',
    '`estimated_workspace_bytes` is a rough analytical payload estimate, excluding allocator '
    'metadata and often stack-frame/container overhead; some estimates include returned buffers. '
    'For branch-and-bound it is a conservative frontier-size proxy, not observed frontier memory. '
    'Sort estimates come from the instrumented algorithms; comparisons are counted before timing '
    'to avoid instrumentation changing the timed run. A zero count on another algorithm means '
    'not instrumented, not zero work.', '',
    '`process_peak_rss_bytes` is a measured cumulative process high-water mark using `getrusage` '
    'on macOS/Unix (0 when unsupported). It includes all earlier experiments, input networks, '
    'returned results, runtime and allocator retention. It cannot be interpreted as the measured '
    'incremental memory of the row algorithm or used alone to rank algorithm memory. A separate '
    'process and allocation profiler would be needed for that claim.', '',
    'Sorting times include copying inputs so that mutation remains fair; graph timings include '
    'local adjacency/table construction. Data generation and verification checks normally run '
    'outside timed calls, but B&B and backtracking check their returned objective in the timed '
    'wrapper. Result checksums prevent unused computation; floating values are converted to '
    'integers for checksums, so the checksum is not a full solution-quality report. CLI scenarios '
    'show exact values and fractional bounds separately.', '',
    'Three repetitions provide a reproducible demonstration, not confidence intervals or a '
    'statistical performance claim. Submicrosecond measurements are noisy. No observed timing '
    'proves an asymptotic bound. See `docs/COMPLEXITY.md` for the analytical derivations.', '',
    'Reproduce with `make benchmark` and '
    '`python3 scripts/benchmark_report.py results/benchmarks.csv`.', '',
]
(root / 'results/BENCHMARK_REPORT.md').write_text('\n'.join(report))
print(f'Wrote benchmark report for {len(rows)} cases')
