#include "utils/scope_exit.hpp"

#include <catch2/catch_test_macros.hpp>

namespace komankondi {

TEST_CASE("scope_exit") {
    int c = 0;
    {
        ScopeExit scope_exit{[&] { ++c; }};
        CHECK(c == 0);
    }
    CHECK(c == 1);
}

}  // namespace komankondi
