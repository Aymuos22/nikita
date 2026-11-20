#include "social/algorithms.hpp"
#include <numeric>
namespace social {
static i64 inverse(i64 a, i64 m) {
    i64 oldr = a, r = m, olds = 1, s = 0;
    while (r) {
        i64 q = oldr / r;
        auto nr = oldr - q * r;
        oldr = r;
        r = nr;
        auto ns = olds - q * s;
        olds = s;
        s = ns;
    }
    if (oldr != 1)
        throw std::invalid_argument("moduli must be pairwise coprime");
    return (olds % m + m) % m;
}
// Overflow-free modular multiplication for moduli up to signed 64-bit max.
static i64 addmod(i64 a, i64 b, i64 m) {
    return a >= m - b ? a - (m - b) : a + b;
}
static i64 mulmod(i64 a, i64 b, i64 m) {
    i64 out = 0;
    while (b) {
        if (b & 1)
            out = addmod(out, a, m);
        b >>= 1;
        if (b)
            a = addmod(a, a, m);
    }
    return out;
}
i64 chinese_remainder(const std::vector<i64> &residues, const std::vector<i64> &moduli) {
    if (residues.size() != moduli.size())
        throw std::invalid_argument("CRT vector mismatch");
    i64 x = 0, product = 1;
    for (std::size_t i = 0; i < moduli.size(); ++i) {
        auto m = moduli[i];
        if (m <= 1 || m > 1000000000)
            throw std::invalid_argument("CRT moduli must be 2..1e9");
        if (std::gcd(product, m) != 1)
            throw std::invalid_argument("CRT moduli must be coprime");
        if (product > std::numeric_limits<i64>::max() / m)
            throw std::overflow_error("CRT modulus product overflow");
        i64 residue = residues[i] % m;
        if (residue < 0)
            residue += m;
        i64 delta = (residue - x % m + m) % m;
        auto t = mulmod(delta, inverse(product % m, m), m);
        x += product * t;
        product *= m;
    }
    return x;
}
static std::vector<std::size_t> prefix(const std::string &p) {
    std::vector<std::size_t> pi(p.size());
    for (std::size_t i = 1; i < p.size(); ++i) {
        auto j = pi[i - 1];
        while (j && p[i] != p[j])
            j = pi[j - 1];
        if (p[i] == p[j])
            ++j;
        pi[i] = j;
    }
    return pi;
}
std::vector<std::size_t> match(const std::string &t, const std::string &p, Matcher method) {
    std::vector<std::size_t> out;
    auto n = t.size(), m = p.size();
    if (!m) {
        for (std::size_t i = 0; i <= n; ++i)
            out.push_back(i);
        return out;
    }
    if (m > n)
        return out;
    if (method == Matcher::Naive) {
        for (std::size_t i = 0; i <= n - m; ++i) {
            std::size_t j = 0;
            while (j < m && t[i + j] == p[j])
                ++j;
            if (j == m)
                out.push_back(i);
        }
    } else if (method == Matcher::RabinKarp) {
        constexpr std::uint64_t mod = 1000000007, base = 257;
        std::uint64_t target = 0, hash = 0, power = 1;
        for (std::size_t i = 0; i < m; ++i) {
            target = (target * base + static_cast<unsigned char>(p[i])) % mod;
            hash = (hash * base + static_cast<unsigned char>(t[i])) % mod;
            if (i + 1 < m)
                power = power * base % mod;
        }
        for (std::size_t i = 0; i <= n - m; ++i) {
            if (hash == target && t.compare(i, m, p) == 0)
                out.push_back(i);
            if (i < n - m) {
                hash = (hash + mod - static_cast<unsigned char>(t[i]) * power % mod) % mod;
                hash = (hash * base + static_cast<unsigned char>(t[i + m])) % mod;
            }
        }
    } else if (method == Matcher::KMP) {
        auto pi = prefix(p);
        std::size_t j = 0;
        for (std::size_t i = 0; i < n; ++i) {
            while (j && t[i] != p[j])
                j = pi[j - 1];
            if (t[i] == p[j])
                ++j;
            if (j == m) {
                out.push_back(i + 1 - m);
                j = pi[j - 1];
            }
        }
    } else {
        if (m > 100000)
            throw std::invalid_argument("automaton limited to 100000 pattern bytes");
        auto pi = prefix(p);
        std::vector<std::array<std::size_t, 256>> delta(m + 1);
        for (std::size_t q = 0; q <= m; ++q)
            for (int c = 0; c < 256; ++c) {
                if (q < m && static_cast<unsigned char>(p[q]) == c)
                    delta[q][c] = q + 1;
                else
                    delta[q][c] = q ? delta[pi[q - 1]][c] : 0;
            }
        std::size_t q = 0;
        for (std::size_t i = 0; i < n; ++i) {
            q = delta[q][static_cast<unsigned char>(t[i])];
            if (q == m)
                out.push_back(i + 1 - m);
        }
    }
    return out;
}
} // namespace social
