# Demonstration script

Run `scripts/demo.sh` from any directory. The script builds the project, demonstrates three scenarios, runs command-level route/matcher comparisons, and records benchmark CSV. Run `make test` separately to show correctness evidence.

## 1. Normal operation

The deterministic 12-user network has technology, education and community posts. Budget 35 selects creators from all 12 eligible candidates. Show the score components printed for the five highest-ranked users; open `results/demo/normal.json` to show each route and creator/topic assignment. Baseline expected values are campaign score **472**, maximum flow **36**, and **1** fast-link weak community.

Explain that Dijkstra and Bellman-Ford calculate the same minimum-delay answers on the same active directed graph. Their wall times can differ even when answers agree. The exact campaign DP and FIFO/LIFO/least-cost branch-and-bound policies return the same optimum; their explored-node counts differ because of frontier order.

## 2. Capacity and budget shock

The same network is analyzed after reducing all capacities to `floor(capacity/4)` and the budget from 35 to 10. Show `results/demo/shock.json`. Expected values: maximum flow **8**, campaign score **265**, and **2** fast-link communities. The historical ranking remains the same because historical degree/engagement records did not change. The active propagation topology changes when capacity becomes zero. The fractional campaign upper bound is **270.6**, above the executable integer optimum 265.

Compare algorithms within the same scenario/input, not normal results against shock results as if they were alternative algorithms. Both shortest-path strategies use exactly the same filtered graph; all exact campaign strategies use exactly the same 12 items and budget.

## 3. Stress case

The generator builds **10,000 users**, 10,000 posts, three topics and about 40,000 directed relationships/interactions with seed 15. Show `results/demo/stress.json`: every user has a ranking, a community and a route result. Verify that Dijkstra/Bellman-Ford still agree. Flow is explicitly `null` because the matrix-based implementation has a 2,000-vertex limit. Campaign selection is exact only within its disclosed top-20 pool. Expected synthetic campaign score is **1233**; changing the generator or seed changes this fixture.

## 4. Syllabus workbench and benchmark explanation

`social_cli workbench` exercises every mandatory implementation, including the separate N-Queens and subset validation exercises. Contrast exact integer campaign value 220 with fractional bound 240 in the standard three-item fixture. Show Classical/Strassen equality for odd dimension three and Huffman roundtrip output.

Open `results/benchmarks.csv` and `results/BENCHMARK_REPORT.md`. Explain increasing problem sizes, the reverse-order chain that forces Bellman-Ford's worst pass pattern, repeated-prefix adversarial text, timing repetitions, counted sorting comparisons, approximate working sets and measured cumulative process RSS. RSS is not per-algorithm memory, so use the analytical space bounds and explicitly labeled estimates to compare algorithms.

## Viva prompts

1. Why must Dijkstra reject negative weights, and what does a reachable negative cycle mean?
2. Why does minimum delay not prove maximum causal influence?
3. Why is the fractional campaign value an upper bound rather than a legal integer plan?
4. Why does `cost=value=a_i`, `B=K=T` give a correct SUBSET SUM reduction?
5. Why is O(kB) pseudo-polynomial, and why must the NP-completeness argument concern unbounded instances?
6. Which bound makes the least-cost B&B pruning safe? Why round it upward?
7. Why can a threshold weak component merge unrelated dense groups through a bridge?
8. What is excluded from the top-20 campaign optimization guarantee?
9. How does a residual minimum cut certify maximum flow?
10. What do recorded benchmark timings establish, and what do they not establish?
