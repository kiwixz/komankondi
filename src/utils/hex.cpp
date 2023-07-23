#include "hex.hpp"

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "utils/exception.hpp"

namespace komankondi {

int from_hex(char digit) {
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    throw Exception{"could not parse hex digit '{}' ({:d})", digit, digit};
}

std::vector<std::byte> from_hex(std::string_view str) {
    if (str.size() % 2 != 0)
        throw Exception{"could not parse hex string '{}': extra nibble", str};
    std::vector<std::byte> r;
    r.resize(str.size() / 2);
    for (size_t i = 0; i < r.size(); ++i) {
        r[i] = static_cast<std::byte>((from_hex(str[i * 2]) << 4)
                                      | from_hex(str[i * 2 + 1]));
    }
    return r;
}

char to_hex(int nibble) {
    if (nibble < 0 || nibble > 15)
        throw Exception{"could not output hex digit for nibble {}", nibble};
    return nibble > 9 ? 'a' + nibble - 10 : '0' + nibble;
}

std::string to_hex(std::span<const std::byte> bin) {
    std::string r;
    r.resize(bin.size() * 2);
    for (size_t i = 0; i < bin.size(); ++i) {
        r[i * 2 + 0] = to_hex(static_cast<int>(bin[i]) >> 4);
        r[i * 2 + 1] = to_hex(static_cast<int>(bin[i]) & 0xf);
    }
    return r;
}

}  // namespace komankondi
