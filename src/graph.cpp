#include "social/algorithms.hpp"
#include <numeric>
namespace social {
Graph::Graph(int vertices) : n(vertices) {
    if (n < 0 || n > 100000)
        throw std::invalid_argument("vertex count must be 0..100000");
}
void Graph::add(int u, int v, i64 w, i64 cap) {
    if (u < 0 || v < 0 || u >= n || v >= n || w < -1000000000 || w > 1000000000 || cap < 0 ||
        cap > 1000000000)
        throw std::invalid_argument("invalid edge endpoint, weight or capacity");
    edges.push_back({u, v, w, cap});
}
static void vertex(const Graph &g, int s) {
    if (s < 0 || s >= g.n)
        throw std::invalid_argument("invalid source/target");
}
static i64 plus(i64 a, i64 b) {
    if (a == INF || b == INF)
        return INF;
    return std::max(-INF, std::min(INF, a + b));
}
Paths dijkstra(const Graph &g, int s) {
    vertex(g, s);
    std::vector<std::vector<std::pair<int, i64>>> adj(g.n);
    for (auto e : g.edges) {
        if (e.weight < 0)
            throw std::invalid_argument("Dijkstra requires nonnegative weights");
        adj[e.from].push_back({e.to, e.weight});
    }
    Paths p{std::vector<i64>(g.n, INF), std::vector<int>(g.n, -1), false};
    using P = std::pair<i64, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> q;
    p.distance[s] = 0;
    q.push({0, s});
    while (!q.empty()) {
        auto [d, u] = q.top();
        q.pop();
        if (d != p.distance[u])
            continue;
        for (auto [v, w] : adj[u])
            if (d + w < p.distance[v]) {
                p.distance[v] = d + w;
                p.parent[v] = u;
                q.push({d + w, v});
            }
    }
    return p;
}
Paths bellman_ford(const Graph &g, int s) {
    vertex(g, s);
    Paths p{std::vector<i64>(g.n, INF), std::vector<int>(g.n, -1), false};
    p.distance[s] = 0;
    for (int i = 0; i < g.n; ++i) {
        bool changed = false;
        for (auto e : g.edges)
            if (p.distance[e.from] != INF &&
                plus(p.distance[e.from], e.weight) < p.distance[e.to]) {
                p.distance[e.to] = plus(p.distance[e.from], e.weight);
                p.parent[e.to] = e.from;
                changed = true;
                if (i == g.n - 1)
                    p.negative_cycle = true;
            }
        if (!changed)
            break;
    }
    return p;
}
std::vector<std::vector<i64>> floyd_warshall(const Graph &g) {
    if (g.n > 600)
        throw std::invalid_argument("Floyd-Warshall limited to 600 vertices");
    Matrix d(g.n, std::vector<i64>(g.n, INF));
    for (int i = 0; i < g.n; ++i)
        d[i][i] = 0;
    for (auto e : g.edges)
        d[e.from][e.to] = std::min(d[e.from][e.to], e.weight);
    for (int k = 0; k < g.n; ++k)
        for (int i = 0; i < g.n; ++i)
            for (int j = 0; j < g.n; ++j)
                d[i][j] = std::min(d[i][j], plus(d[i][k], d[k][j]));
    for (int i = 0; i < g.n; ++i)
        if (d[i][i] < 0)
            throw std::domain_error("negative cycle: all-pairs distances undefined");
    return d;
}
std::vector<int> path_to(const Paths &p, int t) {
    if (t < 0 || static_cast<std::size_t>(t) >= p.distance.size())
        throw std::invalid_argument("invalid path target");
    if (p.negative_cycle)
        throw std::domain_error("negative cycle invalidates paths");
    if (p.distance[t] == INF)
        return {};
    std::vector<int> out;
    for (int v = t; v != -1; v = p.parent[v]) {
        if (out.size() >= p.distance.size())
            throw std::logic_error("cyclic predecessor chain");
        out.push_back(v);
    }
    std::reverse(out.begin(), out.end());
    return out;
}
struct DSU {
    std::vector<int> p, size;
    int components;
    explicit DSU(int n) : p(n), size(n, 1), components(n) {
        std::iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }
    bool join(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b)
            return false;
        if (size[a] < size[b])
            std::swap(a, b);
        p[b] = a;
        size[a] += size[b];
        --components;
        return true;
    }
};
Forest kruskal(const Graph &g) {
    auto edges = g.edges;
    merge_sort(edges, [](auto a, auto b) { return a.weight < b.weight; });
    DSU sets(g.n);
    Forest f;
    for (auto e : edges)
        if (sets.join(e.from, e.to)) {
            f.edges.push_back(e);
            f.weight += e.weight;
        }
    f.components = sets.components;
    return f;
}
Forest prim(const Graph &g) {
    std::vector<std::vector<std::pair<int, int>>> adj(g.n);
    for (std::size_t i = 0; i < g.edges.size(); ++i) {
        auto e = g.edges[i];
        adj[e.from].push_back({e.to, static_cast<int>(i)});
        adj[e.to].push_back({e.from, static_cast<int>(i)});
    }
    std::vector<bool> seen(g.n);
    using P = std::pair<i64, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> q;
    Forest f;
    for (int s = 0; s < g.n; ++s) {
        if (seen[s])
            continue;
        ++f.components;
        seen[s] = true;
        for (auto [v, i] : adj[s])
            if (!seen[v])
                q.push({g.edges[i].weight, i});
        while (!q.empty()) {
            auto [w, i] = q.top();
            q.pop();
            auto e = g.edges[i];
            if (seen[e.from] && seen[e.to])
                continue;
            int v = seen[e.from] ? e.to : e.from;
            seen[v] = true;
            f.weight += w;
            f.edges.push_back(e);
            for (auto [next, j] : adj[v])
                if (!seen[next])
                    q.push({g.edges[j].weight, j});
        }
    }
    return f;
}
i64 tsp(const Matrix &a) {
    std::size_t n = a.size();
    if (n > 18)
        throw std::invalid_argument("TSP limited to 18 vertices");
    for (auto &r : a) {
        if (r.size() != n)
            throw std::invalid_argument("TSP needs square costs");
        for (auto x : r)
            if (x < 0 || (x != INF && x > 1000000000))
                throw std::invalid_argument("invalid TSP cost");
    }
    if (n < 2)
        return 0;
    auto states = std::size_t{1} << n;
    std::vector<i64> dp(states * n, INF);
    dp[n] = 0;
    for (std::size_t mask = 1; mask < states; mask += 2)
        for (std::size_t u = 0; u < n; ++u)
            if (dp[mask * n + u] != INF)
                for (std::size_t v = 1; v < n; ++v)
                    if (!(mask & (std::size_t{1} << v)) && a[u][v] != INF) {
                        auto next = mask | (std::size_t{1} << v);
                        dp[next * n + v] = std::min(dp[next * n + v], dp[mask * n + u] + a[u][v]);
                    }
    i64 best = INF;
    for (std::size_t u = 1; u < n; ++u)
        if (a[u][0] != INF)
            best = std::min(best, plus(dp[(states - 1) * n + u], a[u][0]));
    return best;
}
Flow ford_fulkerson(const Graph &g, int s, int t) {
    vertex(g, s);
    vertex(g, t);
    if (s == t)
        throw std::invalid_argument("source and sink must differ");
    if (g.n > 2000)
        throw std::invalid_argument("matrix flow limited to 2000 vertices");
    Flow f;
    f.residual = Matrix(g.n, std::vector<i64>(g.n));
    for (auto e : g.edges)
        f.residual[e.from][e.to] += e.capacity;
    while (true) {
        std::vector<int> parent(g.n, -1);
        std::vector<int> stack{s};
        parent[s] = s;
        while (!stack.empty() && parent[t] < 0) {
            int u = stack.back();
            stack.pop_back();
            for (int v = 0; v < g.n; ++v)
                if (parent[v] < 0 && f.residual[u][v] > 0) {
                    parent[v] = u;
                    stack.push_back(v);
                }
        }
        if (parent[t] < 0) {
            for (int v = 0; v < g.n; ++v)
                if (parent[v] >= 0)
                    f.source_side.push_back(v);
            break;
        }
        i64 amount = INF;
        for (int v = t; v != s; v = parent[v])
            amount = std::min(amount, f.residual[parent[v]][v]);
        for (int v = t; v != s; v = parent[v]) {
            f.residual[parent[v]][v] -= amount;
            f.residual[v][parent[v]] += amount;
        }
        f.value += amount;
    }
    return f;
}
std::vector<int> bipartite_matching(const std::vector<std::vector<int>> &adj, int right) {
    if (right < 0 || right > 100000 || adj.size() > 10000)
        throw std::invalid_argument("matching dimensions out of range");
    for (auto &row : adj)
        for (int v : row)
            if (v < 0 || v >= right)
                throw std::invalid_argument("invalid matching endpoint");
    std::vector<int> owner(right, -1), out(adj.size(), -1);
    std::function<bool(int, std::vector<bool> &)> augment = [&](int u, std::vector<bool> &seen) {
        for (int v : adj[u])
            if (!seen[v]) {
                seen[v] = true;
                if (owner[v] < 0 || augment(owner[v], seen)) {
                    owner[v] = u;
                    return true;
                }
            }
        return false;
    };
    for (std::size_t u = 0; u < adj.size(); ++u) {
        std::vector<bool> seen(right);
        augment(static_cast<int>(u), seen);
    }
    for (int v = 0; v < right; ++v)
        if (owner[v] >= 0)
            out[owner[v]] = v;
    return out;
}
} // namespace social
