#include "dictgen/wiktionary.hpp"

#include <cassert>
#include <string_view>

#include <fmt/core.h>
#include <httplib.h>
#include <tao/pegtl.hpp>

#include "dictgen/xml.hpp"
#include "utils/exception.hpp"

namespace komankondi::dictgen {
namespace {

template <typename Rule = void>
struct metadata_find_link {
    struct state {
        std::string text;

        std::vector<std::string_view> tags_stack;
    };
};

template <>
struct metadata_find_link<xml::start_tag_name> {
    template <typename T>
    static void apply(const T& in, metadata_find_link<>::state& state) {
        state.tags_stack.push_back(in.string_view());
    }
};

template <>
struct metadata_find_link<xml::element_text> {
    template <typename T>
    static void apply(const T& in, metadata_find_link<>::state& state) {
        std::initializer_list<std::string_view> target{"rss", "channel", "link"};
        if (std::ranges::equal(state.tags_stack, target))
            state.text += in.string_view();
    }
};

template <>
struct metadata_find_link<xml::end_tag_name> {
    template <typename T>
    static void apply([[maybe_unused]] const T& in, metadata_find_link<>::state& state) {
        assert(!state.tags_stack.empty());
        assert(in.string_view() == state.tags_stack.back());
        state.tags_stack.pop_back();
    }
};

}  // namespace


void dictgen_wiktionary(std::string_view language) {
    httplib::SSLClient http{"dumps.wikimedia.org"};
    constexpr std::string_view metadata_url = "/{language}wiktionary/latest/{language}wiktionary-latest-pages-articles.xml.bz2-rss.xml";
    httplib::Result res = http.Get(fmt::format(metadata_url, fmt::arg("language", language)));
    if (!res)
        throw Exception{"could not get wiktionary metadata: {}", httplib::to_string(res.error())};
    if (res->status != 200)
        throw Exception{"could not get wiktionary metadata: http status {} ({})", res->status, res->reason};

    metadata_find_link<>::state state;
    if (!tao::pegtl::parse<xml::document, metadata_find_link>(tao::pegtl::memory_input{res->body, ""}, state))
        throw Exception{"could not parse wiktionary metadata"};

    fmt::println("'{}'", state.text);
}

}  // namespace komankondi::dictgen
