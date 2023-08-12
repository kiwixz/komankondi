#pragma once

#include <charconv>
#include <concepts>
#include <string_view>
#include <system_error>

#include "utils/exception.hpp"

namespace komankondi {

template <std::integral T>
T parse(std::string_view str, int base = 10) {
    T r;
    std::from_chars_result res = std::from_chars(str.data(), str.data() + str.size(), r, base);
    if (res.ec != std::errc{})
        throw SystemException{res.ec, "could not parse integer: '{}'", str};
    if (res.ptr != str.data() + str.size())
        throw Exception{"could not parse integer: trailing '{}' from '{}'", std::string_view{res.ptr, str.size() - (res.ptr - str.data())}, str};
    return r;
}

}  // namespace komankondi
