#include "utils/zstring_view.hpp"

#include <catch2/catch_test_macros.hpp>

namespace komankondi {

TEST_CASE("zstring_view") {
    CHECK(ZStringView{""} == ZStringView{""});
    CHECK(ZStringView{"a"} != ZStringView{"b"});
    CHECK(ZStringView{""} == std::string{""});
    CHECK(ZStringView{"a"} != std::string{"b"});
    CHECK(ZStringView{""} == std::string_view{""});
    CHECK(ZStringView{"a"} != std::string_view{"b"});
    CHECK(ZStringView{""} == "");
    CHECK(ZStringView{"a"} != "b");
}

}  // namespace komankondi
