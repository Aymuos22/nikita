# Social Network Influence and Community Analysis Tool

A portable C++17 implementation of DAA Project 15: network analytics, explainable influence scores, community grouping, constrained campaign selection, and a complete syllabus algorithm workbench. No runtime dependencies beyond the C++ standard library.

## Build and run

Requires a C++17 compiler, Make, and Python 3 for integration tests. On older GCC versions, filesystem support may require `LDFLAGS=-lstdc++fs`.

```sh
make -j4
make test
./build/social_cli
```

The menu loads CSV datasets, generates synthetic networks, runs normal/shock analysis, exports CSV/JSON, and runs the algorithm workbench. Press `0` to exit. Commands also support repeatable demonstrations:

```sh
./build/social_cli analyze data/sample 35 results/sample
./build/social_cli analyze data/sample 10 results/shock shock
./build/social_cli route data/sample 0 11 dijkstra
./build/social_cli route data/sample 0 11 bellman
./build/social_cli route data/sample 0 11 floyd
./build/social_cli match kmp 'community community campaign' community
./build/social_cli workbench
make demo
make benchmark
```

`match` accepts `naive`, `rk`, `fa`, or `kmp`; positions are zero-based byte offsets and overlaps are included. `generate USERS DIRECTORY` writes a reproducible dataset (seed 15). `demo` runs normal operation, a capacity/budget shock, and a 10,000-user stress case on deterministic data. `scripts/demo.sh` runs the complete presentation. `make sanitize` rebuilds and tests with address and undefined-behavior sanitizers; then use `make clean && make -j4` to restore optimized binaries.

## What the application measures

- Influence score: **10 × incoming relationship count + incoming interaction strength**. The exported factors explain each score; this is a transparent heuristic, not a causal influence estimate.
- Communities: weak components of active relationships with delay at most 5. This is an explicit threshold model, not modularity optimization.
- Influence paths: minimum total delay from the first user, using positive-capacity directed relationships. Dijkstra and Bellman-Ford are checked on identical inputs.
- Campaign: maximum additive score under an integer budget, exact over the top **20 candidates**. DP and all three branch-and-bound policies agree on the same candidate set. Overlapping audience reach is not modeled.
- Capacity: integer maximum flow from first to last user; the shock divides every capacity by four, rounding down. Flow is skipped and exported as `null` above 2,000 users.
- Creator/topic assignment: maximum matching using topics each creator has actually posted about. One creator per topic and one topic per creator. Skipped above 10,000 users.
- Infrastructure: minimum spanning forest of the active graph's undirected projection.

All core algorithms are implemented locally. Standard sorting and search algorithms appear only in tests as independent reference implementations. Standard containers, heaps, maps, and utility operations support the implementations.

## Project contents

| Location | Contents |
|---|---|
| `include/social/` | Reusable algorithm and domain interfaces |
| `src/foundations.cpp` | Sorting/search interfaces, divide and conquer, scheduling, Huffman |
| `src/graph.cpp` | Shortest paths, spanning forests, TSP, flow, matching |
| `src/optimization.cpp` | DP, greedy allocation, backtracking, branch and bound |
| `src/text.cpp` | CRT and four string matchers |
| `src/network.cpp` | CSV persistence, data model, analytics, JSON/CSV exports |
| `src/main.cpp`, `src/workbench.cpp` | CLI and complete syllabus demonstration |
| `src/benchmark.cpp` | Repeated, verified benchmark experiments |
| `build/libsocial.a` | Reusable static library built by Make |
| `tests/` | Oracle tests, regressions, invalid-input and CLI integration tests |
| `data/sample/` | Synthetic users, relationships, posts, topics, interaction stream |
| `results/` | Recorded benchmark tables and verification evidence |
| `docs/` | User/design guides, algorithm mapping, analysis, demo and ownership plan |

See [the user guide](docs/USER_GUIDE.md), [design and API guide](docs/DESIGN.md), [algorithm coverage](docs/ALGORITHMS.md), [complexity and correctness report](docs/COMPLEXITY.md), and [eight-week ownership plan](docs/OWNERSHIP.md).

## Scope and limitations

This is a deterministic educational analytics product. All bundled data is synthetic; no social platform credentials or external API is required. Exact algorithms have explicit instance/table limits and reject oversized inputs. The handbook's eight-week milestones and student ownership/review requirements are recorded as a handoff plan; generated code does not establish individual student authorship or human review. Benchmark working-set estimates and cumulative process RSS are separately labeled; neither is a per-call allocation profiler.
