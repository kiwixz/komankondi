#include "dictgen/pipeline.hpp"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace komankondi::dictgen {

TEST_CASE("pipeline") {
    int out = 0;

    make_pipeline<int>(+[](int) {})(0);
    make_pipeline<int>([](int) mutable {})(0);
    make_pipeline<int>([uncopyable = std::unique_ptr<int>{}](int) {})(0);

    make_pipeline<int>([&](int a) { out += a; })(7);
    CHECK(out == 7);

    make_pipeline<int>([](int a) { return a * 2; },
                       [&](int a) { out += a; })(5);
    CHECK(out == 17);
}

}  // namespace komankondi::dictgen
