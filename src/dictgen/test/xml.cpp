#include "dictgen/xml.hpp"

#include <string>
#include <string_view>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <tao/pegtl/contrib/analyze.hpp>

namespace komankondi::dictgen {
namespace {

void parse(std::string_view document, std::initializer_list<std::pair<std::string_view, std::string_view>> expected) {
    CAPTURE(document);
    auto it = expected.begin();
    xml::iterate(document, [&](std::string&& path, std::string&& text) {
        INFO(fmt::format("element {}", it - expected.begin()));
        REQUIRE(it != expected.end());
        CHECK(path == it->first);
        CHECK(text == it->second);
        ++it;
    });
    CHECK(it == expected.end());
}

}  // namespace


TEST_CASE("xml") {
    CHECK(tao::pegtl::analyze<xml::detail::Content>() == 0);

    parse("<one></one>", {{"one", ""}});
    CHECK_THROWS(parse("<one>", {{"one", ""}}));
    CHECK_THROWS(parse("</one>", {{"one", ""}}));

    parse("<?xml version=\"1.0\"?>"
          "<root>"
          " textroot"
          " <a>texta</a>"
          " <b some=\"attribute\"/>"
          " <c>textc</c>"
          " <d/>"
          "</root>",
          {
                  {"root/a", "texta"},
                  {"root/b", ""},
                  {"root/c", "textc"},
                  {"root/d", ""},
                  {"root", " textroot    "},
          });
}

TEST_CASE("xml_partial") {
    xml::Iterate parser;
    CHECK(parser.finished());

    parser("", [](std::string&& path, std::string&& text) {
        CAPTURE(path);
        CAPTURE(text);
        CHECK(false);
    });
    CHECK(parser.finished());

    parser("<", [](std::string&& path, std::string&& text) {
        CAPTURE(path);
        CAPTURE(text);
        CHECK(false);
    });
    CHECK(!parser.finished());

    parser("?xml ", [](std::string&& path, std::string&& text) {
        CAPTURE(path);
        CAPTURE(text);
        CHECK(false);
    });
    CHECK(!parser.finished());

    parser("version=\"1.0\"?><ro", [](std::string&& path, std::string&& text) {
        CAPTURE(path);
        CAPTURE(text);
        CHECK(false);
    });
    CHECK(!parser.finished());

    parser("", [](std::string&& path, std::string&& text) {
        CAPTURE(path);
        CAPTURE(text);
        CHECK(false);
    });
    CHECK(!parser.finished());

    parser("ot> textroot<a>te", [](std::string&& path, std::string&& text) {
        CAPTURE(path);
        CAPTURE(text);
        CHECK(false);
    });
    CHECK(!parser.finished());

    int i = 0;
    parser("xta</a><b>textb</b>", [&](std::string&& path, std::string&& text) {
        switch (i) {
        case 0:
            CHECK(path == "root/a");
            CHECK(text == "texta");
            break;
        case 1:
            CHECK(path == "root/b");
            CHECK(text == "textb");
            break;
        default:
            CAPTURE(path);
            CAPTURE(text);
            CHECK(false);
        }
        ++i;
    });
    CHECK(i == 2);
    CHECK(!parser.finished());

    i = 0;
    parser("</root>", [&](std::string&& path, std::string&& text) {
        if (i == 0) {
            CHECK(path == "root");
            CHECK(text == " textroot");
        }
        else {
            CAPTURE(path);
            CAPTURE(text);
            CHECK(false);
        }
        ++i;
    });
    CHECK(i == 1);
    CHECK(parser.finished());
}

}  // namespace komankondi::dictgen
