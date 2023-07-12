#include "dictgen/xml.hpp"

#include <catch2/catch_test_macros.hpp>
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>

namespace komankondi::dictgen {
namespace {

std::unique_ptr<tao::pegtl::parse_tree::node> parse(std::string_view document) {
    tao::pegtl::memory_input input{document, ""};
    std::unique_ptr<tao::pegtl::parse_tree::node> root = tao::pegtl::parse_tree::parse<xml::document>(input);
    REQUIRE(root);
    REQUIRE(root->children.size() == 1);
    REQUIRE(root->children[0]->is_type<xml::document>());
    return std::move(root->children[0]);
}

}  // namespace


TEST_CASE("analyze") {
    CHECK(tao::pegtl::analyze<xml::document>() == 0);
}

TEST_CASE("empty") {
    parse("");
}

TEST_CASE("one") {
    std::unique_ptr<tao::pegtl::parse_tree::node> doc = parse("<tag></tag>");
    REQUIRE(doc->children.size() == 3);

    tao::pegtl::parse_tree::node& opt_decl = *doc->children[0];
    CHECK(opt_decl.is_type<tao::pegtl::opt<xml::declaration>>());
    CHECK(opt_decl.children.empty());

    tao::pegtl::parse_tree::node& el_content = *doc->children[1];
    CHECK(el_content.is_type<xml::element_content>());

    tao::pegtl::parse_tree::node& eof = *doc->children[2];
    CHECK(eof.is_type<tao::pegtl::eof>());
}

TEST_CASE("declaration") {
    std::unique_ptr<tao::pegtl::parse_tree::node> doc = parse(R"(
        <?xml version="1.0"?>
        <tag></tag>
    )");

    tao::pegtl::parse_tree::node& opt_decl = *doc->children[0];
    CHECK(opt_decl.is_type<tao::pegtl::opt<xml::declaration>>());
    CHECK(opt_decl.children.size() == 1);

    tao::pegtl::parse_tree::node& decl = *opt_decl.children[0];
    CHECK(decl.is_type<xml::declaration>());

    tao::pegtl::parse_tree::node& el_content = *doc->children[1];
    CHECK(el_content.is_type<xml::element_content>());

    tao::pegtl::parse_tree::node& eof = *doc->children[2];
    CHECK(eof.is_type<tao::pegtl::eof>());
}

}  // namespace komankondi::dictgen
