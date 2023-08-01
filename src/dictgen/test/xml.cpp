#include "dictgen/xml.hpp"

#include <initializer_list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <tao/pegtl/contrib/analyze.hpp>

namespace komankondi::dictgen {
namespace {

void check_iterate(std::string_view document, std::initializer_list<std::pair<std::string_view, std::string_view>> expected) {
    CAPTURE(document);
    auto it = expected.begin();
    xml::iterate(document, [&](std::string&& path, std::string&& text) {
        INFO(fmt::format("element {}", it - expected.begin()));
        INFO(fmt::format("got '{}' = '{}'", path, text));
        REQUIRE(it != expected.end());
        CHECK(path == it->first);
        CHECK(text == it->second);
        ++it;
    });
    CHECK(it == expected.end());
}

void check_select(std::string_view document, std::string&& path, std::vector<std::string>&& subpaths,
                  std::initializer_list<xml::Select::Element> expected) {
    CAPTURE(document);
    auto it = expected.begin();
    xml::select(document, std::move(path), std::move(subpaths), [&](xml::Select::Element&& el) {
        INFO(fmt::format("element {}", it - expected.begin()));
        INFO(fmt::format("got {}", el));
        REQUIRE(it != expected.end());
        INFO(fmt::format("expected {}", *it));
        CHECK(el == *it);
        ++it;
    });
    CHECK(it == expected.end());
}

}  // namespace


TEST_CASE("xml_grammar") {
    CHECK(tao::pegtl::analyze<xml::detail::Content>() == 0);
}

TEST_CASE("xml_iterate") {
    check_iterate("<one></one>", {{"one", ""}});
    CHECK_THROWS(check_iterate("<one>", {{"one", ""}}));
    CHECK_THROWS(check_iterate("</one>", {{"one", ""}}));

    check_iterate("<?xml version=\"1.0\"?>"
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

TEST_CASE("xml_iterate_partial") {
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

TEST_CASE("xml_select") {
    check_select("<?xml version=\"1.0\"?>"
                 "<root>"
                 " textroot"
                 " <a>texta0"
                 "  <b>textb0</b>"
                 "  <c>textc0</c>"
                 "  <d/>"
                 " </a>"
                 " <a>texta1"
                 "  <b>textb1</b>"
                 "  <c>textc1</c>"
                 "  <d/>"
                 " </a>"
                 " <a>texta2"
                 "  <b>textb2</b>"
                 "  <b>textb22</b>"
                 " </a>"
                 " <a/>"
                 " <aa/>"
                 "</root>",
                 "root/a",
                 {"", "b", "c", "d"},
                 {
                         {
                                 {"", "texta0       "},
                                 {"b", "textb0"},
                                 {"c", "textc0"},
                                 {"d", ""},
                         },
                         {
                                 {"", "texta1       "},
                                 {"b", "textb1"},
                                 {"c", "textc1"},
                                 {"d", ""},
                         },
                         {
                                 {"", "texta2     "},
                                 {"b", "textb2"},
                                 {"b", "textb22"},
                         },
                         {
                                 {"", ""},
                         },
                 });
}

}  // namespace komankondi::dictgen
