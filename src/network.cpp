#include "social/network.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
namespace social {
using Rows = std::vector<std::vector<std::string>>;
static Rows read_csv(const std::string &path, const std::vector<std::string> &header) {
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("Cannot open " + path);
    Rows rows;
    std::string line;
    bool first = true;
    std::size_t count = 0;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.size() > 1000000)
            throw std::invalid_argument("CSV line too long");
        std::vector<std::string> row;
        std::string field;
        bool quoted = false, closed = false;
        for (std::size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (quoted) {
                if (c == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        field += '"';
                        ++i;
                    } else {
                        quoted = false;
                        closed = true;
                    }
                } else
                    field += c;
            } else if (c == ',') {
                row.push_back(field);
                field.clear();
                closed = false;
            } else if (c == '"' && field.empty() && !closed)
                quoted = true;
            else {
                if (closed || c == '"')
                    throw std::invalid_argument("malformed CSV quote in " + path);
                field += c;
            }
        }
        if (quoted)
            throw std::invalid_argument("unterminated CSV quote in " + path);
        row.push_back(field);
        if (first) {
            if (row != header)
                throw std::invalid_argument("unexpected CSV header in " + path);
            first = false;
        } else {
            if (row.size() != header.size())
                throw std::invalid_argument("wrong field count in " + path);
            rows.push_back(std::move(row));
            if (++count > 1000000)
                throw std::invalid_argument("CSV exceeds one million records");
        }
    }
    if (first)
        throw std::invalid_argument("missing CSV header");
    return rows;
}
static i64 integer(const std::string &s, i64 lo, i64 hi) {
    std::size_t used = 0;
    i64 x;
    try {
        x = std::stoll(s, &used);
    } catch (...) {
        throw std::invalid_argument("invalid integer: " + s);
    }
    if (used != s.size() || s.empty() || s.front() == ' ' || x < lo || x > hi)
        throw std::invalid_argument("integer outside allowed range: " + s);
    return x;
}
Network load_network(const std::string &dir) {
    Network net;
    auto users = read_csv(dir + "/users.csv", {"id", "name", "campaign_cost"});
    for (auto &r : users) {
        int id = static_cast<int>(integer(r[0], 0, 1000000000));
        if (r[1].empty() || net.user_index.count(id))
            throw std::invalid_argument("duplicate user id or empty name");
        net.user_index[id] = static_cast<int>(net.users.size());
        net.users.push_back({id, r[1], static_cast<int>(integer(r[2], 1, 1000000))});
    }
    if (net.users.empty())
        throw std::invalid_argument("network needs at least one user");
    net.graph = Graph(static_cast<int>(net.users.size()));
    std::set<int> topic_ids, post_ids;
    for (auto &r : read_csv(dir + "/topics.csv", {"id", "name"})) {
        int id = static_cast<int>(integer(r[0], 0, 1000000000));
        if (!topic_ids.insert(id).second || r[1].empty())
            throw std::invalid_argument("duplicate topic or empty name");
        net.topics.push_back({id, r[1]});
    }
    for (auto &r : read_csv(dir + "/posts.csv", {"id", "user_id", "topic_id", "text"})) {
        int id = static_cast<int>(integer(r[0], 0, 1000000000)),
            u = static_cast<int>(integer(r[1], 0, 1000000000)),
            t = static_cast<int>(integer(r[2], 0, 1000000000));
        if (!post_ids.insert(id).second || !net.user_index.count(u) || !topic_ids.count(t))
            throw std::invalid_argument("invalid post reference or duplicate id");
        net.posts.push_back({id, u, t, r[3]});
    }
    for (auto &r : read_csv(dir + "/relationships.csv", {"from", "to", "delay", "capacity"})) {
        int u = static_cast<int>(integer(r[0], 0, 1000000000)),
            v = static_cast<int>(integer(r[1], 0, 1000000000));
        if (!net.user_index.count(u) || !net.user_index.count(v))
            throw std::invalid_argument("unknown relationship user");
        net.graph.add(net.user_index.at(u), net.user_index.at(v), integer(r[2], 1, 1000000000),
                      integer(r[3], 0, 1000000000));
    }
    i64 last = -1;
    for (auto &r :
         read_csv(dir + "/interactions.csv", {"from", "to", "post_id", "timestamp", "strength"})) {
        Interaction x{static_cast<int>(integer(r[0], 0, 1000000000)),
                      static_cast<int>(integer(r[1], 0, 1000000000)),
                      static_cast<int>(integer(r[2], 0, 1000000000)),
                      integer(r[3], 0, 1000000000000LL),
                      static_cast<int>(integer(r[4], 1, 1000000))};
        if (!net.user_index.count(x.from) || !net.user_index.count(x.to) ||
            !post_ids.count(x.post) || x.timestamp < last)
            throw std::invalid_argument("invalid interaction reference or unsorted timestamp");
        last = x.timestamp;
        net.interactions.push_back(x);
    }
    return net;
}
Network generate_network(int n, unsigned seed) {
    if (n < 2 || n > 100000)
        throw std::invalid_argument("generated user count must be 2..100000");
    Network net;
    net.graph = Graph(n);
    net.topics = {{0, "technology"}, {1, "education"}, {2, "community"}};
    std::mt19937 rng(seed);
    for (int i = 0; i < n; ++i) {
        net.user_index[i] = i;
        net.users.push_back({i, "user_" + std::to_string(i), 1 + static_cast<int>(rng() % 15)});
        net.posts.push_back(
            {i, i, i % 3, "community technology campaign post " + std::to_string(i)});
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < 4; ++j) {
            int target = j == 0 ? (i + 1) % n : static_cast<int>(rng() % n);
            if (target == i)
                continue;
            net.graph.add(i, target, 1 + rng() % 20, 1 + rng() % 30);
            net.interactions.push_back({i, target, i, static_cast<i64>(net.interactions.size()),
                                        1 + static_cast<int>(rng() % 10)});
        }
    return net;
}
static std::string csv(const std::string &x) {
    std::string out = "\"";
    for (char c : x) {
        if (c == '"')
            out += '"';
        out += c;
    }
    return out + '"';
}
void save_network(const Network &n, const std::string &dir) {
    std::filesystem::create_directories(dir);
    auto open = [&](const std::string &f) {
        std::ofstream out(dir + "/" + f);
        out.exceptions(std::ios::failbit | std::ios::badbit);
        return out;
    };
    auto u = open("users.csv");
    u << "id,name,campaign_cost\n";
    for (auto x : n.users)
        u << x.id << ',' << csv(x.name) << ',' << x.campaign_cost << '\n';
    auto t = open("topics.csv");
    t << "id,name\n";
    for (auto x : n.topics)
        t << x.id << ',' << csv(x.name) << '\n';
    auto p = open("posts.csv");
    p << "id,user_id,topic_id,text\n";
    for (auto x : n.posts)
        p << x.id << ',' << x.user << ',' << x.topic << ',' << csv(x.text) << '\n';
    auto e = open("relationships.csv");
    e << "from,to,delay,capacity\n";
    for (auto x : n.graph.edges)
        e << n.users[x.from].id << ',' << n.users[x.to].id << ',' << x.weight << ',' << x.capacity
          << '\n';
    auto a = open("interactions.csv");
    a << "from,to,post_id,timestamp,strength\n";
    for (auto x : n.interactions)
        a << x.from << ',' << x.to << ',' << x.post << ',' << x.timestamp << ',' << x.strength
          << '\n';
}
Analysis analyze(const Network &net, int budget, bool shock) {
    if (budget < 0 || budget > 1000000)
        throw std::invalid_argument("budget must be 0..1000000");
    Analysis a;
    a.budget = budget;
    auto g = net.graph;
    if (shock)
        for (auto &e : g.edges)
            e.capacity /= 4;
    auto n = net.users.size();
    std::vector<int> incoming(n);
    std::vector<i64> engagement(n);
    for (auto e : g.edges)
        ++incoming[e.to];
    for (auto x : net.interactions)
        engagement[net.user_index.at(x.to)] += x.strength;
    for (std::size_t i = 0; i < n; ++i)
        a.rankings.push_back({net.users[i].id, net.users[i].name,
                              incoming[i] * 10LL + engagement[i], incoming[i], engagement[i]});
    merge_sort(a.rankings, [](auto x, auto y) {
        return x.score > y.score || (x.score == y.score && x.id < y.id);
    });
    // Communities are weak components after filtering to active, fast links (delay <=5).
    a.community.assign(n, -1);
    std::vector<std::vector<int>> adjacency(n);
    for (auto e : g.edges)
        if (e.capacity > 0 && e.weight <= 5) {
            adjacency[e.from].push_back(e.to);
            adjacency[e.to].push_back(e.from);
        }
    for (std::size_t i = 0; i < n; ++i)
        if (a.community[i] < 0) {
            std::vector<int> stack{static_cast<int>(i)};
            a.community[i] = a.community_count;
            while (!stack.empty()) {
                int u = stack.back();
                stack.pop_back();
                for (int v : adjacency[u])
                    if (a.community[v] < 0) {
                        a.community[v] = a.community_count;
                        stack.push_back(v);
                    }
            }
            ++a.community_count;
        }
    // Capacity-zero links cannot carry dissemination traffic; positive capacities do not change
    // delay.
    Graph active(g.n);
    for (auto e : g.edges)
        if (e.capacity > 0)
            active.add(e.from, e.to, e.weight, e.capacity);
    a.paths = dijkstra(active, 0);
    a.mst_weight = kruskal(active).weight;
    // Exact selection is bounded to the best 20 candidates; the report discloses this pool.
    std::vector<Item> items;
    for (std::size_t i = 0; i < std::min<std::size_t>(20, n); ++i) {
        auto r = a.rankings[i];
        if (r.score > 1000000000)
            throw std::invalid_argument("campaign score exceeds solver bound");
        items.push_back(
            {net.users[net.user_index.at(r.id)].campaign_cost, static_cast<int>(r.score)});
    }
    a.campaign = knapsack_dp(items, budget);
    if (n > 1 && n <= 2000)
        a.flow = ford_fulkerson(g, 0, static_cast<int>(n - 1)).value;
    std::map<int, int> topic_index;
    for (std::size_t i = 0; i < net.topics.size(); ++i)
        topic_index[net.topics[i].id] = static_cast<int>(i);
    if (n <= 10000) {
        std::vector<std::vector<int>> eligible(n);
        for (auto &p : net.posts) {
            auto &row = eligible[net.user_index.at(p.user)];
            int t = topic_index.at(p.topic);
            if (std::find(row.begin(), row.end(), t) == row.end())
                row.push_back(t);
        }
        auto assigned = bipartite_matching(eligible, static_cast<int>(net.topics.size()));
        for (std::size_t i = 0; i < n; ++i)
            if (assigned[i] >= 0)
                a.creator_topics.push_back({net.users[i].id, net.topics[assigned[i]].id});
    }
    return a;
}
std::string json_escape(const std::string &s) {
    std::string out;
    constexpr char hex[] = "0123456789abcdef";
    for (unsigned char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (c < 32) {
                out += "\\u00";
                out += hex[c >> 4];
                out += hex[c & 15];
            } else
                out += static_cast<char>(c);
        }
    }
    return out;
}
void export_analysis(const Network &net, const Analysis &a, const std::string &prefix) {
    auto parent = std::filesystem::path(prefix).parent_path();
    if (!parent.empty())
        std::filesystem::create_directories(parent);
    std::ofstream c(prefix + ".csv"), j(prefix + ".json");
    if (!c || !j)
        throw std::runtime_error("cannot open report output");
    c.exceptions(std::ios::failbit | std::ios::badbit);
    j.exceptions(std::ios::failbit | std::ios::badbit);
    c << "id,name,score,incoming,engagement,community,delay_from_source\n";
    for (auto r : a.rankings) {
        int i = net.user_index.at(r.id);
        c << r.id << ',' << csv(r.name) << ',' << r.score << ',' << r.incoming << ','
          << r.engagement << ',' << a.community[i] << ',';
        if (a.paths.distance[i] != INF)
            c << a.paths.distance[i];
        c << '\n';
    }
    j << "{\n  \"users\": " << net.users.size() << ", \"communities\": " << a.community_count
      << ",\n  \"source_user\": " << net.users[0].id << ", \"budget\": " << a.budget
      << ",\n  \"ranking_formula\": \"10 * incoming_relationships + "
         "incoming_interaction_strength\",\n  \"community_rule\": \"weak components of "
         "positive-capacity links with delay <= 5\",\n  \"campaign_candidate_count\": "
      << std::min<std::size_t>(20, net.users.size())
      << ",\n  \"campaign_value\": " << a.campaign.value << ", \"campaign_users\": [";
    for (std::size_t i = 0; i < a.campaign.items.size(); ++i) {
        if (i)
            j << ',';
        j << a.rankings[a.campaign.items[i]].id;
    }
    j << "],\n  \"max_flow\": ";
    if (a.flow < 0)
        j << "null";
    else
        j << a.flow;
    j << ", \"mst_forest_weight\": " << a.mst_weight << ",\n  \"creator_topic_assignment\": [";
    for (std::size_t i = 0; i < a.creator_topics.size(); ++i) {
        if (i)
            j << ',';
        j << "{\"creator\":" << a.creator_topics[i].first
          << ",\"topic\":" << a.creator_topics[i].second << '}';
    }
    j << "],\n  \"rankings\": [";
    for (std::size_t k = 0; k < a.rankings.size(); ++k) {
        auto r = a.rankings[k];
        auto i = net.user_index.at(r.id);
        if (k)
            j << ',';
        j << "\n    {\"id\":" << r.id << ",\"name\":\"" << json_escape(r.name)
          << "\",\"score\":" << r.score << ",\"incoming\":" << r.incoming
          << ",\"engagement\":" << r.engagement << ",\"community\":" << a.community[i]
          << ",\"distance\":";
        if (a.paths.distance[i] == INF)
            j << "null";
        else
            j << a.paths.distance[i];
        j << ",\"path\":[";
        auto path = path_to(a.paths, i);
        for (std::size_t z = 0; z < path.size(); ++z) {
            if (z)
                j << ',';
            j << net.users[path[z]].id;
        }
        j << "]}";
    }
    j << "\n  ]\n}\n";
}
} // namespace social
