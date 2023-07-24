#include "dictgen/pipeline.hpp"

#include <catch2/catch_test_macros.hpp>

namespace komankondi::dictgen {

TEST_CASE("pipeline") {
    int out = 0;

    Pipeline{+[](int) {}}.in(0);

    Pipeline{[&](int a) { out += a; }}.in(7);
    CHECK(out == 7);

    Pipeline{[](int a) { return a * 2; },
             [&](int a) { out += a; }}
            .in(5);
    CHECK(out == 17);
}

}  // namespace komankondi::dictgen
