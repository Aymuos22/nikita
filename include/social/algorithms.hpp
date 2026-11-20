#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace social {
using i64 = std::int64_t;
constexpr i64 INF = std::numeric_limits<i64>::max() / 4;
struct Metrics {
    std::uint64_t operations = 0;
    std::size_t workspace_bytes = 0;
};
// Stable merge sort and stack-bounded, three-way quicksort. STL sort is not used.
template <class T, class Less = std::less<T>>
void merge_sort(std::vector<T> &a, Less less = {}, Metrics *m = nullptr) {
    std::vector<T> tmp(a.size());
    if (m)
        m->workspace_bytes = tmp.size() * sizeof(T);
    std::function<void(std::size_t, std::size_t)> rec = [&](auto l, auto r) {
        if (r - l < 2)
            return;
        auto mid = l + (r - l) / 2;
        rec(l, mid);
        rec(mid, r);
        auto i = l, j = mid, k = l;
        while (i < mid && j < r) {
            if (m)
                ++m->operations;
            tmp[k++] = less(a[j], a[i]) ? a[j++] : a[i++];
        }
        while (i < mid)
            tmp[k++] = a[i++];
        while (j < r)
            tmp[k++] = a[j++];
        for (k = l; k < r; ++k)
            a[k] = tmp[k];
    };
    rec(0, a.size());
}
template <class T, class Less = std::less<T>>
void quick_sort(std::vector<T> &a, Less less = {}, Metrics *m = nullptr) {
    std::size_t depth = 0;
    auto cmp = [&](const T &x, const T &y) {
        if (m)
            ++m->operations;
        return less(x, y);
    };
    std::function<void(std::size_t, std::size_t, std::size_t)> rec = [&](auto l, auto r, auto d) {
        depth = std::max(depth, d);
        while (r - l > 1) {
            T p = a[l + (r - l) / 2];
            auto i = l, lt = l, gt = r;
            while (i < gt) {
                if (cmp(a[i], p))
                    std::swap(a[lt++], a[i++]);
                else if (cmp(p, a[i]))
                    std::swap(a[i], a[--gt]);
                else
                    ++i;
            }
            if (lt - l < r - gt) {
                rec(l, lt, d + 1);
                l = gt;
            } else {
                rec(gt, r, d + 1);
                r = lt;
            }
        }
    };
    rec(0, a.size(), 1);
    if (m)
        m->workspace_bytes = depth * 4 * sizeof(std::size_t); // stack payload estimate
}
int binary_search_iterative(const std::vector<i64> &a, i64 value);
int binary_search_recursive(const std::vector<i64> &a, i64 value);
struct Subarray {
    i64 sum;
    std::size_t begin, end;
}; // half-open, nonempty
Subarray maximum_subarray(const std::vector<i64> &a);
i64 fast_power(i64 base, unsigned exponent); // checked signed arithmetic
int celebrity(const std::vector<std::vector<int>> &knows);
using Matrix = std::vector<std::vector<i64>>;
Matrix classical_multiply(const Matrix &a, const Matrix &b);
Matrix strassen_multiply(const Matrix &a, const Matrix &b);
struct Interval {
    int id, start, finish;
};
std::vector<int> activity_selection(std::vector<Interval> intervals);
std::vector<int> interval_partition(const std::vector<Interval> &intervals);
struct HuffmanResult {
    std::array<std::string, 256> codes{};
    std::string bits;
};
HuffmanResult huffman_encode(const std::string &text);
std::string huffman_decode(const HuffmanResult &encoded);
struct Item {
    int cost, value;
};
struct Selection {
    i64 value = 0;
    std::vector<int> items;
    std::uint64_t explored = 0;
};
double fractional_knapsack(std::vector<Item> items, int capacity);
Selection knapsack_dp(const std::vector<Item> &items, int capacity);
Selection knapsack_backtracking(const std::vector<Item> &items, int capacity);
enum class Frontier { FIFO, LIFO, LeastCost };
Selection knapsack_branch_bound(const std::vector<Item> &items, int capacity, Frontier policy);
i64 matrix_chain(const std::vector<int> &dimensions);
std::string lcs(const std::string &a, const std::string &b);
double optimal_bst(const std::vector<double> &successful, const std::vector<double> &unsuccessful);
std::vector<std::vector<int>> n_queens(int n);
std::vector<std::vector<int>> subset_sum(const std::vector<int> &values, i64 target);
struct Edge {
    int from, to;
    i64 weight, capacity;
};
struct Graph {
    int n;
    std::vector<Edge> edges;
    explicit Graph(int vertices = 0);
    void add(int from, int to, i64 weight, i64 capacity = 0);
};
struct Paths {
    std::vector<i64> distance;
    std::vector<int> parent;
    bool negative_cycle = false;
};
Paths dijkstra(const Graph &graph, int source);
Paths bellman_ford(const Graph &graph, int source);
std::vector<std::vector<i64>> floyd_warshall(const Graph &graph);
std::vector<int> path_to(const Paths &paths, int target);
struct Forest {
    i64 weight = 0;
    int components = 0;
    std::vector<Edge> edges;
};
Forest prim(const Graph &graph); // each directed edge becomes an undirected candidate
Forest kruskal(const Graph &graph);
i64 tsp(const Matrix &costs); // Held-Karp, <=18 vertices; INF means absent
struct Flow {
    i64 value = 0;
    Matrix residual;
    std::vector<int> source_side;
};
Flow ford_fulkerson(const Graph &graph, int source, int sink);
std::vector<int> bipartite_matching(const std::vector<std::vector<int>> &adjacency,
                                    int right_count);
i64 chinese_remainder(const std::vector<i64> &residues, const std::vector<i64> &moduli);
enum class Matcher { Naive, RabinKarp, Automaton, KMP };
std::vector<std::size_t> match(const std::string &text, const std::string &pattern, Matcher method);
} // namespace social
