#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace komankondi {

int from_hex(char digit);
std::vector<std::byte> from_hex(std::string_view str);

char to_hex(int nibble);
std::string to_hex(std::span<const std::byte> bin);

}  // namespace komankondi
