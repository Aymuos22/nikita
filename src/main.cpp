#include "social/network.hpp"
#include <chrono>
#include <iostream>
#include <optional>
#include <sstream>
using namespace social;
static int number(const std::string &s) {
    std::size_t used = 0;
    int n = std::stoi(s, &used);
    if (used != s.size())
        throw std::invalid_argument("invalid integer");
    return n;
}
static void report(const Network &net, int budget, bool shock, const std::string &prefix) {
    auto a = analyze(net, budget, shock);
    export_analysis(net, a, prefix);
    std::cout << (shock ? "Capacity shock" : "Normal operation") << ": users=" << net.users.size()
              << ", communities=" << a.community_count << ", campaign value=" << a.campaign.value
              << ", flow=";
    if (a.flow < 0)
        std::cout << "skipped (matrix flow limit: 2000)";
    else
        std::cout << a.flow;
    std::cout << "\nRank = 10 * incoming links + incoming interaction strength\n";
    for (std::size_t i = 0; i < std::min<std::size_t>(5, a.rankings.size()); ++i) {
        auto r = a.rankings[i];
        std::cout << r.name << ": " << r.score << " = 10 * " << r.incoming << " + " << r.engagement
                  << '\n';
    }
    Graph active(net.graph.n);
    for (auto e : net.graph.edges) {
        if (shock)
            e.capacity /= 4;
        if (e.capacity)
            active.add(e.from, e.to, e.weight, e.capacity);
    }
    auto start = std::chrono::steady_clock::now();
    auto d = dijkstra(active, 0);
    auto middle = std::chrono::steady_clock::now();
    auto b = bellman_ford(active, 0);
    auto end = std::chrono::steady_clock::now();
    if (d.distance != b.distance)
        throw std::logic_error("routing mismatch");
    std::cout << "Same-input shortest paths agree. Dijkstra="
              << std::chrono::duration<double, std::micro>(middle - start).count()
              << " us; Bellman-Ford="
              << std::chrono::duration<double, std::micro>(end - middle).count() << " us\n";
    std::vector<Item> candidates;
    for (std::size_t i = 0; i < std::min<std::size_t>(20, net.users.size()); ++i) {
        auto r = a.rankings[i];
        candidates.push_back(
            {net.users[net.user_index.at(r.id)].campaign_cost, static_cast<int>(r.score)});
    }
    for (auto policy : {Frontier::FIFO, Frontier::LIFO, Frontier::LeastCost}) {
        auto selected = knapsack_branch_bound(candidates, budget, policy);
        if (selected.value != a.campaign.value)
            throw std::logic_error("campaign mismatch");
        std::cout << "Exact campaign policy " << static_cast<int>(policy) << ": " << selected.value
                  << ", nodes=" << selected.explored << '\n';
    }
    std::cout << "Fractional relaxation upper bound=" << fractional_knapsack(candidates, budget)
              << "; exact candidates=" << candidates.size() << "; budget=" << budget
              << "\nReports: " << prefix << ".csv and .json\n";
}
static void usage() {
    std::cout << "Social Network Influence and Community Analysis\n  social_cli demo "
                 "[output-directory]\n  social_cli analyze DATA_DIRECTORY BUDGET OUTPUT_PREFIX "
                 "[shock]\n  social_cli generate USERS OUTPUT_DIRECTORY\n  social_cli workbench\n  "
                 "social_cli match naive|rk|fa|kmp TEXT PATTERN\n  "
                 "social_cli route DATA_DIRECTORY SOURCE_ID TARGET_ID dijkstra|bellman|floyd\n  "
                 "social_cli menu\n";
}
int main(int argc, char **argv) {
    try {
        std::string command = argc > 1 ? argv[1] : "menu";
        if (command == "--help" || command == "help") {
            usage();
            return 0;
        }
        if (command == "workbench" && argc == 2) {
            run_workbench(std::cout);
            return 0;
        }
        if (command == "match" && argc == 5) {
            const std::map<std::string, Matcher> methods = {{"naive", Matcher::Naive},
                                                            {"rk", Matcher::RabinKarp},
                                                            {"fa", Matcher::Automaton},
                                                            {"kmp", Matcher::KMP}};
            auto method = methods.find(argv[2]);
            if (method == methods.end())
                throw std::invalid_argument("unknown string matching algorithm");
            for (auto offset : match(argv[3], argv[4], method->second))
                std::cout << offset << '\n';
            return 0;
        }
        if (command == "route" && argc == 6) {
            auto net = load_network(argv[2]);
            const int source = net.user_index.at(number(argv[3]));
            const int target = net.user_index.at(number(argv[4]));
            const std::string method = argv[5];
            Graph active(net.graph.n);
            for (auto e : net.graph.edges)
                if (e.capacity > 0)
                    active.add(e.from, e.to, e.weight, e.capacity);
            if (method == "floyd") {
                auto distance = floyd_warshall(active)[source][target];
                if (distance == INF)
                    std::cout << "unreachable\n";
                else
                    std::cout << "delay=" << distance << '\n';
            } else {
                if (method != "dijkstra" && method != "bellman")
                    throw std::invalid_argument("unknown routing algorithm");
                auto paths =
                    method == "dijkstra" ? dijkstra(active, source) : bellman_ford(active, source);
                auto path = path_to(paths, target);
                if (path.empty())
                    std::cout << "unreachable\n";
                else {
                    std::cout << "delay=" << paths.distance[target] << "; path:";
                    for (int user : path)
                        std::cout << ' ' << net.users[user].id;
                    std::cout << '\n';
                }
            }
            return 0;
        }
        if (command == "generate" && argc == 4) {
            save_network(generate_network(number(argv[2])), argv[3]);
            return 0;
        }
        if (command == "analyze" && (argc == 5 || argc == 6)) {
            if (argc == 6 && std::string(argv[5]) != "shock")
                throw std::invalid_argument("expected shock");
            report(load_network(argv[2]), number(argv[3]), argc == 6, argv[4]);
            return 0;
        }
        if (command == "demo" && (argc == 2 || argc == 3)) {
            std::string dir = argc == 3 ? argv[2] : "results/demo";
            auto net = generate_network(12);
            report(net, 35, false, dir + "/normal");
            report(net, 10, true, dir + "/shock");
            report(generate_network(10000), 35, false, dir + "/stress");
            run_workbench(std::cout);
            return 0;
        }
        if (command == "menu" && argc <= 2) {
            std::optional<Network> net;
            while (true) {
                std::cout << "\n1 Load CSV directory\n2 Generate network\n3 Analyze and export\n4 "
                             "Capacity shock\n5 Algorithm workbench\n0 Exit\n> "
                          << std::flush;
                std::string choice;
                if (!std::getline(std::cin, choice) || choice == "0")
                    return 0;
                try {
                    if (choice == "1") {
                        std::cout << "Directory: " << std::flush;
                        std::string path;
                        if (!std::getline(std::cin, path))
                            return 0;
                        net = load_network(path);
                        std::cout << "Loaded " << net->users.size() << " users\n";
                    } else if (choice == "2") {
                        std::cout << "User count: " << std::flush;
                        std::string count;
                        if (!std::getline(std::cin, count))
                            return 0;
                        net = generate_network(number(count));
                    } else if (choice == "3" || choice == "4") {
                        if (!net)
                            throw std::invalid_argument("load or generate a network first");
                        std::cout << "Budget: " << std::flush;
                        std::string cap;
                        if (!std::getline(std::cin, cap))
                            return 0;
                        std::cout << "Output prefix: " << std::flush;
                        std::string prefix;
                        if (!std::getline(std::cin, prefix))
                            return 0;
                        report(*net, number(cap), choice == "4", prefix);
                    } else if (choice == "5")
                        run_workbench(std::cout);
                    else
                        std::cout << "Unknown menu option\n";
                } catch (const std::exception &e) {
                    std::cerr << "Error: " << e.what() << '\n';
                }
            }
        }
        usage();
        return 2;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
