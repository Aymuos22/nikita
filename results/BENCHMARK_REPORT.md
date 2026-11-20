# Recorded benchmark report

Measurements recorded on 2026-11-21 (Asia/Kolkata task date).

- Host: Darwin 25.5.0, arm64.
- Compiler: Apple clang version 21.0.0 (clang-2100.0.123.102).
- Release flags: `-std=c++17 -O2 -Wall -Wextra -Wpedantic`.
- Physical memory: 17179869184 bytes.
- Generator seed: 15. Each timed case runs three repetitions in one process.
- Timer: `std::chrono::steady_clock`; CSV stores best and arithmetic mean microseconds.
- Recorded experiments: 122 rows, 36 distinct algorithm variants.

## Same-input comparisons

| Dataset | Algorithm | Size n | Mean µs | Estimated workspace bytes | Sort comparisons |
|---|---|---:|---:|---:|---:|
| small | merge_sort | 128 | 4.180 | 1024 | 735 |
| small | quick_sort | 128 | 4.236 | 160 | 1425 |
| small | dijkstra | 128 | 24.722 | 16312 | 0 |
| small | bellman_ford | 128 | 4.931 | 1536 | 0 |
| small | matcher_0 | 3328 | 5.194 | 32 | 0 |
| small | matcher_3 | 3328 | 7.111 | 72 | 0 |
| medium | merge_sort | 2048 | 76.861 | 16384 | 19923 |
| medium | quick_sort | 2048 | 94.986 | 256 | 43026 |
| medium | dijkstra | 2048 | 407.195 | 262096 | 0 |
| medium | bellman_ford | 2048 | 137.611 | 24576 | 0 |
| medium | matcher_0 | 53248 | 65.166 | 32 | 0 |
| medium | matcher_3 | 53248 | 99.403 | 72 | 0 |
| large | merge_sort | 10000 | 572.527 | 80000 | 120442 |
| large | quick_sort | 10000 | 782.055 | 288 | 238680 |
| large | dijkstra | 10000 | 2870.080 | 1279952 | 0 |
| large | bellman_ford | 10000 | 1010.380 | 120000 | 0 |
| large | matcher_0 | 260000 | 332.111 | 32 | 0 |
| large | matcher_3 | 260000 | 458.986 | 72 | 0 |
| adversarial | merge_sort | 2048 | 62.430 | 16384 | 11264 |
| adversarial | quick_sort | 2048 | 3.722 | 64 | 4096 |
| adversarial | dijkstra | 2048 | 73.778 | 114664 | 0 |
| adversarial | bellman_ford | 2048 | 3083.970 | 24576 | 0 |
| adversarial | matcher_0 | 16384 | 666.292 | 32 | 0 |
| adversarial | matcher_3 | 16384 | 85.806 | 512 | 0 |

## Interpretation

All route variants are compared on the same directed graph; sorting variants receive copies of the same sequence; text variants receive identical text/pattern pairs. Correctness is checked before recording comparisons. Matching variants 0/1/2/3 mean naive/Rabin-Karp/DFA/KMP. Campaign, matrix and spanning-tree alternatives are also checked for agreement where applicable.

The adversarial graph is a reverse-edge-order directed chain, forcing Bellman-Ford to discover one more reachable vertex per pass. The random graph family can favor in-place Bellman-Ford because edge ordering propagates several relaxations in one pass; Dijkstra also pays adjacency construction and heap costs. Observed winners are instance-specific. The adversarial text repeats `a` and uses a long nearly-matching pattern ending in `b`, increasing naive comparisons. The duplicate-only adversarial sorting sequence favors three-way quicksort; this is not a quicksort worst-case pivot construction.

Bounded families scale their own problem sizes, not graph sizes: matrix dimensions 4/6/8; knapsack 10/14/18 items; TSP and N-Queens 4/6/8; subset enumeration 10/12/14 values. Exact solvers are deliberately excluded from 10,000-vertex stress inputs. The raw table contains all mandatory variants and their bounded experiments.

## Measurement limits

`estimated_workspace_bytes` is a rough analytical payload estimate, excluding allocator metadata and often stack-frame/container overhead; some estimates include returned buffers. For branch-and-bound it is a conservative frontier-size proxy, not observed frontier memory. Sort estimates come from the instrumented algorithms; comparisons are counted before timing to avoid instrumentation changing the timed run. A zero count on another algorithm means not instrumented, not zero work.

`process_peak_rss_bytes` is a measured cumulative process high-water mark using `getrusage` on macOS/Unix (0 when unsupported). It includes all earlier experiments, input networks, returned results, runtime and allocator retention. It cannot be interpreted as the measured incremental memory of the row algorithm or used alone to rank algorithm memory. A separate process and allocation profiler would be needed for that claim.

Sorting times include copying inputs so that mutation remains fair; graph timings include local adjacency/table construction. Data generation and verification checks normally run outside timed calls, but B&B and backtracking check their returned objective in the timed wrapper. Result checksums prevent unused computation; floating values are converted to integers for checksums, so the checksum is not a full solution-quality report. CLI scenarios show exact values and fractional bounds separately.

Three repetitions provide a reproducible demonstration, not confidence intervals or a statistical performance claim. Submicrosecond measurements are noisy. No observed timing proves an asymptotic bound. See `docs/COMPLEXITY.md` for the analytical derivations.

Reproduce with `make benchmark` and `python3 scripts/benchmark_report.py results/benchmarks.csv`.
