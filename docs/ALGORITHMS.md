# Mandatory algorithm coverage

All implementations are exposed by `include/social/algorithms.hpp`, exercised in `workbench`, and benchmarked. Tests use standard-library or exhaustive reference oracles only in test code. Both the CLI and GUI use the same C++ implementations. The GUI exposes route and text strategy selectors, campaign/community results and a complete workbench action; it does not replace the algorithms with browser implementations. The loaded-data workbench maps every required family onto the selected network or text, using explicit small projections for expensive analytical tasks. N-Queens remains the requested standalone validation exercise. The no-argument workbench retains fixed regression examples.

| Required algorithm | Implementation/API | Social-network mapping and evidence |
|---|---|---|
| Merge sort | header `merge_sort` | Stable influence ranking; random and stability oracle tests |
| Quick sort | header `quick_sort` | Alternative ordering of activity scores; three-way duplicate handling |
| Iterative binary search | foundations `binary_search_iterative` | Lookup in sorted engagement/ID arrays; first duplicate position |
| Recursive binary search | foundations `binary_search_recursive` | Same lookup; identical lower-bound contract |
| Maximum subarray | foundations `maximum_subarray` | Highest net activity window; divide-and-conquer crossing calculation |
| Fast exponentiation | foundations `fast_power` | Integer scenario-growth factors; checked overflow |
| Celebrity | foundations `celebrity` | Universal-followed account validation on a directed knows matrix |
| Classical matrix multiplication | foundations `classical_multiply` | Two-step interaction projection |
| Strassen multiplication | foundations `strassen_multiply` | Same projection with seven recursive products and padding |
| Activity selection | foundations `activity_selection` | Maximum count of compatible post slots |
| Interval partitioning | foundations `interval_partition` | Minimum concurrent publication/moderation channels |
| Fractional knapsack | optimization `fractional_knapsack` | Divisible promotion allocation and integer campaign upper bound |
| Huffman coding | foundations `huffman_encode/decode` | Lossless domain-text compression exercise |
| Dijkstra | graph `dijkstra` | Integrated nonnegative minimum-delay dissemination paths |
| Prim | graph `prim` | Undirected dissemination infrastructure forest comparison |
| Kruskal | graph `kruskal` | Integrated undirected infrastructure forest weight |
| 0/1 knapsack DP | optimization `knapsack_dp` | Integrated cost-limited additive campaign selection |
| Matrix chain | optimization `matrix_chain` | Cheapest order of chained analytics matrix projections |
| LCS | optimization `lcs` | Common topic/event subsequence between streams |
| Optimal BST | optimization `optimal_bst` | Weighted key/topic lookup cost including misses |
| Bellman-Ford | graph `bellman_ford` | Same-input route comparison; signed analytical graphs and cycle detection |
| Floyd-Warshall | graph `floyd_warshall` | Small-network all-pairs route queries |
| Bounded TSP | graph `tsp` | Small ambassador outreach tours; Held-Karp exact DP |
| N-Queens | optimization `n_queens` | Explicit syllabus constraint-validation exercise |
| Sum of subsets | optimization `subset_sum` | Exact campaign budget combination enumeration, including signed adjustments |
| Knapsack backtracking | optimization `knapsack_backtracking` | Exhaustive feasible campaign decisions with capacity pruning |
| FIFO branch and bound | optimization `knapsack_branch_bound(...FIFO)` | Exact campaign search with queue frontier |
| LIFO branch and bound | optimization `knapsack_branch_bound(...LIFO)` | Same search with stack frontier |
| Least-cost branch and bound | optimization `knapsack_branch_bound(...LeastCost)` | Same search ordered by negative profit upper bound |
| Ford-Fulkerson | graph `ford_fulkerson` | Integrated integer dissemination capacity and minimum-cut evidence |
| Maximum bipartite matching | graph `bipartite_matching` | Integrated unique creator-to-posted-topic assignment |
| Chinese Remainder Theorem | text `chinese_remainder` | Reconstruct partitioned counters; residues 2 mod 3, 3 mod 5, 2 mod 7 recover 23 |
| Naive string matching | text `match(...Naive)` | Domain keyword/event sequence baseline |
| Rabin-Karp | text `match(...RabinKarp)` | Rolling hash with exact collision verification |
| Finite-automaton matching | text `match(...Automaton)` | Complete byte transition table using prefix fallback |
| KMP | text `match(...KMP)` | Linear prefix-based domain keyword matching |

The `std::map` ID index is permitted supporting infrastructure, not a substitute for optimal-BST DP. `std::priority_queue` supplies heaps for Dijkstra, Prim, Huffman and bound ordering; the algorithms' relaxations, merges, pruning and searches are implemented here. Matrix and graph algorithms do not call external numerical libraries.


Run `./build/social_cli workbench data/sample 35` or the GUI workbench to use actual records. Scheduling maps first interaction timestamps and text lengths to declared analytical slots. Matrix operations use at most eight users; LCS uses the first two users' text; OBST weights derive from engagement. CRT reconstructs the actual interaction counter modulo 105. The workbench prints every projection and assumption. Full-network routes, ranking and campaign inputs retain their original contracts.

Use `make comparisons` or **Compare this network** for per-strategy elapsed time, measured peak heap payload, exact references and quality gaps. The feasible density-greedy comparator is an additional diagnostic, not a replacement for any mandatory implementation.
