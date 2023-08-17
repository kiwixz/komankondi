#pragma once

#include <array>
#include <cstddef>

namespace komankondi {

template <int size_with_terminator>
struct StaticString {
    static constexpr int size = size_with_terminator - 1;

    std::array<char, size_with_terminator> data;

    consteval StaticString(const char (&str)[size_with_terminator]) :
            data{std::to_array(str)} {
    }

    consteval char operator[](size_t i) const {
        return data[i];
    }
};

}  // namespace komankondi
