# Correctness and complexity report

Use `n` for sequence length, `m` for pattern length, `V/E` for graph vertices/edges, `B` for integer budget, `k` for item count, `F` for integral maximum flow, and `L/R` for bipartite side sizes. Space below excludes caller-owned inputs and returned solutions unless stated. Bounds describe the actual algorithms; instance caps are engineering limits, not improvements to asymptotic complexity.

## Time and auxiliary space

| Algorithm | Best case | Typical/average qualification | Worst case | Auxiliary space |
|---|---|---|---|---|
| Merge sort | Θ(n log n) | Θ(n log n) | Θ(n log n) | Θ(n) + Θ(log n) stack |
| Three-way quicksort | Θ(n), all equal | Expected Θ(n log n) for random order | Θ(n²), deterministic pivot can be adversarial | O(log n) stack: recurse on smaller partition |
| Both binary searches | Θ(1) for empty/constant length | Θ(log n) for this first-position implementation | Θ(log n) | O(1) iterative; Θ(log n) recursive |
| Divide-and-conquer max subarray | Θ(n log n) | Θ(n log n) | Θ(n log n) | Θ(log n) |
| Fast powering | Θ(1) at exponent zero | Θ(log exponent) | Θ(log exponent) checked arithmetic steps | Θ(1) |
| Celebrity with verification | Θ(n) | Θ(n) | Θ(n) | Θ(1), supplied matrix excluded |
| Classical matrix product | Θ(n³) | Θ(n³) | Θ(n³) | Θ(n²) result |
| Strassen | Θ(n^log₂7) | Θ(n^2.807…) | Θ(n^log₂7), padding <2n | O(n²) live auxiliary matrices |
| Activity selection | Θ(n log n) including sort | Θ(n log n) | Θ(n log n) | O(n) |
| Interval partitioning | Θ(n log n) including sort | Θ(n log n) | Θ(n log n) | O(n) |
| Fractional knapsack | Θ(k log k) including sort | Θ(k log k) | Θ(k log k) | O(k) |
| Huffman | Ω(n), input scan | O(n + σ log σ + output bits) | O(n + σ log σ + output bits) | O(σ² + output bits), σ≤256 |
| Dijkstra with lazy heap | Ω(V+E), adjacency construction | O((V+E) log(E+1)) | O((V+E) log(E+1)) | O(V+E), including stale heap entries |
| Bellman-Ford | Θ(V+E) when first passes stabilize | No universal average bound | O(VE + V) | O(V) |
| Floyd-Warshall | Θ(V³) | Θ(V³) | Θ(V³) | Θ(V²) |
| Prim, lazy edge heap | Ω(V+E) | O(V+E log(E+1)) | O(V+E log(E+1)) | O(V+E) |
| Kruskal | Θ(E log E + V) for sorting/init | O(E log E + E α(V) + V) | Same upper bound | O(V+E) |
| Knapsack DP | Θ(k(B+1)) | Θ(k(B+1)) | Θ(k(B+1)); pseudo-polynomial | Θ(k(B+1)) reconstruction table |
| Matrix-chain DP | Θ(k³) | Θ(k³) | Θ(k³) | Θ(k²) |
| LCS reconstruction | Θ(nm) for nonempty inputs | Θ(nm) | Θ(nm) | Θ(nm) |
| Optimal BST DP | Θ(k³) | Θ(k³) | Θ(k³) | Θ(k²) |
| Held-Karp TSP | Θ(k²2^k) loops in dense instances | O(k²2^k) | O(k²2^k) | Θ(k2^k) |
| N-Queens enumeration | Ω(number of emitted cells) | Depends on n and pruning | O(n² n!) upper bound for safety scans | O(n) stack plus stored solutions |
| Subset enumeration | Θ(2^k), visits full binary tree | Θ(2^k) plus output copies | O(k2^k) including emitted indices | O(k) stack plus output |
| Knapsack backtracking | Ω(k), if nothing fits | Depends on capacities | O(k2^k), includes solution-vector copies | O(k) stack/current/best selections |
| FIFO branch and bound | Ω(k log k), initial ratio sort | Depends on bound tightness | O(k²2^k) upper bound, vector copies and bounds | O(k2^k) worst stored frontier; cap enforced |
| LIFO branch and bound | Ω(k log k), initial ratio sort | Depends on bound tightness | O(k²2^k) upper bound | O(k²), O(k) pending nodes each storing O(k) indices |
| Least-cost branch and bound | Ω(k log k) | Depends on bound tightness | O(k²2^k), heap overhead included | O(k2^k), cap enforced |
| Ford-Fulkerson, DFS matrix | Ω(V²+E), initialization and failed search | Capacity-dependent | O(E+V²(F+1)), each augmentation raises flow ≥1 | Θ(V²) |
| Bipartite augmenting paths | Ω(L+R+E) | O(L(E+R)) | O(L(E+R)) | O(L+R), apart from input adjacency |
| CRT iterative reconstruction | Ω(k) | O(k log M) word-arithmetic steps | O(k log M), bounded word moduli | O(1), input excluded |
| Naive matching | Θ(n-m+1) comparisons when first symbols differ | O(nm), distribution-dependent | Θ(nm) | O(1) plus matches |
| Rabin-Karp | Θ(n+m) | Expected Θ(n+m) with few candidate verifications | Θ(nm), collisions or repeated matches | O(1) plus matches |
| DFA matching | Θ(256m+n) | Θ(256m+n) | Θ(256m+n) | Θ(256m) |
| KMP | Θ(n+m) | Θ(n+m) | Θ(n+m) | Θ(m) |

