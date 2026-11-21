#include "social/algorithms.hpp"
#include <cmath>
#include <memory>
#include <numeric>
namespace social {
int binary_search_iterative(const std::vector<i64> &a, i64 x) {
    std::size_t l = 0, r = a.size();
    while (l < r) {
        auto mid = l + (r - l) / 2;
        if (a[mid] < x)
            l = mid + 1;
        else
            r = mid;
    }
    return l < a.size() && a[l] == x ? static_cast<int>(l) : -1;
}
int binary_search_recursive(const std::vector<i64> &a, i64 x) {
    std::function<std::size_t(std::size_t, std::size_t)> f = [&](auto l, auto r) -> std::size_t {
        if (l == r)
            return l;
        auto mid = l + (r - l) / 2;
        return a[mid] < x ? f(mid + 1, r) : f(l, mid);
    };
    auto p = f(0, a.size());
    return p < a.size() && a[p] == x ? static_cast<int>(p) : -1;
}
Subarray maximum_subarray(const std::vector<i64> &a) {
    if (a.empty() || a.size() > 1000000)
        throw std::invalid_argument("maximum subarray requires 1..1000000 values");
    for (auto x : a)
        if (x < -1000000000LL || x > 1000000000LL)
            throw std::invalid_argument("subarray values must be within +/-1e9");
    std::function<Subarray(std::size_t, std::size_t)> f = [&](auto l, auto r) -> Subarray {
        if (r - l == 1)
            return {a[l], l, r};
        auto mid = l + (r - l) / 2;
        auto left = f(l, mid), right = f(mid, r);
        i64 sum = 0, ls = -INF, rs = -INF;
        auto b = mid, e = mid;
        for (auto i = mid; i > l;) {
            sum += a[--i];
            if (sum > ls) {
                ls = sum;
                b = i;
            }
        }
        sum = 0;
        for (auto i = mid; i < r; ++i) {
            sum += a[i];
            if (sum > rs) {
                rs = sum;
                e = i + 1;
            }
        }
        Subarray best = left.sum >= right.sum ? left : right;
        return ls + rs > best.sum ? Subarray{ls + rs, b, e} : best;
    };
    return f(0, a.size());
}
static i64 checked_mul(i64 a, i64 b) {
    if (a > 0 ? (b > 0 ? a > std::numeric_limits<i64>::max() / b
                       : b < std::numeric_limits<i64>::min() / a)
              : (a < 0 && (b > 0 ? a < std::numeric_limits<i64>::min() / b
                                 : b < 0 && a < std::numeric_limits<i64>::max() / b)))
        throw std::overflow_error("power overflow");
    return a * b;
}
i64 fast_power(i64 a, unsigned n) {
    if (n == 0)
        return 1;
    const i64 half = fast_power(a, n / 2);
    const i64 square = checked_mul(half, half);
    return (n & 1) ? checked_mul(square, a) : square;
}
int celebrity(const std::vector<std::vector<int>> &k) {
    for (auto &r : k)
        if (r.size() != k.size())
            throw std::invalid_argument("knows must be square");
    if (k.empty())
        return -1;
    // Each half contributes a candidate. The directed relation eliminates one;
    // final verification rejects the survivor when no global celebrity exists.
    std::function<std::size_t(std::size_t, std::size_t)> candidate =
        [&](auto left, auto right) -> std::size_t {
        if (right - left == 1)
            return left;
        auto mid = left + (right - left) / 2;
        auto a = candidate(left, mid), b = candidate(mid, right);
        return k[a][b] ? b : a;
    };
    const auto c = candidate(0, k.size());
    for (std::size_t i = 0; i < k.size(); ++i)
        if (i != c && (k[c][i] || !k[i][c]))
            return -1;
    return static_cast<int>(c);
}
static std::size_t shape(const Matrix &a) {
    auto n = a.size();
    for (auto &r : a)
        if (r.size() != n)
            throw std::invalid_argument("square matrix required");
    return n;
}
static Matrix classical_kernel(const Matrix &a, const Matrix &b) {
    auto n = shape(a);
    if (shape(b) != n)
        throw std::invalid_argument("matrix size mismatch");
    Matrix c(n, std::vector<i64>(n));
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t k = 0; k < n; ++k)
            for (std::size_t j = 0; j < n; ++j)
                c[i][j] += a[i][k] * b[k][j];
    return c;
}
static void validate_matrix(const Matrix &a) {
    if (a.size() > 256)
        throw std::invalid_argument("matrix limit is 256");
    shape(a);
    for (auto &row : a)
        for (auto x : row)
            if (x < -1000000 || x > 1000000)
                throw std::invalid_argument("matrix entries must be within +/-1e6");
}
Matrix classical_multiply(const Matrix &a, const Matrix &b) {
    validate_matrix(a);
    validate_matrix(b);
    return classical_kernel(a, b);
}
static Matrix add(const Matrix &a, const Matrix &b, int sign = 1) {
    auto c = a;
    for (std::size_t i = 0; i < a.size(); ++i)
        for (std::size_t j = 0; j < a.size(); ++j)
            c[i][j] += sign * b[i][j];
    return c;
}
static Matrix strassen(const Matrix &a, const Matrix &b) {
    auto n = a.size();
    if (n <= 2)
        return classical_kernel(a, b);
    auto h = n / 2;
    std::array<Matrix, 4> x, y;
    for (int q = 0; q < 4; ++q) {
        x[q] = y[q] = Matrix(h, std::vector<i64>(h));
        for (std::size_t i = 0; i < h; ++i)
            for (std::size_t j = 0; j < h; ++j) {
                x[q][i][j] = a[i + (q / 2) * h][j + (q % 2) * h];
                y[q][i][j] = b[i + (q / 2) * h][j + (q % 2) * h];
            }
    }
    auto p1 = strassen(add(x[0], x[3]), add(y[0], y[3]));
    auto p2 = strassen(add(x[2], x[3]), y[0]);
    auto p3 = strassen(x[0], add(y[1], y[3], -1));
    auto p4 = strassen(x[3], add(y[2], y[0], -1));
    auto p5 = strassen(add(x[0], x[1]), y[3]);
    auto p6 = strassen(add(x[2], x[0], -1), add(y[0], y[1]));
    auto p7 = strassen(add(x[1], x[3], -1), add(y[2], y[3]));
    std::array<Matrix, 4> z = {add(add(add(p1, p4), p5, -1), p7), add(p3, p5), add(p2, p4),
                               add(add(add(p1, p2, -1), p3), p6)};
    Matrix c(n, std::vector<i64>(n));
    for (int q = 0; q < 4; ++q)
        for (std::size_t i = 0; i < h; ++i)
            for (std::size_t j = 0; j < h; ++j)
                c[i + (q / 2) * h][j + (q % 2) * h] = z[q][i][j];
    return c;
}
Matrix strassen_multiply(const Matrix &a, const Matrix &b) {
    validate_matrix(a);
    validate_matrix(b);
    auto n = shape(a);
    if (shape(b) != n)
        throw std::invalid_argument("matrix size mismatch");
    if (!n)
        return {};
    std::size_t p = 1;
    while (p < n)
        p *= 2;
    Matrix x(p, std::vector<i64>(p)), y = x;
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j) {
            x[i][j] = a[i][j];
            y[i][j] = b[i][j];
        }
    auto c = strassen(x, y);
    c.resize(n);
    for (auto &r : c)
        r.resize(n);
    return c;
}
static void validate_intervals(const std::vector<Interval> &a) {
    for (auto x : a)
        if (x.start >= x.finish)
            throw std::invalid_argument("interval must have positive duration");
}
std::vector<int> activity_selection(std::vector<Interval> a) {
    validate_intervals(a);
    merge_sort(a, [](auto x, auto y) {
        return x.finish < y.finish || (x.finish == y.finish && x.id < y.id);
    });
    std::vector<int> out;
    int end = std::numeric_limits<int>::min();
    for (auto x : a)
        if (x.start >= end) {
            out.push_back(x.id);
            end = x.finish;
        }
    return out;
}
std::vector<int> interval_partition(const std::vector<Interval> &a) {
    validate_intervals(a);
    std::vector<std::pair<Interval, int>> order;
    for (std::size_t i = 0; i < a.size(); ++i)
        order.push_back({a[i], static_cast<int>(i)});
    merge_sort(order, [](auto x, auto y) { return x.first.start < y.first.start; });
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                        std::greater<std::pair<int, int>>>
        q;
    int rooms = 0;
    std::vector<int> result(a.size());
    for (auto x : order) {
        int room;
        if (!q.empty() && q.top().first <= x.first.start) {
            room = q.top().second;
            q.pop();
        } else
            room = rooms++;
        result[x.second] = room;
        q.push({x.first.finish, room});
    }
    return result;
}
HuffmanResult huffman_encode(const std::string &text) {
    struct Node {
        std::size_t freq;
        int ch, left, right;
    };
    std::array<std::size_t, 256> freq{};
    for (unsigned char c : text)
        ++freq[c];
    std::vector<Node> nodes;
    using P = std::pair<std::size_t, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> q;
    for (int c = 0; c < 256; ++c)
        if (freq[c]) {
            q.push({freq[c], static_cast<int>(nodes.size())});
            nodes.push_back({freq[c], c, -1, -1});
        }
    HuffmanResult out;
    if (q.empty())
        return out;
    while (q.size() > 1) {
        auto x = q.top();
        q.pop();
        auto y = q.top();
        q.pop();
        q.push({x.first + y.first, static_cast<int>(nodes.size())});
        nodes.push_back({x.first + y.first, -1, x.second, y.second});
    }
    std::function<void(int, std::string)> visit = [&](int i, std::string code) {
        auto n = nodes[i];
        if (n.ch >= 0) {
            out.codes[n.ch] = code.empty() ? "0" : code;
            return;
        }
        visit(n.left, code + "0");
        visit(n.right, code + "1");
    };
    visit(q.top().second, "");
    for (unsigned char c : text)
        out.bits += out.codes[c];
    return out;
}
std::string huffman_decode(const HuffmanResult &h) {
    struct N {
        int ch = -1;
        std::array<int, 2> next{{-1, -1}};
    };
    std::vector<N> trie(1);
    for (int c = 0; c < 256; ++c)
        if (!h.codes[c].empty()) {
            int p = 0;
            for (char b : h.codes[c]) {
                if (b != '0' && b != '1')
                    throw std::invalid_argument("invalid code");
                if (trie[p].ch >= 0)
                    throw std::invalid_argument("non-prefix code");
                int bit = b - '0';
                if (trie[p].next[bit] < 0) {
                    trie[p].next[bit] = static_cast<int>(trie.size());
                    trie.push_back({});
                }
                p = trie[p].next[bit];
            }
            if (trie[p].ch >= 0 || trie[p].next[0] >= 0 || trie[p].next[1] >= 0)
                throw std::invalid_argument("duplicate or non-prefix code");
            trie[p].ch = c;
        }
    std::string out;
    int p = 0;
    for (char b : h.bits) {
        if (b != '0' && b != '1')
            throw std::invalid_argument("invalid bit");
        p = trie[p].next[b - '0'];
        if (p < 0)
            throw std::invalid_argument("unknown code");
        if (trie[p].ch >= 0) {
            out += static_cast<char>(trie[p].ch);
            p = 0;
        }
    }
    if (p)
        throw std::invalid_argument("truncated code");
    return out;
}
} // namespace social
