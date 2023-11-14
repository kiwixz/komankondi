#include "utils/log.hpp"

#include <span>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

namespace komankondi {

TEST_CASE("log_hexdump") {
    std::string input = "\t !\"#$%&'()*+,-./0z";
    input += '\0';
    input += "😎";

    std::string_view expected = "  00000000  0920 2122 2324 2526 2728 292a 2b2c 2d2e  . !\"#$%&'()*+,-.\n"
                                "  00000010  2f30 7a00 f09f 988e                      /0z.....\n";

    CHECK(fmt::format("{}", log::Hexdump{input}) == expected);
    CHECK(fmt::format("{}", log::Hexdump{std::string_view{input}}) == expected);
    CHECK(fmt::format("{}", log::Hexdump{std::vector<char>{input.begin(), input.end()}}) == expected);
    CHECK(fmt::format("{}", log::Hexdump{std::span<const char>{input}}) == expected);
    CHECK(fmt::format("{}", log::Hexdump{std::span<char>{input}}) == expected);
    CHECK(fmt::format("{}", log::Hexdump{std::span<const std::byte>{reinterpret_cast<const std::byte*>(input.data()), input.size()}}) == expected);
    CHECK(fmt::format("{}", log::Hexdump{std::span<std::byte>{reinterpret_cast<std::byte*>(input.data()), input.size()}}) == expected);
}

}  // namespace komankondi
