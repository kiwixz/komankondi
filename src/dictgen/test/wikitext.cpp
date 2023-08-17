#include <catch2/catch_test_macros.hpp>
#include <tao/pegtl/contrib/analyze.hpp>

#include "dictgen/wikitext/extract.hpp"
#include "dictgen/wikitext/grammar.hpp"

namespace komankondi::dictgen::wikitext {

TEST_CASE("wikitext_grammar") {
    CHECK(tao::pegtl::analyze<grammar::Article>() == 0);
}

TEST_CASE("wikitext_simple") {
    extract_definitions(R"(
== {{langue|fr}} ==
=== {{S|nom|fr}} ===
# 1
# 2
# 3
    )");
    extract_definitions(R"(
== {{langue|fr}} ==
=== {{S|nom|fr}} ===
# [[link|1]]
    )");
    extract_definitions(R"(
== {{langue|fr}} ==
=== {{S|nom|fr}} ===
# {{2|template}}
    )");
}

}  // namespace komankondi::dictgen::wikitext
