#pragma once
#include "social/algorithms.hpp"
#include <iosfwd>
#include <map>

namespace social {
struct User {
    int id;
    std::string name;
    int campaign_cost;
};
struct Topic {
    int id;
    std::string name;
};
struct Post {
    int id, user, topic;
    std::string text;
};
struct Interaction {
    int from, to, post;
    i64 timestamp;
    int strength;
};
struct Network {
    std::vector<User> users;
    std::vector<Topic> topics;
    std::vector<Post> posts;
    std::vector<Interaction> interactions;
    Graph graph;
    std::map<int, int> user_index;
};
Network load_network(const std::string &directory);
Network generate_network(int n, unsigned seed = 15);
void save_network(const Network &network, const std::string &directory);
struct Rank {
    int id;
    std::string name;
    i64 score;
    int incoming;
    i64 engagement;
};
struct Analysis {
    std::vector<Rank> rankings;
    std::vector<int> community;
    int community_count = 0;
    Paths paths;
    Selection campaign;
    i64 flow = -1;
    i64 mst_weight = 0;
    int budget = 0;
    std::vector<std::pair<int, int>> creator_topics;
};
Analysis analyze(const Network &network, int budget, bool shock = false);
void export_analysis(const Network &network, const Analysis &result, const std::string &prefix);
std::string json_escape(const std::string &text);
void run_workbench(std::ostream &out);
void run_workbench(std::ostream &out, const Network &network, int budget, bool shock = false);
} // namespace social
