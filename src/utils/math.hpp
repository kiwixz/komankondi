#pragma once

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>

namespace komankondi {

template <std::integral T>
constexpr T pow_mod(T n, T exp, T mod) {
    assert(exp >= 0);
    assert(mod <= std::numeric_limits<uint32_t>::max());

    n %= mod;
    uint64_t r = 1;
    while (exp > 0) {
        if (exp % 2 == 1) {
            r = (r * n) % mod;
        }
        n = (static_cast<uint64_t>(n) * n) % mod;
        exp /= 2;
    }
    return r;
}

constexpr auto ceil_div(std::integral auto num, std::integral auto den) {
    return (num + den - 1) / den;
}

constexpr auto ceil(std::integral auto num, std::integral auto den) {
    return ceil_div(num, den) * den;
}

constexpr auto floor(std::integral auto a, std::integral auto b) {
    return a / b * b;
}

constexpr auto split_div(std::integral auto num, std::integral auto index, std::integral auto den) {
    return ceil_div(num - index, den);
}

}  // namespace komankondi