For empty patterns all matchers output n+1 positions, requiring Θ(n) output. For m>n they exit before preprocessing. `σ` is the number of distinct bytes. Huffman stores its bit string as one ASCII byte per bit, so theoretical bit savings are not physical file savings. Matrices and numerical modules enforce bounds stated in the design guide. The finite input limits keep represented integer results within 64 bits, with explicit overflow errors for powering and CRT.

## Recurrence solutions

**Merge sort**, for n a power of two, has `T(n)=2T(n/2)+cn`, `T(1)=d`.

- Iteration: after j expansions, `T(n)=2^j T(n/2^j)+jcn`. Put `j=log₂ n`: `dn+cn log₂n`, hence Θ(n log n).
- Substitution: assume `T(n/2) ≤ A(n/2)log₂(n/2)+B(n/2)`. Substitution gives `T(n) ≤ An log₂n + Bn + (c-A)n`; choose `A≥c`. A matching lower induction establishes Ω(n log n).
- Recursion tree: level j has 2^j subproblems of size n/2^j, each with linear nonrecursive work; each level costs cn and there are log₂n internal levels plus n constant leaves.
- Master Theorem: a=2, b=2, f(n)=Θ(n)=Θ(n^log_b a), balanced case gives Θ(n log n).

**Binary search:** `T(n)=T(ceil(n/2))+Θ(1)`; iteration halves n until one, giving Θ(log n). The recursion tree is one chain; Master case with a=1, b=2, f=Θ(1) also gives Θ(log n), ignoring rounding. Recursive implementation uses this many frames; iterative has constant auxiliary space.

**Maximum subarray:** `T(n)=2T(n/2)+Θ(n)` because the crossing window scans both halves. It has the same solution as merge sort. Kadane's linear scan would be preferable for a production peak-window query; the assessed implementation intentionally demonstrates divide and conquer.

**Strassen:** `T(n)=7T(n/2)+Θ(n²)`. Since n² grows polynomially slower than n^log₂7, Master case 1 gives Θ(n^log₂7). The recursion tree's level-j work is Θ(n²(7/4)^j); deepest levels dominate. Padding to p with n≤p<2n changes constants, not the exponent. The classic kernel takes Θ(n³).

**Fast powering:** conceptual recursive recurrence `T(e)=T(floor(e/2))+Θ(1)` solves to Θ(log e); the iterative implementation maintains the same squaring sequence with constant space.

