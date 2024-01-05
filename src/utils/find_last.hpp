#pragma once

#include <algorithm>

namespace komankondi {

auto find_last(auto&& range, auto value) {
    auto it = std::find(range.rbegin(), range.rend(), value);
    if (it == range.rend())
        return range.end();
    return range.begin() + (range.rend() - it - 1);
}

}  // namespace komankondi
