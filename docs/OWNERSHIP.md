# Eight-week execution and individual ownership

This repository contains an AI-assisted initial implementation. No student names, human reviews, issue discussions, or past development milestones have been fabricated. Students should study, adapt and defend the algorithms they own, in accordance with their institution's rules. The assignments below are a plan; fill actual names and evidence before academic submission.

| Week | Milestone | Review evidence to collect |
|---|---|---|
| 1 | Confirm influence model, privacy rules, CSV schema, datasets and constraints | Approved requirements, fixtures, architecture review |
| 2 | Sorting/search, divide and conquer, recurrences | Individual traces, comparison counts and recurrence derivations |
| 3 | Scheduling, fractional allocation, Huffman, Dijkstra, MST | Greedy exchange/cut arguments, roundtrips and route comparisons |
| 4 | DP families and exact campaign integration | State transitions, reconstruction traces and brute-force checks |
| 5 | N-Queens, subsets, backtracking, FIFO/LIFO/least-cost B&B | Bounds/pruning proof, node counts, capacity violations tested |
| 6 | Flow, matching, CRT, all text matchers | Min-cut evidence, assignment eligibility, congruences and overlaps |
| 7 | Integrate, stress-test, analyze reductions and limitations | Benchmark CSV, complexity review, regression results |
| 8 | Final documentation, viva, reproducible demonstration and release | Human code review, individual defense and tagged release |

## Algorithm ownership register

Assign a named owner to **each algorithm**, even when one student owns multiple rows/groups. Each owner must explain the implemented code, trace an example, derive complexity, state limitations and show an independent correctness test. Split the groups below into individual rows when assigning students.

| Module | Algorithms to assign individually | Owner | Reviewer | Evidence |
|---|---|---|---|---|
| Foundations | Merge, quick, both binary searches, maximum subarray, powering, celebrity, classical, Strassen | Unassigned | Unassigned | Pending student defense |
| Greedy | Activity selection, interval partitioning, fractional knapsack, Huffman, Dijkstra, Prim, Kruskal | Unassigned | Unassigned | Pending student defense |
| DP | Knapsack, chain order, LCS, OBST, Bellman-Ford, Floyd-Warshall, Held-Karp | Unassigned | Unassigned | Pending student defense |
| Search | N-Queens, subsets, backtracking knapsack, each of FIFO/LIFO/least-cost B&B | Unassigned | Unassigned | Pending student defense |
| Capacity | Ford-Fulkerson, maximum matching | Unassigned | Unassigned | Pending student defense |
| Number/text | CRT, naive, Rabin-Karp, DFA, KMP | Unassigned | Unassigned | Pending student defense |

## Local issue register

These are handoff issues; they are not external GitHub tickets.

| ID | Work | Status |
|---|---|---|
| DAA-01 | Implement all mandatory algorithms and reusable APIs | Implemented; automated checks recorded |
| DAA-02 | Integrate ranking, communities, paths, campaigns, flow and matching | Implemented with documented size/model limits |
| DAA-03 | Demonstrate normal, shock and stress scenarios | Implemented and regression-tested |
| DAA-04 | Collect benchmarks and report methodology/limits | Implemented; raw data included |
| DAA-05 | Assign named students and record individual algorithm explanations | Pending student action |
| DAA-06 | Conduct human peer code review and record findings | Pending student action |
| DAA-07 | Record live/video demonstration | GUI/CLI script provided; recording pending student action |
| DAA-08 | Add local GUI and update presentation materials | Implemented; GUI/API and browser workflows verified |
| DAA-09 | Close loaded-data and measured-memory/quality comparison gaps | Implemented; bounded mappings and same-input scenario evidence recorded |

## Review record

Automated review includes warning-enabled C++17 builds, exhaustive small-instance oracles for optimization/flow/matching/TSP, randomized comparisons, malformed-input checks, an end-to-end stress case, and address/undefined-behavior sanitizers. The GUI update adds eight grouped HTTP tests and 64 real-browser checks for local imports, scenarios, visual interaction, algorithms, downloads and mobile layouts. During self-review, the benchmark's accidental C++20 structured-binding capture was removed; max-subarray oracles were changed to test unsorted source sequences; arithmetic input bounds and explicit search-frontier/output limits were added. This is an automated/self-review record, not a human code-review sign-off.

For further work, create a branch such as `student/<name>/<algorithm>`, make meaningful commits, request another student's review, and record the commit hash and review decision against each owned algorithm. This starter release has its own actual Git commits and tag; it does not represent eight weeks of human collaboration.