**Quicksort:** general recurrence is `T(n)=T(a)+T(n-a-b)+Θ(n)` where b is the pivot-equal partition size. Balanced distinct partitions give Θ(n log n); an all-equal input gives Θ(n); repeatedly extreme partitions give `T(n)=T(n-1)+Θ(n)=Θ(n²)` by summing n+(n-1)+…+1. The standard Master Theorem does not apply to arbitrary unequal splits or the n-1 recurrence.

**Subset enumeration:** `T(k)=2T(k-1)+Θ(1)`, excluding emitted vector copies, expands to Θ(2^k). This is a decrement recurrence; applying the divide-by-b Master Theorem would be invalid. Backtracking knapsack has the same upper search-tree bound before capacity pruning. Branch-and-bound does not guarantee polynomial work.

## Correctness evidence

| Family | Invariant or argument | Executed independent evidence |
|---|---|---|
| Sorting/search | Merged prefix contains smallest remaining elements; quicksort partitions `<,=,>`; binary range contains first feasible match | Random arrays vs `std::sort`/`lower_bound`, stable equal-key ordering |
| Peak window | Every optimal interval lies left, right, or crosses the midpoint | Exhaustive interval sums |
| Celebrity/powering | Elimination discards a provably non-celebrity, then verifies; `result × base^remaining` invariant | Missing/known celebrity, negative bases, boundary overflow |
| Matrices | Quadrant identities expand to the same product | Signed random matrices, dimensions 0..9 including non-powers of two |
| Scheduling | Earliest finishing exchange preserves optimal cardinality; any new room is justified by overlapping active intervals | Conflict and channel reuse tests |
| Huffman | Combine two least frequent trees; prefix-free paths decode uniquely | Empty/singleton/binary text roundtrips, malformed prefix rejection |
| Knapsack | DP considers include/exclude; exact tree searches enumerate feasible choices; fractional upper bounds never underestimate descendants | Exhaustive subsets vs DP/backtracking/all three B&B policies; cost/index reconstruction |
| Shortest paths | Dijkstra settles minimum nonnegative tentative delay; Bellman passes cover increasingly long paths; Floyd adds permitted intermediate vertices | Random graphs cross-check all sources, negative edges and cycles, unreachable routes |
| MST | Cut-safe edge choices; DSU prevents cycles | Prim/Kruskal weight and component agreement on random disconnected graphs |
| TSP | State `(mask,last)` contains cheapest path using exactly mask | Independent tour permutation enumeration; absent-tour case |
| Flow | Residual augmentation preserves feasibility and conservation; absence of residual s→t path certifies a cut | Every source/sink cut enumerated on random small graphs; returned cut equals flow |
| Matching | Alternating augmentation increases cardinality; absence of augmenting path is optimal | Independent recursive assignment oracle and uniqueness/eligibility checks |
| Text | Exact comparisons, verified rolling hashes, suffix-state DFA, prefix fallback KMP | Independent substring positions on random byte strings; overlaps and empty pattern |
| CRT | Each merge preserves existing congruences and satisfies the next by a modular inverse | All residues for 0..104 modulo 3/5/7; negative residues, noncoprime rejection |
| Other DP/constraints | Optimal substructure for chains/LCS/BST; row-by-row nonattack condition; full subset inclusion/exclusion | Known costs/sequences, 4/8-Queens counts, signed subset fixtures |

Unit and integration tests use fixed seeds. Sanitizers check executed memory/arithmetic paths, not all possible paths. Recorded results appear in `results/VALIDATION.txt`. Human peer review is still a student deliverable, not asserted by this report.

## Complexity classes and a reduction

Minimum-delay reachability within a bound with nonnegative integer delays is in P (Dijkstra plus compare). Integer max-flow threshold and matching-cardinality threshold decisions are in P. Minimum spanning-tree weight threshold is in P. These are also in NP since P ⊆ NP; they are not claimed to be NP-complete.

