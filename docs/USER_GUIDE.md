# User guide

The recommended graphical entry point is Influence Studio:

```sh
make gui
```

It opens `http://127.0.0.1:8765`, automatically analyzes the sample network, and provides Overview, Communities, Campaign planner and Algorithm lab views. Keep the terminal open; Ctrl+C stops the server. No extra Python/npm packages are required. On macOS, `run_gui.command` is a double-click launcher. See [the GUI guide](GUI.md) for the complete workflow and Desktop-folder permissions.

In the GUI, choose **Change dataset** to use sample data, generate up to 10,000 users, load a directory, or import the five CSV files. Select a graph node to inspect score factors and its source path. Change budget and click **Run analysis**; the scenario buttons also apply normal/shock budget presets (35/10). **Export report** downloads the latest successful C++ CSV or JSON. A failed request leaves the last successful result visible with an error message.

The browser display samples large graphs, but calculations and searchable rankings use the full dataset. GUI imports are limited to 12 MB and 10,000 users; those limits are separate from the larger CLI limits described below. Dataset snapshots and generated files are temporary; downloaded reports persist in your chosen download location.

The following commands describe the alternative CLI workflow. Run commands from the repository root. `make` produces the application, test runner, static library and benchmark runner. Start `./build/social_cli` for the menu or use `--help` for syntax. Paths with spaces should be quoted in the shell.

## Loading a social network

Pass a directory containing these five files, each with the exact header shown:

| File | Header |
|---|---|
| users.csv | `id,name,campaign_cost` |
| topics.csv | `id,name` |
| posts.csv | `id,user_id,topic_id,text` |
| relationships.csv | `from,to,delay,capacity` |
| interactions.csv | `from,to,post_id,timestamp,strength` |

The sample has 12 users. IDs are unique within each entity type and range from 0 to 1,000,000,000. User IDs need not be contiguous. References must exist. Every user has positive campaign cost (at most 1,000,000); topics and users need nonempty names. Post text may be empty. Relationship delay is a positive integer at most 1,000,000,000; capacity is a nonnegative integer with the same upper bound. Parallel relationships and self-links are allowed and counted as separate records. Interaction strength is 1..1,000,000, timestamps are nonnegative integers at most 10^12 in nondecreasing order. An interaction's post must exist; it need not be authored by either interaction endpoint (a repost can discuss someone else's post).

CSV supports commas and escaped double quotes in quoted fields. CRLF and UTF-8 text are supported. Quoted multiline fields are deliberately rejected; files must have one record per line. File limits are one million rows and one million bytes per line; network vertex limit is 100,000. IDs and input headers are validated before analysis. Names/text are opaque UTF-8 byte sequences; Unicode normalization is not applied. String algorithms operate on bytes.

```sh
./build/social_cli generate 1000 data/generated
./build/social_cli analyze data/generated 50 results/generated
```

A prefix such as `results/generated` creates `generated.csv` and `generated.json`, including missing parent directories. Existing output files are overwritten. JSON exports paths, rankings, community labels, selected campaign users, matching pairs, flow and spanning-forest weight. CSV exports one row per user with score factors, community and delay. `null` means an unreachable distance or skipped flow; an unreachable CSV delay is blank. Creator matching is an empty list when skipped above 10,000 users.

The first user in `users.csv` is the dissemination/flow source; the last user is the flow sink. Use `route` to choose other endpoints. Floyd's route option outputs the distance; Dijkstra/Bellman also output an example optimal path. Equal-cost optimal routes need not be unique.

## Running comparisons

```sh
./build/social_cli route data/sample 0 11 dijkstra
./build/social_cli route data/sample 0 11 bellman
./build/social_cli route data/sample 0 11 floyd
./build/social_cli match naive 'aaaaa' aaa
./build/social_cli match rk 'aaaaa' aaa
./build/social_cli match fa 'aaaaa' aaa
./build/social_cli match kmp 'aaaaa' aaa
```

All four matching commands return `0`, `1`, `2`. An empty pattern matches every boundary including the end. A pattern longer than the text has no matches. The CLI exits 0 on success, 1 on a runtime/validation error, and 2 on invalid command shape; the interactive menu displays errors and continues.

## Campaign and shock interpretation

For `analyze ... 35 ...`, budget is 35 cost units. Candidates are the top 20 ranked users (all users in the sample), with ties broken by ascending ID. The DP result maximizes the sum of the candidates' scores without exceeding budget. It is exact for this candidate pool, and does not claim global optimality over excluded users or deduplicated audience coverage. The fractional result is an upper bound; partial creator purchases are not executable campaigns.

`shock` divides each relationship's capacity by four. Links reduced to zero stop contributing to routing, communities and infrastructure; historical rankings still use the original relationship/interaction records. Budgets are supplied independently: the demo reduces 35 to 10 as an additional constraint shock.

## Limits and troubleshooting

Graph limits and algorithm-specific caps are documented in the API and complexity report. A budget can be at most 1,000,000 but the DP table also must fit ten million cells, so large budgets with many candidates are rejected. If a file cannot be opened, check the directory and all five required filenames. Build with C++17 or later. To change build flags, run `make clean` first because Make does not track flag changes. `make sanitize` uses compiler sanitizers available on Clang/GCC; it is a development command, not required to run the application.
