# Same-input scenario comparisons

Generated with `make comparisons`. Each row uses three calls on identical scenario inputs. C++ heap payload is measured separately for each call; the table reports the largest peak and mean time. Source loading, input construction and exact-reference calculation are outside the timed/heap-metered call.

| Scenario | Strategy | Mean microseconds | Peak heap bytes | Value | Exact reference | Relative gap | Interpretation |
|---|---|---:|---:|---:|---:|---:|---|
| normal | dijkstra | 1.208 | 1,392 | 6.000 | 6.000 | 0.00% | exact_distance |
| normal | bellman_ford | 0.181 | 144 | 6.000 | 6.000 | 0.00% | exact_distance |
| normal | prim | 3.333 | 3,208 | 28.000 | 28.000 | 0.00% | exact_forest_cost |
| normal | kruskal | 1.098 | 2,208 | 28.000 | 28.000 | 0.00% | exact_forest_cost |
| normal | knapsack_dp | 0.847 | 4,344 | 472.000 | 472.000 | 0.00% | exact_feasible |
| normal | knapsack_backtracking | 6.861 | 104 | 472.000 | 472.000 | 0.00% | exact_feasible |
| normal | branch_bound_0 | 18.542 | 9,552 | 472.000 | 472.000 | 0.00% | exact_feasible |
| normal | branch_bound_1 | 1.653 | 4,376 | 472.000 | 472.000 | 0.00% | exact_feasible |
| normal | branch_bound_2 | 1.986 | 820 | 472.000 | 472.000 | 0.00% | exact_feasible |
| normal | fractional_knapsack | 0.417 | 240 | 472.000 | 472.000 | 0.00% | upper_bound_not_integer_feasible |
| normal | density_greedy_integer | 0.361 | 240 | 472.000 | 472.000 | 0.00% | heuristic_feasible |
| normal | ford_fulkerson | 1.222 | 1,616 | 36.000 | 36.000 | 0.00% | exact_flow |
| shock | dijkstra | 1.194 | 1,392 | 23.000 | 23.000 | 0.00% | exact_distance |
| shock | bellman_ford | 0.139 | 144 | 23.000 | 23.000 | 0.00% | exact_distance |
| shock | prim | 2.792 | 2,248 | 34.000 | 34.000 | 0.00% | exact_forest_cost |
| shock | kruskal | 1.319 | 2,016 | 34.000 | 34.000 | 0.00% | exact_forest_cost |
| shock | knapsack_dp | 0.556 | 1,544 | 265.000 | 265.000 | 0.00% | exact_feasible |
| shock | knapsack_backtracking | 0.806 | 72 | 265.000 | 265.000 | 0.00% | exact_feasible |
| shock | branch_bound_0 | 3.250 | 4,268 | 265.000 | 265.000 | 0.00% | exact_feasible |
| shock | branch_bound_1 | 1.125 | 4,244 | 265.000 | 265.000 | 0.00% | exact_feasible |
| shock | branch_bound_2 | 1.319 | 820 | 265.000 | 265.000 | 0.00% | exact_feasible |
| shock | fractional_knapsack | 0.389 | 240 | 270.600 | 265.000 | 2.11% | upper_bound_not_integer_feasible |
| shock | density_greedy_integer | 0.319 | 240 | 265.000 | 265.000 | 0.00% | heuristic_feasible |
| shock | ford_fulkerson | 1.222 | 1,616 | 8.000 | 8.000 | 0.00% | exact_flow |
| stress | dijkstra | 1442.540 | 1,196,608 | 56.000 | 56.000 | 0.00% | exact_distance |
| stress | bellman_ford | 546.014 | 120,000 | 56.000 | 56.000 | 0.00% | exact_distance |
| stress | prim | 4684.944 | 2,219,816 | 33559.000 | 33559.000 | 0.00% | exact_forest_cost |
| stress | kruskal | 2315.361 | 1,919,952 | 33559.000 | 33559.000 | 0.00% | exact_forest_cost |
| stress | knapsack_dp | 2.417 | 6,840 | 1233.000 | 1233.000 | 0.00% | exact_feasible |
| stress | knapsack_backtracking | 45.375 | 104 | 1233.000 | 1233.000 | 0.00% | exact_feasible |
| stress | branch_bound_0 | 32.667 | 14,288 | 1233.000 | 1233.000 | 0.00% | exact_feasible |
| stress | branch_bound_1 | 6.222 | 4,412 | 1233.000 | 1233.000 | 0.00% | exact_feasible |
| stress | branch_bound_2 | 8.361 | 1,688 | 1233.000 | 1233.000 | 0.00% | exact_feasible |
| stress | fractional_knapsack | 0.583 | 368 | 1309.667 | 1233.000 | 6.22% | upper_bound_not_integer_feasible |
| stress | density_greedy_integer | 0.903 | 368 | 1189.000 | 1233.000 | 3.57% | heuristic_feasible |
| adversarial_campaign | dijkstra | 0.514 | 172 | 2.000 | 2.000 | 0.00% | exact_distance |
| adversarial_campaign | bellman_ford | 0.111 | 36 | 2.000 | 2.000 | 0.00% | exact_distance |
| adversarial_campaign | prim | 0.667 | 232 | 2.000 | 2.000 | 0.00% | exact_forest_cost |
| adversarial_campaign | kruskal | 0.250 | 192 | 2.000 | 2.000 | 0.00% | exact_forest_cost |
| adversarial_campaign | knapsack_dp | 1.903 | 2,136 | 220.000 | 220.000 | 0.00% | exact_feasible |
| adversarial_campaign | knapsack_backtracking | 0.333 | 56 | 220.000 | 220.000 | 0.00% | exact_feasible |
| adversarial_campaign | branch_bound_0 | 1.528 | 4,172 | 220.000 | 220.000 | 0.00% | exact_feasible |
| adversarial_campaign | branch_bound_1 | 0.875 | 4,172 | 220.000 | 220.000 | 0.00% | exact_feasible |
| adversarial_campaign | branch_bound_2 | 0.958 | 380 | 220.000 | 220.000 | 0.00% | exact_feasible |
| adversarial_campaign | fractional_knapsack | 0.194 | 96 | 240.000 | 220.000 | 9.09% | upper_bound_not_integer_feasible |
| adversarial_campaign | density_greedy_integer | 0.153 | 96 | 160.000 | 220.000 | 27.27% | heuristic_feasible |
| adversarial_campaign | ford_fulkerson | 0.319 | 168 | 10.000 | 10.000 | 0.00% | exact_flow |