Consider the unbounded candidate version of the campaign decision: *is there a subset with total cost at most B and total value at least K?* A list of chosen indices is a polynomial-size certificate; summing binary-encoded costs/values and checking both bounds takes polynomial time, so it is in NP.

Reduce **SUBSET SUM** with positive integers `a₁,…,a_k` and target T to campaign selection by creating item i with `cost_i=value_i=a_i` and setting `B=K=T`. A campaign is feasible exactly when its sum is both ≤T and ≥T, hence exactly T. The mapping copies k integers and a target, has polynomial size/time, and preserves yes/no answers in both directions. Consequently the general campaign decision is weakly NP-complete and its optimization version is NP-hard. The O(kB) DP is pseudo-polynomial because B needs only O(log B) bits in the input.

This reduction concerns arbitrary unbounded mathematical instances, not the program's fixed top-20 pool, numeric bounds, or its 10-million-cell table cap. Fixed-size capped instances do not establish asymptotic NP-hardness. Likewise Hamiltonian-cycle-to-TSP decision can use a complete graph with original arcs costing 1, other arcs costing 2, and threshold k; a tour costs ≤k exactly when every chosen arc belongs to the original graph. TSP decision is NP-complete and optimization NP-hard in the general unbounded model; our ≤18-vertex solver is a restricted exact experiment.

## Recommended execution strategy and practical limits

Use stable merge sort for explainable deterministic rankings; the implemented three-way quicksort is useful for many equal values, but has a quadratic worst case. Use adjacency-list Dijkstra for nonnegative propagation delays; reserve Bellman-Ford for signed analytical costs, and Floyd-Warshall for bounded dense all-pairs queries. Validate the input rather than treating negative cycles as valid influence rankings.

Use DP for modest integer campaign budgets and branch-and-bound to study exact search on small candidate sets. Fractional allocation is only a relaxation for indivisible creator contracts. Exact campaign value is an additive surrogate; audience overlap and causal influence require a different model. Communities are threshold components and can collapse through one qualifying bridge. The threshold is an exposed modeling assumption in JSON, fixed at 5 in this release.

Use KMP for general text streams when bounded preprocessing is desirable; Rabin-Karp retains exact verification and may degrade on many candidates. DFA preprocessing has a substantial 256-symbol memory constant. Use Prim/Kruskal for the declared undirected infrastructure model, not directed dissemination trees. Ford-Fulkerson's capacity-dependent DFS behavior and dense residual storage limit its production scalability; a future sparse flow implementation with a capacity-independent augmentation policy would improve it.

Measured benchmark comparisons, including cases where Bellman-Ford beats Dijkstra on favorable edge order, are discussed in `results/BENCHMARK_REPORT.md`. A better asymptotic guarantee does not imply faster time for every measured instance.

## Ethics and limitations

Use consented or synthetic data. Degree and engagement can reflect spam, coordinated behavior or unequal access; the ranking is not a person-quality judgment. Reports may reveal relationships and should be shared only with appropriate consent. The application does not infer protected traits, scrape platforms, post content or contact users. Numeric capacity/delay units are synthetic scenario units, not measured platform guarantees. Reproducible data and explicit formulas support auditing, not causal claims.


## GUI and presentation costs

The optional browser GUI does not change any engine complexity bound. Python handles HTTP, input snapshots, CSV metadata and subprocess orchestration. Response size includes all exported rankings and source paths, so it depends on the total number of emitted path vertices as well as user count. Large full-network reports can still be expensive even when the visual graph is sampled.

Graph visualization uses at most 102 nodes, up to 1,000 active input edges and 150 pairwise layout iterations, then collapses parallel lines for display. This bounds interactive rendering work; it does not limit the network used by C++ rankings, routes, communities or campaign candidate selection. The GUI supports at most 10,000 users / 12 MB per import, paginates rankings and communities, and enforces 90-second engine-job timeouts. A timeout or instance-limit rejection is an error, not a partial optimum. Dedicated GUI HTTP tests and optional real-browser checks verify the wrapper separately from the algorithm proofs.
