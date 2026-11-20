#include "social/network.hpp"
#include <ostream>
namespace social {
void run_workbench(std::ostream &o) {
    std::vector<i64> activity = {4, -2, 7, -9, 5};
    auto sorted = activity;
    merge_sort(sorted);
    auto quick = activity;
    quick_sort(quick);
    o << "Sorting: merge/quick agree=" << (sorted == quick)
      << "; binary searches=" << binary_search_iterative(sorted, 5) << ','
      << binary_search_recursive(sorted, 5) << '\n';
    auto peak = maximum_subarray(activity);
    o << "Peak engagement window: " << peak.sum << " [" << peak.begin << ',' << peak.end
      << "); power 2^10=" << fast_power(2, 10)
      << "; celebrity=" << celebrity({{0, 1, 1}, {0, 0, 0}, {0, 1, 0}}) << '\n';
    Matrix matrix = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    o << "Classical/Strassen interaction projection agree="
      << (classical_multiply(matrix, matrix) == strassen_multiply(matrix, matrix)) << '\n';
    std::vector<Interval> slots = {{0, 0, 3}, {1, 1, 4}, {2, 3, 5}};
    auto rooms = interval_partition(slots);
    o << "Nonoverlapping posts=" << activity_selection(slots).size()
      << "; parallel channels=" << 1 + *std::max_element(rooms.begin(), rooms.end()) << '\n';
    auto encoded = huffman_encode("community community influence");
    o << "Huffman round trip=" << (huffman_decode(encoded) == "community community influence")
      << "; encoded bits=" << encoded.bits.size() << " (codebook excluded)\n";
    std::vector<Item> items = {{10, 60}, {20, 100}, {30, 120}};
    o << "Campaign: fractional upper bound=" << fractional_knapsack(items, 50)
      << "; exact DP=" << knapsack_dp(items, 50).value
      << "; backtracking=" << knapsack_backtracking(items, 50).value << '\n';
    for (auto mode : {Frontier::FIFO, Frontier::LIFO, Frontier::LeastCost}) {
        auto s = knapsack_branch_bound(items, 50, mode);
        o << "Branch-bound policy " << static_cast<int>(mode) << ": value=" << s.value
          << ", nodes=" << s.explored << '\n';
    }
    o << "Matrix-chain projection cost=" << matrix_chain({10, 20, 30, 10})
      << "; common topic sequence=" << lcs("ABCDGH", "AEDFHR")
      << "; optimal lookup BST cost=" << optimal_bst({0.3, 0.2}, {0.1, 0.1, 0.3}) << '\n';
    o << "4-Queens validation solutions=" << n_queens(4).size()
      << "; subset budget combinations=" << subset_sum({2, 3, 5}, 5).size() << '\n';
    Graph g(4);
    g.add(0, 1, 2, 3);
    g.add(0, 2, 5, 2);
    g.add(1, 2, 1, 1);
    g.add(1, 3, 4, 2);
    g.add(2, 3, 1, 3);
    o << "Dissemination delay Dijkstra/Bellman/Floyd=" << dijkstra(g, 0).distance[3] << '/'
      << bellman_ford(g, 0).distance[3] << '/' << floyd_warshall(g)[0][3]
      << "; Prim/Kruskal infrastructure=" << prim(g).weight << '/' << kruskal(g).weight
      << "; max flow=" << ford_fulkerson(g, 0, 3).value << '\n';
    o << "TSP ambassador tour="
      << tsp({{0, 10, 15, 20}, {10, 0, 35, 25}, {15, 35, 0, 30}, {20, 25, 30, 0}})
      << "; creator-topic matching=";
    for (auto v : bipartite_matching({{0, 1}, {0}, {1, 2}}, 3))
        o << v << ' ';
    o << "\nCRT counter reconstruction=" << chinese_remainder({2, 3, 2}, {3, 5, 7}) << '\n';
    for (auto mode : {Matcher::Naive, Matcher::RabinKarp, Matcher::Automaton, Matcher::KMP})
        o << "Matcher " << static_cast<int>(mode)
          << " occurrences=" << match("community community influence", "community", mode).size()
          << '\n';
}
} // namespace social
