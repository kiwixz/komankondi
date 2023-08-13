#include "iequal.hpp"

#include <string_view>

#include <range/v3/algorithm/equal.hpp>

namespace komankondi {

bool iequal(std::string_view a, std::string_view b) {
    return ranges::equal(a, b, [](char ac, char bc) {
        auto to_lower = [](char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; };
        return to_lower(ac) == to_lower(bc);
    });
}

}  // namespace komankondi
