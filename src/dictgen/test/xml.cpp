#include "dictgen/xml.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/memory_input.hpp>
#include <tao/pegtl/parse.hpp>

namespace komankondi::dictgen {
namespace {

void parse(std::string_view document, std::initializer_list<std::pair<std::string_view, std::string_view>> expected) {
    CAPTURE(document);
    auto it = expected.begin();
    xml::actions::iterate(tao::pegtl::memory_input{document, ""},
                          [&](std::string&& path, std::string&& text) {
                              INFO(fmt::format("element {}", it - expected.begin()));
                              REQUIRE(it != expected.end());
                              CHECK(path == it->first);
                              CHECK(text == it->second);
                              ++it;
                          });
    CHECK(it == expected.end());
}

}  // namespace


TEST_CASE("analyze") {
    CHECK(tao::pegtl::analyze<xml::Document>() == 0);
}

TEST_CASE("one") {
    parse("<one></one>", {{"one", ""}});
}

TEST_CASE("simple") {
    parse(R"(
        <?xml version="1.0"?>
        <root>
            textroot
            <a>texta</a>
            <b>textb</b>
        </root>
    )",
          {
                  {"root/a", "texta"},
                  {"root/b", "textb"},
                  {"root", "textroot"},
          });
}

}  // namespace komankondi::dictgen
