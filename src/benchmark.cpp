#include "social/benchmark_memory.hpp"
#include "social/network.hpp"
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#endif
using namespace social;
namespace {
volatile std::uint64_t sink = 0;
std::size_t peak_rss() {
#if defined(__unix__) || defined(__APPLE__)
    struct rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) != 0)
        return 0;
#if defined(__APPLE__)
    return static_cast<std::size_t>(usage.ru_maxrss);
#else
    return static_cast<std::size_t>(usage.ru_maxrss) * 1024;
#endif
#else
    return 0;
#endif
}
template <class F>
void bench(std::ostream &out, const std::string &data, const std::string &algorithm, std::size_t n,
           std::size_t bytes, F f, std::uint64_t operations = 0, double reference = -1,
           const char *quality = "not_applicable") {
    double best = 1e100, total = 0;
    std::uint64_t result = 0;
    double objective = 0;
    std::size_t peak_heap = 0;
    for (int r = 0; r < 3; ++r) {
        auto start = std::chrono::steady_clock::now();
        benchmark_memory::begin();
        try {
            objective = static_cast<double>(f());
        } catch (...) {
            benchmark_memory::finish();
            throw;
        }
        peak_heap = std::max(peak_heap, benchmark_memory::finish());
        result = static_cast<std::uint64_t>(objective);
        auto end = std::chrono::steady_clock::now();
        double us = std::chrono::duration<double, std::micro>(end - start).count();
        best = std::min(best, us);
        total += us;
        sink = result;
    }
    out << data << ',' << algorithm << ',' << n << ',' << best << ',' << total / 3 << ',';
    if (bytes)
        out << bytes;
    out << ',' << peak_rss() << ',' << operations << ',' << result << ',' << peak_heap << ','
        << std::setprecision(12);
    if (std::string(quality) != "unreachable")
        out << objective;
    out << ',';
    if (reference >= 0)
        out << reference;
    out << ',';
    if (reference >= 0)
        out << std::abs(objective - reference) / std::max(1.0, std::abs(reference));
    out << ',' << quality << '\n';
}
} // namespace
static void header(std::ostream &out) {
    out << "dataset,algorithm,n,best_us,mean_us,estimated_workspace_bytes,process_peak_rss_bytes,"
           "counted_sort_comparisons,result_checksum,measured_peak_heap_bytes,solution_value,"
           "reference_value,relative_gap,quality_kind\n";
}
static void compare_scenario(const std::string &directory, int budget, const std::string &output,
                             bool shock) {
    auto network = load_network(directory);
    auto analysis = analyze(network, budget, shock);
    auto parent = std::filesystem::path(output).parent_path();
    if (!parent.empty())
        std::filesystem::create_directories(parent);
    std::ofstream out(output);
    out.exceptions(std::ios::failbit | std::ios::badbit);
    header(out);
    const std::string name = shock ? "capacity_shock" : "normal";
    Graph active(network.graph.n);
    for (auto edge : network.graph.edges) {
        if (shock)
            edge.capacity /= 4;
        if (edge.capacity)
            active.add(edge.from, edge.to, edge.weight, edge.capacity);
    }
    auto oracle = bellman_ford(active, 0);
    if (dijkstra(active, 0).distance != oracle.distance)
        throw std::logic_error("scenario route mismatch");
    const auto target = network.users.size() - 1;
    const bool unreachable = oracle.distance[target] == INF;
    const double distance = unreachable ? -1 : static_cast<double>(oracle.distance[target]);
    const char *route_quality = unreachable ? "unreachable" : "exact_distance";
    bench(
        out, name, "dijkstra", network.users.size(), 0,
        [&] { return dijkstra(active, 0).distance[target]; }, 0, distance, route_quality);
    bench(
        out, name, "bellman_ford", network.users.size(), 0,
        [&] { return bellman_ford(active, 0).distance[target]; }, 0, distance, route_quality);
    const auto forest = kruskal(active);
    if (prim(active).weight != forest.weight)
        throw std::logic_error("scenario MST mismatch");
    bench(
        out, name, "prim", network.users.size(), 0, [&] { return prim(active).weight; }, 0,
        forest.weight, "exact_forest_cost");
    bench(
        out, name, "kruskal", network.users.size(), 0, [&] { return kruskal(active).weight; }, 0,
        forest.weight, "exact_forest_cost");
    std::vector<Item> candidates;
    for (std::size_t i = 0; i < analysis.rankings.size() && i < 20; ++i) {
        auto user = analysis.rankings[i];
        candidates.push_back({network.users[network.user_index.at(user.id)].campaign_cost,
                              static_cast<int>(user.score)});
    }
    const auto optimum = analysis.campaign.value;
    bench(
        out, name, "knapsack_dp", candidates.size(), 0,
        [&] { return knapsack_dp(candidates, budget).value; }, 0, optimum, "exact_feasible");
    bench(
        out, name, "knapsack_backtracking", candidates.size(), 0,
        [&] {
            auto result = knapsack_backtracking(candidates, budget);
            if (result.value != optimum)
                throw std::logic_error("scenario backtracking mismatch");
            return result.value;
        },
        0, optimum, "exact_feasible");
    for (auto policy : {Frontier::FIFO, Frontier::LIFO, Frontier::LeastCost})
        bench(
            out, name, "branch_bound_" + std::to_string(static_cast<int>(policy)),
            candidates.size(), 0,
            [&] {
                auto result = knapsack_branch_bound(candidates, budget, policy);
                if (result.value != optimum)
                    throw std::logic_error("scenario B&B mismatch");
                return result.value;
            },
            0, optimum, "exact_feasible");
    bench(
        out, name, "fractional_knapsack", candidates.size(), 0,
        [&] { return fractional_knapsack(candidates, budget); }, 0, optimum,
        "upper_bound_not_integer_feasible");
    bench(
        out, name, "density_greedy_integer", candidates.size(), 0,
        [&] {
            auto items = candidates;
            merge_sort(items, [](auto a, auto b) {
                return static_cast<i64>(a.value) * b.cost > static_cast<i64>(b.value) * a.cost;
            });
            int remaining = budget;
            i64 value = 0;
            for (auto item : items)
                if (item.cost <= remaining) {
                    remaining -= item.cost;
                    value += item.value;
                }
            return value;
        },
        0, optimum, "heuristic_feasible");
    if (network.users.size() <= 2000 && network.users.size() > 1)
        bench(
            out, name, "ford_fulkerson", network.users.size(), 0,
            [&] { return ford_fulkerson(active, 0, static_cast<int>(target)).value; }, 0,
            analysis.flow, "exact_flow");
    std::cout
        << "Scenario comparisons written to " << output
        << "; same input and candidate pool, 3 repetitions; measured per-call C++ heap payload.\n";
}
int main(int argc, char **argv) {
    try {
        if (argc >= 2 && std::string(argv[1]) == "--scenario") {
            if (argc != 5 && argc != 6)
                throw std::invalid_argument(
                    "usage: social_bench --scenario DIRECTORY BUDGET OUTPUT.csv [shock]");
            std::size_t used = 0;
            int budget = std::stoi(argv[3], &used);
            if (used != std::string(argv[3]).size())
                throw std::invalid_argument("invalid budget");
            if (argc == 6 && std::string(argv[5]) != "shock")
                throw std::invalid_argument("expected shock");
            compare_scenario(argv[2], budget, argv[4], argc == 6);
            return 0;
        }
        std::string path = argc > 1 ? argv[1] : "results/benchmarks.csv";
        auto parent = std::filesystem::path(path).parent_path();
        if (!parent.empty())
            std::filesystem::create_directories(parent);
        std::ofstream out(path);
        out.exceptions(std::ios::failbit | std::ios::badbit);
        header(out);
        std::mt19937 rng(15);
        for (auto spec : std::vector<std::pair<std::string, int>>{
                 {"small", 128}, {"medium", 2048}, {"large", 10000}, {"adversarial", 2048}}) {
            const std::string label = spec.first;
            const int n = spec.second;
            std::vector<i64> values(n);
            for (auto &x : values)
                x = label == "adversarial" ? 7 : static_cast<i64>(rng() % 100000);
            auto sorted = values;
            Metrics merge_metrics, quick_metrics;
            merge_sort(sorted, std::less<i64>{}, &merge_metrics);
            auto quick_copy = values;
            quick_sort(quick_copy, std::less<i64>{}, &quick_metrics);
            if (sorted != quick_copy)
                throw std::logic_error("sort mismatch");
            bench(
                out, label, "merge_sort", n, merge_metrics.workspace_bytes,
                [&] {
                    auto x = values;
                    merge_sort(x);
                    return x[x.size() / 2];
                },
                merge_metrics.operations);
            bench(
                out, label, "quick_sort", n, quick_metrics.workspace_bytes,
                [&] {
                    auto x = values;
                    quick_sort(x);
                    return x[x.size() / 2];
                },
                quick_metrics.operations);
            bench(out, label, "binary_iterative", n, 3 * sizeof(std::size_t),
                  [&] { return binary_search_iterative(sorted, sorted[n / 2]); });
            bench(out, label, "binary_recursive", n, 16 * 3 * sizeof(std::size_t),
                  [&] { return binary_search_recursive(sorted, sorted[n / 2]); });
            auto net = generate_network(n);
            if (label == "adversarial") {
                net.graph = Graph(n);
                for (int i = n - 2; i >= 0; --i)
                    net.graph.add(i, i + 1, 1, 1);
            }
            auto d = dijkstra(net.graph, 0), b = bellman_ford(net.graph, 0);
            if (d.distance != b.distance)
                throw std::logic_error("benchmark routing mismatch");
            bench(out, label, "dijkstra", n, (net.graph.edges.size() * 3 + n * 4) * sizeof(i64),
                  [&] { return dijkstra(net.graph, 0).distance.back(); });
            bench(out, label, "bellman_ford", n, n * (sizeof(i64) + sizeof(int)),
                  [&] { return bellman_ford(net.graph, 0).distance.back(); });
            auto k = kruskal(net.graph);
            if (k.weight != prim(net.graph).weight)
                throw std::logic_error("benchmark MST mismatch");
            bench(out, label, "prim", n, (net.graph.edges.size() * 4 + n) * sizeof(i64),
                  [&] { return prim(net.graph).weight; });
            bench(out, label, "kruskal", n,
                  net.graph.edges.size() * sizeof(Edge) + n * 2 * sizeof(int),
                  [&] { return kruskal(net.graph).weight; });
            std::string text;
            for (int i = 0; i < n; ++i)
                text += label == "adversarial" ? "aaaaaaaa" : "social community campaign ";
            std::string pattern = label == "adversarial" ? std::string(63, 'a') + "b" : "community";
            auto expected = match(text, pattern, Matcher::Naive);
            for (auto method :
                 {Matcher::Naive, Matcher::RabinKarp, Matcher::Automaton, Matcher::KMP}) {
                if (match(text, pattern, method) != expected)
                    throw std::logic_error("benchmark string mismatch");
                std::size_t bytes = method == Matcher::Automaton
                                        ? (pattern.size() + 1) * 256 * sizeof(std::size_t)
                                    : method == Matcher::KMP ? pattern.size() * sizeof(std::size_t)
                                                             : 32;
                bench(out, label, "matcher_" + std::to_string(static_cast<int>(method)),
                      text.size(), bytes, [&] { return match(text, pattern, method).size(); });
            }
            bench(out, label, "max_subarray", n, 16 * sizeof(Subarray),
                  [&] { return maximum_subarray(values).sum; });
            bench(out, label, "huffman", text.size(), 511 * 32 + text.size() * 8,
                  [&] { return huffman_encode(text).bits.size(); });
        }
        // Bounded families use problem-specific sizes; exponential solvers never receive stress
        // graph sizes.
        for (int tier = 0; tier < 4; ++tier) {
            std::string label = std::vector<std::string>{
                "bounded_small", "bounded_medium", "bounded_large", "bounded_adversarial"}[tier];
            int n = 4 + std::min(tier, 2) * 2;
            Matrix a(n, std::vector<i64>(n));
            for (auto &row : a)
                for (auto &x : row)
                    x = rng() % 5;
            if (classical_multiply(a, a) != strassen_multiply(a, a))
                throw std::logic_error("matrix mismatch");
            bench(out, label, "classical_matrix", n, n * n * 8,
                  [&] { return classical_multiply(a, a)[0][0]; });
            bench(out, label, "strassen_matrix", n, n * n * 8 * 32,
                  [&] { return strassen_multiply(a, a)[0][0]; });
            std::vector<Item> items;
            for (int i = 0; i < 10 + tier * 4; ++i)
                items.push_back(
                    {1 + static_cast<int>(rng() % 15), 1 + static_cast<int>(rng() % 100)});
            int cap = 30 + tier * 15;
            if (tier == 3) {
                items = {{10, 60}, {20, 100}, {30, 120}};
                cap = 50;
            }
            auto exact = knapsack_dp(items, cap).value;
            bench(
                out, label, "fractional_knapsack", items.size(), items.size() * sizeof(Item),
                [&] { return fractional_knapsack(items, cap); }, 0, exact,
                "upper_bound_not_integer_feasible");
            bench(
                out, label, "knapsack_dp", items.size(),
                (items.size() + 1) * (cap + 1) * sizeof(i64),
                [&] { return knapsack_dp(items, cap).value; }, 0, exact, "exact_feasible");
            bench(
                out, label, "knapsack_backtracking", items.size(), items.size() * 64,
                [&] {
                    auto s = knapsack_backtracking(items, cap);
                    if (s.value != exact)
                        throw std::logic_error("backtracking mismatch");
                    return s.value;
                },
                0, exact, "exact_feasible");
            for (auto policy : {Frontier::FIFO, Frontier::LIFO, Frontier::LeastCost})
                bench(
                    out, label, "branch_bound_" + std::to_string(static_cast<int>(policy)),
                    items.size(), (std::size_t{1} << items.size()) * 64,
                    [&] {
                        auto s = knapsack_branch_bound(items, cap, policy);
                        if (s.value != exact)
                            throw std::logic_error("B&B mismatch");
                        return s.value;
                    },
                    0, exact, "exact_feasible");
            std::vector<Interval> intervals;
            for (int i = 0; i < 20 * (tier + 1); ++i)
                intervals.push_back({i, i, i + 3});
            bench(out, label, "activity_selection", intervals.size(),
                  intervals.size() * sizeof(Interval) * 2,
                  [&] { return activity_selection(intervals).size(); });
            bench(out, label, "interval_partition", intervals.size(), intervals.size() * 32, [&] {
                auto r = interval_partition(intervals);
                return 1 + *std::max_element(r.begin(), r.end());
            });
            std::vector<int> dims(10 * (tier + 1), 5);
            bench(out, label, "matrix_chain", dims.size(), dims.size() * dims.size() * 8,
                  [&] { return matrix_chain(dims); });
            std::string x(50 * (tier + 1), 'a'), y = x;
            y[0] = 'b';
            bench(out, label, "lcs", x.size(), (x.size() + 1) * (y.size() + 1) * 4,
                  [&] { return lcs(x, y).size(); });
            std::vector<double> p(10 * (tier + 1), 1), q(p.size() + 1, 1);
            bench(out, label, "optimal_bst", p.size(), (p.size() + 1) * (p.size() + 1) * 16,
                  [&] { return optimal_bst(p, q); });
            bench(out, label, "n_queens", n, n * 64, [&] { return n_queens(n).size(); });
            std::vector<int> subsets(10 + tier * 2, 1);
            bench(out, label, "subset_sum", subsets.size(),
                  (std::size_t{1} << subsets.size()) * subsets.size() * 4,
                  [&] { return subset_sum(subsets, subsets.size() / 2).size(); });
            std::vector<std::vector<int>> relation(n, std::vector<int>(n));
            for (int i = 0; i < n - 1; ++i)
                relation[i][n - 1] = 1;
            bench(out, label, "celebrity", n, 8, [&] { return celebrity(relation); });
            bench(out, label, "fast_power", n, 16,
                  [&] { return fast_power(2, static_cast<unsigned>(n)); });
            bench(out, label, "CRT", 3, 32,
                  [&] { return chinese_remainder({2, 3, 2}, {3, 5, 7}); });
            auto net = generate_network(n);
            bench(out, label, "floyd_warshall", n, n * n * 8,
                  [&] { return floyd_warshall(net.graph)[0][n - 1]; });
            bench(out, label, "ford_fulkerson", n, n * n * 8 + n * 8,
                  [&] { return ford_fulkerson(net.graph, 0, n - 1).value; });
            std::vector<std::vector<int>> adjacency(n);
            for (int i = 0; i < n; ++i)
                adjacency[i] = {i, (i + 1) % n};
            bench(out, label, "bipartite_matching", n, n * 16, [&] {
                auto r = bipartite_matching(adjacency, n);
                return std::count_if(r.begin(), r.end(), [](int v) { return v >= 0; });
            });
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    a[i][j] = i == j ? 0 : 1 + rng() % 50;
            bench(out, label, "held_karp_tsp", n, (std::size_t{1} << n) * n * 8,
                  [&] { return tsp(a); });
        }
        std::cout << "Wrote " << path
                  << " (3 repetitions; per-call heap payload measured; working-set estimates are "
                     "approximate; RSS is cumulative "
                     "per process)\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
