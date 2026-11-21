#include "social/network.hpp"
#include <iomanip>
#include <ostream>
#include <set>

namespace social {
void run_workbench(std::ostream &out, const Network &network, int budget, bool shock) {
    const auto result = analyze(network, budget, shock);
    const auto count = network.users.size();
    out << "DATASET WORKBENCH: " << count << " users, " << network.posts.size() << " posts, "
        << network.interactions.size() << " interactions\n";
    out << "Scenario=" << (shock ? "capacity_shock" : "normal") << "; budget=" << budget
        << "; all mappings below use this dataset unless explicitly labeled validation.\n";
    std::vector<i64> scores;
    for (auto user : result.rankings)
        scores.push_back(user.score);
    auto quick = scores;
    merge_sort(scores);
    quick_sort(quick);
    if (scores != quick)
        throw std::logic_error("domain sort disagreement");
    auto key = scores.back();
    out << "Sorting: merge/quick agree=1; highest score=" << key
        << "; binary searches=" << binary_search_iterative(scores, key) << ','
        << binary_search_recursive(scores, key)
        << "; tree ID index first user=" << network.user_index.at(network.users.front().id) << '\n';

    const auto events = std::min<std::size_t>(256, network.interactions.size());
    i64 average = 0;
    for (std::size_t i = 0; i < events; ++i)
        average += network.interactions[i].strength;
    if (events)
        average /= static_cast<i64>(events);
    std::vector<i64> activity;
    for (std::size_t i = 0; i < events; ++i)
        activity.push_back(network.interactions[i].strength - average);
    if (activity.empty())
        activity.push_back(0);
    auto peak = maximum_subarray(activity);
    out << "Maximum-subarray engagement above integer mean=" << peak.sum << "; event range ["
        << peak.begin << ',' << peak.end << "); first " << events << " events.\n";
    out << "Powering growth scenario: 2^" << std::min<std::size_t>(10, events) << '='
        << fast_power(2, static_cast<unsigned>(std::min<std::size_t>(10, events)))
        << " (analytical factor, not predicted reach).\n";

    const auto bounded = std::min<std::size_t>(8, count);
    Graph active(static_cast<int>(count)), subgraph(static_cast<int>(bounded));
    Matrix matrix(bounded, std::vector<i64>(bounded)),
        costs(bounded, std::vector<i64>(bounded, INF));
    std::vector<std::vector<int>> knows(bounded, std::vector<int>(bounded));
    for (std::size_t i = 0; i < bounded; ++i)
        costs[i][i] = 0;
    for (auto edge : network.graph.edges) {
        if (shock)
            edge.capacity /= 4;
        if (!edge.capacity)
            continue;
        active.add(edge.from, edge.to, edge.weight, edge.capacity);
        if (static_cast<std::size_t>(edge.from) < bounded &&
            static_cast<std::size_t>(edge.to) < bounded) {
            knows[edge.from][edge.to] = 1;
            matrix[edge.from][edge.to] = 1;
            costs[edge.from][edge.to] = std::min(costs[edge.from][edge.to], edge.weight);
            subgraph.add(edge.from, edge.to, edge.weight, edge.capacity);
        }
    }
    int found = celebrity(knows);
    out << "Celebrity among first " << bounded << " users: ";
    if (found < 0)
        out << "none";
    else
        out << network.users[found].id;
    out << '\n';
    if (classical_multiply(matrix, matrix) != strassen_multiply(matrix, matrix))
        throw std::logic_error("domain matrix disagreement");
    out << "Classical/Strassen two-step active-link projection agree=1; first " << bounded
        << " users.\n";

    std::vector<Interval> slots;
    for (std::size_t i = 0; i < std::min<std::size_t>(32, network.posts.size()); ++i) {
        const auto &post = network.posts[i];
        i64 start = static_cast<i64>(i) * 2;
        for (const auto &event : network.interactions)
            if (event.post == post.id) {
                start = event.timestamp % 10000;
                break;
            }
        slots.push_back(
            {post.id, static_cast<int>(start), static_cast<int>(start + 1 + post.text.size() % 5)});
    }
    auto rooms = interval_partition(slots);
    const int channels = rooms.empty() ? 0 : 1 + *std::max_element(rooms.begin(), rooms.end());
    out << "Activity selection posting IDs: ";
    for (int id : activity_selection(slots))
        out << id << ' ';
    out << "; interval partition channels=" << channels
        << ". Durations=1+text bytes mod 5; starts=first interaction timestamp mod 10000, else "
           "2*record index.\n";
    std::string text;
    for (const auto &post : network.posts) {
        if (text.size() >= 32000)
            break;
        text += post.text.substr(0, 32000 - text.size());
        if (text.size() < 32000)
            text += '\n';
    }
    auto compressed = huffman_encode(text);
    if (huffman_decode(compressed) != text)
        throw std::logic_error("domain Huffman roundtrip failed");
    out << "Huffman round trip=1; input bytes=" << text.size()
        << "; encoded bits=" << compressed.bits.size()
        << " (ASCII bit representation; codebook excluded).\n";
    auto d = dijkstra(active, 0), b = bellman_ford(active, 0);
    if (d.distance != b.distance)
        throw std::logic_error("domain route disagreement");
    out << "Dijkstra/Bellman-Ford agree=1; full active network. Prim/Kruskal forest="
        << prim(active).weight << '/' << kruskal(active).weight << '\n';
    auto all = floyd_warshall(subgraph);
    if (all[0] != dijkstra(subgraph, 0).distance)
        throw std::logic_error("domain Floyd disagreement");
    out << "Floyd-Warshall verified on first " << bounded
        << " users' induced graph (paths may differ from full-network routes).\n";

    std::vector<Item> items;
    for (std::size_t i = 0; i < std::min<std::size_t>(20, count); ++i) {
        auto user = result.rankings[i];
        items.push_back({network.users[network.user_index.at(user.id)].campaign_cost,
                         static_cast<int>(user.score)});
    }
    const auto optimum = knapsack_dp(items, budget).value;
    if (knapsack_backtracking(items, budget).value != optimum)
        throw std::logic_error("domain campaign disagreement");
    out << "Loaded-data campaign: exact DP/backtracking=" << optimum
        << "; candidates=" << items.size()
        << "; fractional upper bound=" << fractional_knapsack(items, budget) << '\n';
    for (auto policy : {Frontier::FIFO, Frontier::LIFO, Frontier::LeastCost}) {
        auto selected = knapsack_branch_bound(items, budget, policy);
        if (selected.value != optimum)
            throw std::logic_error("domain B&B disagreement");
        out << "Branch-bound policy " << static_cast<int>(policy) << ": value=" << selected.value
            << ", nodes=" << selected.explored << '\n';
    }
    std::vector<int> dimensions;
    for (std::size_t i = 0; i < bounded; ++i)
        dimensions.push_back(1 + std::min(9999, result.rankings[i].incoming));
    dimensions.push_back(1 + static_cast<int>(bounded));
    out << "Matrix-chain analytics projection cost=" << matrix_chain(dimensions)
        << "; dimensions derived from bounded incoming counts.\n";
    std::string first, second;
    for (const auto &post : network.posts) {
        if (post.user == network.users[0].id && first.size() < 256)
            first += post.text.substr(0, 256 - first.size());
        if (count > 1 && post.user == network.users[1].id && second.size() < 256)
            second += post.text.substr(0, 256 - second.size());
    }
    out << "LCS first two users' post text byte length=" << lcs(first, second).size()
        << " (first 256 bytes per user).\n";
    std::vector<double> hit(bounded), miss(bounded + 1, 1.0 / static_cast<double>(bounded + 1));
    for (std::size_t i = 0; i < bounded; ++i)
        hit[i] = 1 + result.rankings[i].engagement;
    out << "Optimal BST weighted lookup cost=" << optimal_bst(hit, miss)
        << " (ranked engagement+1 hit weights; uniform analytical miss weights).\n";
    auto tour = tsp(costs);
    out << "TSP first " << bounded << " users' direct active links: ";
    if (tour == INF)
        out << "no feasible tour";
    else
        out << tour;
    out << '\n';
    out << "N-Queens validation n=" << bounded
        << ", solutions=" << n_queens(static_cast<int>(bounded)).size()
        << " (syllabus validation, independent of social feasibility).\n";
    std::vector<int> subset_costs;
    for (std::size_t i = 0; i < std::min<std::size_t>(16, items.size()); ++i)
        subset_costs.push_back(items[i].cost);
    out << "Subset exact-budget combinations=" << subset_sum(subset_costs, budget).size()
        << " among first " << subset_costs.size() << " candidate costs.\n";
    out << "Ford-Fulkerson=";
    if (result.flow < 0)
        out << "skipped (requires 2..2000 users)";
    else
        out << result.flow;
    out << "; eligible creator-topic maximum matching=" << result.creator_topics.size() << '\n';
    const auto counter = static_cast<i64>(network.interactions.size());
    auto reconstructed = chinese_remainder({counter % 3, counter % 5, counter % 7}, {3, 5, 7});
    if (reconstructed != counter % 105)
        throw std::logic_error("domain CRT disagreement");
    out << "CRT counter reconstruction=" << reconstructed
        << " (mod 105; actual interaction counter=" << counter << ").\n";
    std::string pattern =
        network.topics.empty() ? "community" : network.topics[0].name.substr(0, 128);
    auto matches = match(text, pattern, Matcher::Naive);
    for (auto method : {Matcher::Naive, Matcher::RabinKarp, Matcher::Automaton, Matcher::KMP}) {
        auto positions = match(text, pattern, method);
        if (positions != matches)
            throw std::logic_error("domain matcher disagreement");
        out << "Matcher " << static_cast<int>(method) << " occurrences=" << positions.size()
            << "; first topic keyword byte length=" << pattern.size() << '\n';
    }
    out << "Bounded projections above are explicit; the full-network ranking, routing and "
           "candidate selection remain unchanged.\n";
}
} // namespace social
