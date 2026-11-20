#include "social/algorithms.hpp"
#include <cmath>
#include <deque>
#include <numeric>
namespace social {
static void validate(const std::vector<Item> &a, int cap) {
    if (cap < 0 || cap > 1000000 || a.size() > 10000)
        throw std::invalid_argument("knapsack bounds exceeded");
    for (auto x : a)
        if (x.cost <= 0 || x.cost > 1000000 || x.value < 0 || x.value > 1000000000)
            throw std::invalid_argument("items need positive cost and nonnegative bounded value");
}
double fractional_knapsack(std::vector<Item> a, int cap) {
    validate(a, cap);
    merge_sort(a, [](auto x, auto y) {
        return static_cast<i64>(x.value) * y.cost > static_cast<i64>(y.value) * x.cost;
    });
    double value = 0;
    for (auto x : a) {
        int use = std::min(cap, x.cost);
        value += static_cast<double>(x.value) * use / x.cost;
        cap -= use;
        if (!cap)
            break;
    }
    return value;
}
Selection knapsack_dp(const std::vector<Item> &a, int cap) {
    validate(a, cap);
    if ((a.size() + 1) * static_cast<std::size_t>(cap + 1) > 10000000)
        throw std::invalid_argument("DP table exceeds 10 million cells");
    std::vector<std::vector<i64>> dp(a.size() + 1, std::vector<i64>(cap + 1));
    Selection out;
    for (std::size_t i = 1; i <= a.size(); ++i)
        for (int c = 0; c <= cap; ++c) {
            ++out.explored;
            dp[i][c] = dp[i - 1][c];
            if (a[i - 1].cost <= c)
                dp[i][c] = std::max(dp[i][c], dp[i - 1][c - a[i - 1].cost] + a[i - 1].value);
        }
    out.value = dp.back()[cap];
    int c = cap;
    for (auto i = a.size(); i > 0; --i)
        if (dp[i][c] != dp[i - 1][c]) {
            out.items.push_back(static_cast<int>(i - 1));
            c -= a[i - 1].cost;
        }
    std::reverse(out.items.begin(), out.items.end());
    return out;
}
Selection knapsack_backtracking(const std::vector<Item> &a, int cap) {
    validate(a, cap);
    if (a.size() > 25)
        throw std::invalid_argument("backtracking limited to 25 items");
    Selection best;
    std::vector<int> chosen;
    std::function<void(std::size_t, int, i64)> dfs = [&](auto i, int remaining, i64 value) {
        ++best.explored;
        if (value > best.value) {
            best.value = value;
            best.items = chosen;
        }
        if (i == a.size())
            return;
        if (a[i].cost <= remaining) {
            chosen.push_back(static_cast<int>(i));
            dfs(i + 1, remaining - a[i].cost, value + a[i].value);
            chosen.pop_back();
        }
        dfs(i + 1, remaining, value);
    };
    dfs(0, cap, 0);
    return best;
}
Selection knapsack_branch_bound(const std::vector<Item> &a, int cap, Frontier policy) {
    validate(a, cap);
    if (a.size() > 25)
        throw std::invalid_argument("branch and bound limited to 25 items");
    std::vector<int> order(a.size());
    std::iota(order.begin(), order.end(), 0);
    merge_sort(order, [&](int i, int j) {
        return static_cast<i64>(a[i].value) * a[j].cost > static_cast<i64>(a[j].value) * a[i].cost;
    });
    struct Node {
        std::size_t i;
        int cost;
        i64 value, upper;
        std::vector<int> chosen;
    };
    auto bound = [&](const Node &node) {
        i64 value = node.value;
        int remaining = cap - node.cost;
        for (auto i = node.i; i < order.size(); ++i) {
            auto x = a[order[i]];
            if (x.cost <= remaining) {
                value += x.value;
                remaining -= x.cost;
            } else {
                value += (static_cast<i64>(x.value) * remaining + x.cost - 1) / x.cost;
                break;
            }
        }
        return value;
    };
    auto cmp = [](const Node &x, const Node &y) { return x.upper < y.upper; };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> heap(cmp);
    std::deque<Node> q;
    auto push = [&](Node n) {
        if (q.size() + heap.size() >= 1000000)
            throw std::length_error("branch-and-bound frontier exceeds one million nodes");
        if (policy == Frontier::LeastCost)
            heap.push(std::move(n));
        else
            q.push_back(std::move(n));
    };
    Node root{0, 0, 0, 0, {}};
    root.upper = bound(root);
    push(root);
    Selection best;
    while (policy == Frontier::LeastCost ? !heap.empty() : !q.empty()) {
        Node x;
        if (policy == Frontier::LeastCost) {
            x = heap.top();
            heap.pop();
        } else if (policy == Frontier::FIFO) {
            x = q.front();
            q.pop_front();
        } else {
            x = q.back();
            q.pop_back();
        }
        ++best.explored;
        if (x.value > best.value) {
            best.value = x.value;
            best.items = x.chosen;
        }
        if (x.upper <= best.value || x.i == a.size())
            continue;
        int index = order[x.i];
        Node skip = x;
        ++skip.i;
        skip.upper = bound(skip);
        if (skip.upper > best.value)
            push(skip);
        if (x.cost + a[index].cost <= cap) {
            Node take = skip;
            take.cost += a[index].cost;
            take.value += a[index].value;
            take.chosen.push_back(index);
            take.upper = bound(take);
            if (take.upper > best.value)
                push(take);
        }
    }
    merge_sort(best.items);
    return best;
}
i64 matrix_chain(const std::vector<int> &d) {
    if (d.size() < 2)
        return 0;
    if (d.size() > 201)
        throw std::invalid_argument("matrix chain limited to 200 matrices");
    for (int x : d)
        if (x <= 0 || x > 10000)
            throw std::invalid_argument("dimensions must be 1..10000");
    auto n = d.size() - 1;
    Matrix dp(n, std::vector<i64>(n));
    for (std::size_t len = 2; len <= n; ++len)
        for (std::size_t i = 0; i + len <= n; ++i) {
            auto j = i + len - 1;
            dp[i][j] = INF;
            for (auto k = i; k < j; ++k)
                dp[i][j] = std::min(dp[i][j], dp[i][k] + dp[k + 1][j] +
                                                  static_cast<i64>(d[i]) * d[k + 1] * d[j + 1]);
        }
    return dp[0][n - 1];
}
std::string lcs(const std::string &a, const std::string &b) {
    if ((a.size() + 1) > 10000000 / (b.size() + 1))
        throw std::invalid_argument("LCS exceeds 10 million cells");
    std::vector<std::vector<int>> dp(a.size() + 1, std::vector<int>(b.size() + 1));
    for (std::size_t i = 1; i <= a.size(); ++i)
        for (std::size_t j = 1; j <= b.size(); ++j)
            dp[i][j] =
                a[i - 1] == b[j - 1] ? dp[i - 1][j - 1] + 1 : std::max(dp[i - 1][j], dp[i][j - 1]);
    std::string out;
    auto i = a.size(), j = b.size();
    while (i && j) {
        if (a[i - 1] == b[j - 1]) {
            out += a[--i];
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1])
            --i;
        else
            --j;
    }
    std::reverse(out.begin(), out.end());
    return out;
}
double optimal_bst(const std::vector<double> &p, const std::vector<double> &q) {
    auto n = p.size();
    if (q.size() != n + 1 || n > 300)
        throw std::invalid_argument("OBST needs n+1 miss weights, n <=300");
    for (double x : p)
        if (!std::isfinite(x) || x < 0)
            throw std::invalid_argument("invalid frequency");
    for (double x : q)
        if (!std::isfinite(x) || x < 0)
            throw std::invalid_argument("invalid frequency");
    std::vector<std::vector<double>> e(n + 1, std::vector<double>(n + 1)), w = e;
    for (std::size_t i = 0; i <= n; ++i)
        e[i][i] = w[i][i] = q[i];
    for (std::size_t len = 1; len <= n; ++len)
        for (std::size_t i = 0; i + len <= n; ++i) {
            auto j = i + len;
            w[i][j] = w[i][j - 1] + p[j - 1] + q[j];
            e[i][j] = std::numeric_limits<double>::infinity();
            for (auto r = i; r < j; ++r)
                e[i][j] = std::min(e[i][j], e[i][r] + e[r + 1][j] + w[i][j]);
        }
    return e[0][n];
}
std::vector<std::vector<int>> n_queens(int n) {
    if (n < 0 || n > 12)
        throw std::invalid_argument("N-Queens restricted to 0..12");
    std::vector<std::vector<int>> out;
    std::vector<int> pos;
    std::function<void(int)> dfs = [&](int r) {
        if (r == n) {
            out.push_back(pos);
            return;
        }
        for (int c = 0; c < n; ++c) {
            bool safe = true;
            for (int i = 0; i < r; ++i)
                if (pos[i] == c || std::abs(pos[i] - c) == r - i) {
                    safe = false;
                    break;
                }
            if (safe) {
                pos.push_back(c);
                dfs(r + 1);
                pos.pop_back();
            }
        }
    };
    dfs(0);
    return out;
}
std::vector<std::vector<int>> subset_sum(const std::vector<int> &a, i64 target) {
    if (a.size() > 22)
        throw std::invalid_argument("subset enumeration limited to 22 values");
    std::vector<std::vector<int>> out;
    std::vector<int> chosen;
    std::function<void(std::size_t, i64)> dfs = [&](auto i, i64 sum) {
        if (i == a.size()) {
            if (sum == target) {
                if (out.size() >= 100000)
                    throw std::length_error("subset output exceeds 100000 solutions");
                out.push_back(chosen);
            }
            return;
        }
        dfs(i + 1, sum);
        chosen.push_back(static_cast<int>(i));
        dfs(i + 1, sum + a[i]);
        chosen.pop_back();
    };
    dfs(0, 0);
    return out;
}
} // namespace social
