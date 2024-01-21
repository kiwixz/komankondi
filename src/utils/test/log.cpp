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

TEST_CASE("log_bytes") {
    CHECK(fmt::format("{}", log::Bytes{}) == "0 B");
    CHECK(fmt::format("{}", log::Bytes{1023}) == "1023 B");
    CHECK(fmt::format("{}", log::Bytes{1024}) == "1.00 KiB");
    CHECK(fmt::format("{}", log::Bytes{1029}) == "1.00 KiB");
    CHECK(fmt::format("{}", log::Bytes{1030}) == "1.01 KiB");
    CHECK(fmt::format("{}", log::Bytes{10240}) == "10.00 KiB");
    CHECK(fmt::format("{}", log::Bytes{102400}) == "100.00 KiB");
    CHECK(fmt::format("{}", log::Bytes{1024000}) == "1000.00 KiB");
    CHECK(fmt::format("{}", log::Bytes{10240000}) == "9.77 MiB");
    CHECK(fmt::format("{}", log::Bytes{std::numeric_limits<uint64_t>::max()}) == "16.00 EiB");
}

}  // namespace komankondi
