# Project 15 acceptance checklist

This maps the supplied ten-line brief, expected product and minimum demonstration to executable evidence. The C++17 engine remains the assessed implementation; the GUI is an optional local browser front end.

| Brief item | Implementation and evidence |
|---|---|
| 1. Typed social data, arrays, trees and graphs | Typed User/Topic/Post/Interaction/Edge records; vector storage; ordered ID tree index; separate delay/capacity graph fields. CSV/reference validation in `src/network.cpp`. |
| 2. Influence, communities, dissemination under constraints | Explainable ranking formula, active minimum-delay paths, threshold weak components, top-20 budget-constrained campaigns, integer max flow, assignment eligibility. Overview/Communities/Campaign views expose results and limits. |
| 3. Merge/quick sort, both binary searches and indices | Student algorithms in the library. Loaded-data workbench compares both sorts and searches actual network scores and uses the external-user ID index. |
| 4. Divide and conquer | Maximum subarray on centered interaction strength, powering growth factors, celebrity on bounded active-user relationships, Classical/Strassen two-step projection. All are callable from the library and exercised on loaded inputs. |
| 5. Greedy scheduling, compression, routes and infrastructure | Loaded posts map to declared analytical intervals; activity selection and interval partitioning run on these slots. Huffman compresses loaded text. Dijkstra and Prim/Kruskal run on active graph models. |
| 6. All required DP alternatives | Actual candidate costs/scores for knapsack; degree-derived matrix chain; post-text LCS; engagement-weighted optimal BST; full-network Bellman-Ford; bounded induced-graph Floyd/TSP. Caps and analytical assumptions are printed. |
| 7. Backtracking and exact bound policies | N-Queens remains an explicit syllabus validation exercise. Subsets use actual candidate costs. DP, backtracking and FIFO/LIFO/least-cost B&B use identical candidate items/budget and must agree. |
| 8. Flow and meaningful matching | Ford-Fulkerson on dissemination capacities, with min-cut oracle tests. Creator-topic maximum matching uses topics each creator actually posted about. |
| 9. CRT and text benchmarks | Actual interaction-counter residues are reconstructed modulo 105. All four matchers check loaded post text against a loaded topic keyword. Benchmarks also include repeated-prefix adversarial text. |
| 10. Analytical and empirical reports | `COMPLEXITY.md` contains recurrences, asymptotics, correctness arguments, complexity classes, reductions and strategy guidance. Benchmark and scenario reports include time, measured per-call C++ heap payload and solution-quality gaps. |

## Expected product

| Deliverable | Evidence / run command |
|---|---|
| Portable C/C++ analytics application | C++17 standard-library engine and CLI; `make -j4`. Local GUI adds Python 3 standard library plus a browser. Platform verification is on macOS; cross-platform testing is not asserted. |
| Reusable library | `include/social/algorithms.hpp`, `include/social/network.hpp`, `build/libsocial.a`. Assessed algorithms are locally implemented; permitted containers/heaps support them. |
| CLI/GUI, loading, choice, scenarios and exports | `./build/social_cli`, `make gui`; scenario controls, route/matcher selectors, loaded-data workbench, CSV/JSON downloads. |
| Comparative benchmark runner | `make benchmark`: small/medium/large/adversarial and bounded families, all 36 mandatory variants. `make comparisons`: identical-input comparisons for normal, shock, stress and a controlled quality counterexample. |
| Correctness/regression suite | `make test`: independent C++ oracles, CLI cases, heap-meter invariants and GUI HTTP tests. Optional browser checks in `tests/gui_browser.cjs`. `make sanitize` checks executed C++ paths. |
| Git/build/data/docs/demo/release | Makefile, sample provenance, user/design/complexity/GUI guides, GUI-aware report/deck/run sheet, `scripts/demo.sh`, `scripts/compare_scenarios.py`, historical `v1.0.0` and current `v1.1.0` acceptance release. |

## Minimum demonstration

| Scenario | Input | Strategies compared on that identical input |
|---|---|---|
| Normal | Seed-15, 12 users, budget 35 | Dijkstra/Bellman-Ford; Prim/Kruskal; DP/backtracking/all B&B policies; integer heuristic and fractional bound |
| Capacity/constraint shock | Same data, capacities floor(c/4), budget 10 | Same strategy pairs on the shocked graph and shocked-budget candidate decision |
| Large stress | Seed-15, 10,000 users, budget 35 | Same route/forest/campaign comparisons; flow skipped by its explicit 2,000-user dense-matrix cap |
| Additional adversarial quality fixture | Three creators: costs 10/20/30, scores 60/100/120, budget 50 | Exact feasible value 220; feasible density-greedy value 160; fractional upper bound 240 |

`results/comparisons/REPORT.md` includes measured mean time, largest per-call live C++ heap payload over three calls, objective, exact reference, relative gap and guarantee label. Exact feasible strategies must agree. Fractional bounds are explicitly non-executable for indivisible campaigns. Comparisons within a scenario hold graph, pool and budget fixed; normal and shock are different inputs.

The heap meter excludes stack, inputs allocated before a call, allocator metadata and malloc-only allocations. Cumulative process RSS and analytical workspace estimates remain separate columns. Instrumented benchmark timing has bookkeeping overhead; these measurements are evidence, not universal speed claims.

## Boundaries and remaining human evidence

Algorithm integration uses explicit bounded analytical projections where the syllabus problem is expensive or not naturally a core social feature. N-Queens is a validation problem by design. Campaign optimality concerns an additive score over the top 20 candidates, not global causal influence or deduplicated audience reach. Scheduling durations, OBST miss weights and growth factors are declared analytical assumptions, not inferred platform guarantees.

Named student ownership, human peer-review sign-off and a recorded presentation cannot be supplied by automated code execution. Their placeholders remain honest and unassigned in `OWNERSHIP.md`. They are separate from the implemented software deliverables above.
