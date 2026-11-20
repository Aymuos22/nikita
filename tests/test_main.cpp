#include "social/network.hpp"
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
using namespace social;
static int checks = 0;
#define CHECK(...)                                                                                 \
    do {                                                                                           \
        ++checks;                                                                                  \
        if (!(__VA_ARGS__))                                                                        \
            throw std::runtime_error(std::string("line ") + std::to_string(__LINE__) +             \
                                     ": " #__VA_ARGS__);                                           \
    } while (false)
template <class F> void rejects(F f) {
    bool threw = false;
    try {
        f();
    } catch (const std::exception &) {
        threw = true;
    }
    CHECK(threw);
}
static i64 brute_knapsack(const std::vector<Item> &a, int cap) {
    i64 best = 0;
    for (std::size_t mask = 0; mask < (std::size_t{1} << a.size()); ++mask) {
        int cost = 0;
        i64 value = 0;
        for (std::size_t j = 0; j < a.size(); ++j)
            if (mask & (std::size_t{1} << j)) {
                cost += a[j].cost;
                value += a[j].value;
            }
        if (cost <= cap)
            best = std::max(best, value);
    }
    return best;
}
int main() {
    try {
        std::mt19937 rng(2026);
        for (int trial = 0; trial < 150; ++trial) {
            int n = static_cast<int>(rng() % 70);
            std::vector<i64> a(n);
            for (auto &x : a)
                x = static_cast<int>(rng() % 21) - 10;
            auto expected = a, b = a, original = a;
            std::sort(expected.begin(), expected.end());
            merge_sort(a);
            quick_sort(b);
            CHECK(a == expected);
            CHECK(b == expected);
            for (int target = -12; target <= 12; ++target) {
                auto it = std::lower_bound(a.begin(), a.end(), target);
                int pos = it == a.end() || *it != target ? -1 : static_cast<int>(it - a.begin());
                CHECK(binary_search_iterative(a, target) == pos);
                CHECK(binary_search_recursive(a, target) == pos);
            }
            if (n) {
                i64 best = -INF;
                for (int i = 0; i < n; ++i) {
                    i64 sum = 0;
                    for (int j = i; j < n; ++j) {
                        sum += original[j];
                        best = std::max(best, sum);
                    }
                }
                auto peak = maximum_subarray(original);
                CHECK(peak.sum == best);
                CHECK(peak.begin < peak.end && peak.end <= b.size());
                CHECK(std::accumulate(original.begin() + peak.begin, original.begin() + peak.end,
                                      i64{0}) == peak.sum);
            }
        }
        std::vector<std::pair<int, int>> stable = {{2, 0}, {1, 1}, {2, 2}, {1, 3}};
        merge_sort(stable, [](auto a, auto b) { return a.first < b.first; });
        CHECK(stable == std::vector<std::pair<int, int>>({{1, 1}, {1, 3}, {2, 0}, {2, 2}}));
        CHECK(fast_power(-2, 3) == -8);
        CHECK(fast_power(0, 0) == 1);
        CHECK(fast_power(std::numeric_limits<i64>::min(), 1) == std::numeric_limits<i64>::min());
        rejects([] { fast_power(2, 63); });
        rejects([] { fast_power(std::numeric_limits<i64>::min(), 2); });
        rejects([] { maximum_subarray({}); });
        rejects([] { maximum_subarray({1000000001LL}); });
        CHECK(celebrity({}) == -1);
        CHECK(celebrity({{0, 1, 1}, {0, 0, 0}, {0, 1, 0}}) == 1);
        CHECK(celebrity({{0, 1}, {1, 0}}) == -1);
        rejects([] { celebrity({{0, 1}}); });
        for (int n = 0; n <= 9; ++n) {
            Matrix a(n, std::vector<i64>(n)), b = a;
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j) {
                    a[i][j] = static_cast<int>(rng() % 11) - 5;
                    b[i][j] = static_cast<int>(rng() % 11) - 5;
                }
            CHECK(strassen_multiply(a, b) == classical_multiply(a, b));
        }
        rejects([] { strassen_multiply({{1, 2}}, {{1}}); });
        rejects([] { classical_multiply({{1000001}}, {{1}}); });
        for (int trial = 0; trial < 40; ++trial) {
            std::vector<Interval> slots;
            for (int i = 0; i < 8; ++i) {
                int start = static_cast<int>(rng() % 10);
                slots.push_back({i, start, start + 1 + static_cast<int>(rng() % 5)});
            }
            int best = 0;
            for (unsigned mask = 0; mask < 256; ++mask) {
                int count = 0;
                bool feasible = true;
                for (int i = 0; i < 8; ++i)
                    if (mask & (1u << i)) {
                        ++count;
                        for (int j = 0; j < i; ++j)
                            if ((mask & (1u << j)) && slots[i].start < slots[j].finish &&
                                slots[j].start < slots[i].finish)
                                feasible = false;
                    }
                if (feasible)
                    best = std::max(best, count);
            }
            CHECK(static_cast<int>(activity_selection(slots).size()) == best);
            auto assigned = interval_partition(slots);
            int peak = 0;
            for (int t = 0; t < 15; ++t) {
                int overlapping = 0;
                for (auto slot : slots)
                    if (slot.start <= t && t < slot.finish)
                        ++overlapping;
                peak = std::max(peak, overlapping);
            }
            CHECK(*std::max_element(assigned.begin(), assigned.end()) + 1 == peak);
            for (int i = 0; i < 8; ++i)
                for (int j = 0; j < i; ++j)
                    if (assigned[i] == assigned[j])
                        CHECK(slots[i].finish <= slots[j].start ||
                              slots[j].finish <= slots[i].start);
        }
        CHECK(activity_selection({{0, 0, 3}, {1, 1, 2}, {2, 2, 4}}) == std::vector<int>({1, 2}));
        auto rooms = interval_partition({{0, 0, 3}, {1, 1, 2}, {2, 2, 4}});
        CHECK(rooms[0] != rooms[1]);
        CHECK(rooms[1] == rooms[2]);
        rejects([] { activity_selection({{0, 1, 1}}); });
        for (auto text : std::vector<std::string>{"", "aaaaa", "community influence",
                                                  std::string("\0\xff\0", 3)}) {
            auto h = huffman_encode(text);
            CHECK(huffman_decode(h) == text);
        }
        HuffmanResult bad;
        bad.codes[0] = "0";
        bad.codes[1] = "01";
        rejects([&] { huffman_decode(bad); });
        for (int trial = 0; trial < 80; ++trial) {
            int n = rng() % 11, cap = rng() % 35;
            std::vector<Item> items;
            for (int i = 0; i < n; ++i)
                items.push_back({1 + static_cast<int>(rng() % 15), static_cast<int>(rng() % 100)});
            i64 exact = brute_knapsack(items, cap);
            auto validate_selection = [&](const Selection &s) {
                CHECK(s.value == exact);
                int cost = 0;
                i64 value = 0;
                std::vector<bool> seen(items.size());
                for (int i : s.items) {
                    CHECK(i >= 0 && static_cast<std::size_t>(i) < items.size());
                    CHECK(!seen[i]);
                    seen[i] = true;
                    cost += items[i].cost;
                    value += items[i].value;
                }
                CHECK(cost <= cap);
                CHECK(value == exact);
            };
            validate_selection(knapsack_dp(items, cap));
            validate_selection(knapsack_backtracking(items, cap));
            for (auto mode : {Frontier::FIFO, Frontier::LIFO, Frontier::LeastCost})
                validate_selection(knapsack_branch_bound(items, cap, mode));
            CHECK(fractional_knapsack(items, cap) + 1e-8 >= exact);
        }
        rejects([] { knapsack_dp({{0, 3}}, 5); });
        CHECK(matrix_chain({30, 35, 15, 5, 10, 20, 25}) == 15125);
        CHECK(lcs("AGGTAB", "GXTXAYB") == "GTAB");
        CHECK(lcs("", "a").empty());
        CHECK(std::abs(optimal_bst({.15, .1, .05, .1, .2}, {.05, .1, .05, .05, .05, .1}) - 2.75) <
              1e-10);
        CHECK(n_queens(4).size() == 2);
        CHECK(n_queens(8).size() == 92);
        CHECK(subset_sum({-1, 1, 2}, 1).size() == 2);
        CHECK(subset_sum({}, 0).size() == 1);
        for (int trial = 0; trial < 60; ++trial) {
            int n = 2 + rng() % 6;
            Graph g(n);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    if (i != j && rng() % 3 == 0)
                        g.add(i, j, rng() % 12, rng() % 8);
            auto all = floyd_warshall(g);
            for (int s = 0; s < n; ++s) {
                auto d = dijkstra(g, s), b = bellman_ford(g, s);
                CHECK(!b.negative_cycle);
                CHECK(d.distance == all[s]);
                CHECK(b.distance == all[s]);
                for (int t = 0; t < n; ++t) {
                    auto p = path_to(d, t);
                    if (d.distance[t] == INF)
                        CHECK(p.empty());
                    else
                        CHECK(!p.empty() && p.front() == s && p.back() == t);
                }
            }
            auto a = prim(g), b = kruskal(g);
            CHECK(a.weight == b.weight);
            CHECK(a.components == b.components);
            auto f = ford_fulkerson(g, 0, n - 1);
            i64 mincut = INF;
            for (unsigned mask = 1; mask < (1u << n); mask += 2)
                if (!(mask & (1u << (n - 1)))) {
                    i64 cap = 0;
                    for (auto e : g.edges)
                        if ((mask & (1u << e.from)) && !(mask & (1u << e.to)))
                            cap += e.capacity;
                    mincut = std::min(mincut, cap);
                }
            CHECK(f.value == mincut);
            i64 cut = 0;
            std::vector<bool> side(n);
            for (int u : f.source_side)
                side[u] = true;
            for (auto e : g.edges)
                if (side[e.from] && !side[e.to])
                    cut += e.capacity;
            CHECK(cut == f.value);
        }
        Graph neg(3);
        neg.add(0, 1, -1);
        neg.add(1, 2, 2);
        CHECK(bellman_ford(neg, 0).distance[2] == 1);
        rejects([&] { dijkstra(neg, 0); });
        neg.add(2, 1, -3);
        CHECK(bellman_ford(neg, 0).negative_cycle);
        rejects([&] { floyd_warshall(neg); });
        rejects([] { Graph(-1); });
        rejects([] { Graph(1).add(0, 1, 2); });
        rejects([] { ford_fulkerson(Graph(1), 0, 0); });
        for (int trial = 0; trial < 25; ++trial) {
            int n = 2 + rng() % 6;
            Matrix costs(n, std::vector<i64>(n));
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    costs[i][j] = i == j ? 0 : 1 + rng() % 20;
            std::vector<int> order(n - 1);
            std::iota(order.begin(), order.end(), 1);
            i64 best = INF;
            do {
                i64 cost = 0;
                int last = 0;
                for (int v : order) {
                    cost += costs[last][v];
                    last = v;
                }
                cost += costs[last][0];
                best = std::min(best, cost);
            } while (std::next_permutation(order.begin(), order.end()));
            CHECK(tsp(costs) == best);
        }
        CHECK(tsp({{0, INF}, {INF, 0}}) == INF);
        for (int trial = 0; trial < 40; ++trial) {
            int n = 1 + rng() % 5;
            std::vector<std::vector<int>> adj(n);
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    if (rng() % 2)
                        adj[i].push_back(j);
            auto result = bipartite_matching(adj, n);
            int found = 0;
            unsigned used = 0;
            for (int i = 0; i < n; ++i)
                if (result[i] >= 0) {
                    CHECK(std::find(adj[i].begin(), adj[i].end(), result[i]) != adj[i].end());
                    CHECK(!(used & (1u << result[i])));
                    used |= 1u << result[i];
                    ++found;
                }
            std::function<int(int, unsigned)> brute = [&](int u, unsigned mask) {
                if (u == n)
                    return 0;
                int best = brute(u + 1, mask);
                for (int v : adj[u])
                    if (!(mask & (1u << v)))
                        best = std::max(best, 1 + brute(u + 1, mask | (1u << v)));
                return best;
            };
            CHECK(found == brute(0, 0));
        }
        CHECK(chinese_remainder({2, 3, 2}, {3, 5, 7}) == 23);
        CHECK(chinese_remainder({-1, 2}, {3, 5}) == 2);
        CHECK(chinese_remainder({}, {}) == 0);
        rejects([] { chinese_remainder({1, 2}, {4, 6}); });
        rejects([] { chinese_remainder({1}, {1}); });
        for (int x = 0; x < 105; ++x)
            CHECK(chinese_remainder({x % 3, x % 5, x % 7}, {3, 5, 7}) == x);
        for (int trial = 0; trial < 200; ++trial) {
            std::string text, pattern;
            for (unsigned i = 0, n = rng() % 80; i < n; ++i)
                text += static_cast<char>(rng() % 5);
            for (unsigned i = 0, n = rng() % 15; i < n; ++i)
                pattern += static_cast<char>(rng() % 5);
            std::vector<std::size_t> oracle;
            for (std::size_t i = 0; i <= text.size(); ++i)
                if (text.compare(i, pattern.size(), pattern) == 0)
                    oracle.push_back(i);
            for (auto mode : {Matcher::Naive, Matcher::RabinKarp, Matcher::Automaton, Matcher::KMP})
                CHECK(match(text, pattern, mode) == oracle);
        }
        for (auto mode : {Matcher::Naive, Matcher::RabinKarp, Matcher::Automaton, Matcher::KMP})
            CHECK(match("aaaaa", "aaa", mode) == std::vector<std::size_t>({0, 1, 2}));
        auto net = generate_network(12);
        auto normal = analyze(net, 35), shock = analyze(net, 10, true);
        CHECK(normal.flow >= shock.flow);
        CHECK(normal.campaign.value >= shock.campaign.value);
        CHECK(normal.rankings.size() == 12);
        CHECK(json_escape("\"\n\t") == "\\\"\\n\\t");
        std::cout << "Passed " << checks << " checks (deterministic seed 2026)\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
