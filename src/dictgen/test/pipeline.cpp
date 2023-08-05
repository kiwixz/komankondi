#include "dictgen/pipeline.hpp"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace komankondi::dictgen {

TEST_CASE("pipeline") {
    int out = 0;

    Pipeline{make_pipe<int>(+[](int) {})}(0);
    Pipeline{make_pipe<int>([](int) mutable {})}(0);
    Pipeline{make_pipe<int>([uncopyable = std::unique_ptr<int>{}](int) {})}(0);

    Pipeline{make_pipe<int>([&](int a) { out += a; })}(7);
    CHECK(out == 7);

    Pipeline{make_pipe<int>([](int a) { return a * 2; }),
             make_pipe<int>([&](int a) { out += a; })}(5);
    CHECK(out == 17);

    Pipeline{make_pipe<int>([](int a, auto&& sink) { sink(a * 2); }),
             make_pipe<int>([&](int a) { out += a; })}(8);
    CHECK(out == 33);
}

}  // namespace komankondi::dictgen