## What the results establish

Normal and shock are separate scenarios: all strategies within each scenario receive exactly the same active graph, candidate pool and budget. Stress contains 10,000 users and keeps the same top-20 candidate policy. Flow is omitted in stress because the dense residual solver is limited to 2,000 vertices.

Shortest-path, forest and exact campaign methods agree with their reference objective. The feasible density heuristic can lose quality: in the controlled three-creator case it scores 160 versus exact 220 (27.27% loss). The fractional result 240 is a 9.09% upper-bound gap, not a feasible integer solution. The controlled values map to actual network scores: one incoming edge contributes 10 and interaction strength contributes the remainder (50/90/110); costs are 10/20/30, with budget 50.

The relative gap is abs(value-reference)/max(1,abs(reference)); quality_kind identifies exact feasible solutions, feasible heuristics and fractional bounds so their guarantees are not confused. Input costs and values are not changed between strategies.

## Memory and timing method

The benchmark executable alone replaces C++ new/new[] and delete to count requested payload bytes. Each call starts a new measurement epoch. It includes algorithm allocations and returned buffers, and excludes pre-existing inputs, stack, allocator metadata, allocations performed only through C malloc and any other process. This is measured heap payload, not total resident memory. The wrapper adds allocation bookkeeping, so benchmark times are instrumented and may differ from the CLI. Run order, caches and OS scheduling still affect short measurements.

The ordinary benchmark CSV also retains a labeled analytical workspace estimate and cumulative process RSS; these are distinct from the per-call measured heap column. No subtracted RSS estimate is presented as an algorithm allocation measurement.

## Correctness and production selection

Dijkstra/Bellman-Ford compare complete distance vectors before timing. Prim/Kruskal compare forest weights. Knapsack DP supplies the reference; independent unit tests compare DP, backtracking and every bound policy with exhaustive small-instance oracles. Exact timed wrappers check equality. The domain workbench verifies matrix products, compression roundtrip, bounded all-pairs routes, CRT residues and all four text matchers on loaded records.

For nonnegative delays, prefer Dijkstra for its scalability guarantee; Bellman-Ford remains valuable for signed models and cycle detection. Choose DP when integer budgets fit its table, and exact search only for bounded candidate sets. Use a feasible heuristic only when its observed and worst-case quality limitations are acceptable; fractional allocation is an upper-bound diagnostic for indivisible creator contracts. Preserve the declared model and candidate limits in any production claim.
