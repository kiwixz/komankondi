#include "dictgen/wikitext/extract.hpp"

#include <string>
#include <string_view>
#include <vector>

#include <tao/pegtl/change_action_and_states.hpp>
#include <tao/pegtl/memory_input.hpp>
#include <tao/pegtl/parse.hpp>

#include "dict/word.hpp"
#include "dictgen/wikitext/grammar.hpp"
#include "utils/exception.hpp"
#include "utils/log.hpp"

namespace komankondi::dictgen::wikitext {
namespace {

struct ParsingException : Exception {
    template <typename... Args>
    ParsingException(const tao::pegtl::position& position, fmt::format_string<Args...> format, Args&&... args) :
            Exception{"Could not parse wikitext at {}:{}, {}", position.line, position.column,
                      fmt::format(format, std::forward<Args>(args)...)} {
    }
};


struct UnwikitextifyAppend {
    template <typename Input>
    static void apply(const Input& in, std::string& s) {
        s += in.string_view();
    }
};

template <typename T>
struct Unwikitextify : tao::pegtl::nothing<T> {};

template <>
struct Unwikitextify<tao::pegtl::any> : UnwikitextifyAppend {};

template <>
struct Unwikitextify<grammar::LinkText> : UnwikitextifyAppend {};

template <>
struct Unwikitextify<grammar::TemplateName> {
    template <typename Input>
    static void apply(const Input& in, std::string& s) {
        s += '(';
        s += in.string_view();
        s += ')';
    }
};


struct ExtractState {
    std::vector<std::string> word;

    int section_depth;
    std::string section_name;
};

template <typename T>
struct Extract : tao::pegtl::nothing<T> {};

template <int depth>
struct Extract<grammar::Heading<depth>> {
    template <typename Input>
    static void apply(const Input& in, ExtractState& s) {
        log::dev("h{} {}", depth, in.string_view());
        s.section_depth = depth;
        s.section_name = in.string();
    }
};

template <>
struct Extract<grammar::ListOrdered> : tao::pegtl::change_action_and_states<Unwikitextify, std::string> {
    template <typename Input>
    static void success(const Input& /*in*/, std::string& text, ExtractState& s) {
        log::dev("ol {}", text);
    }
};

template <>
struct Extract<grammar::Link> {
    template <typename Input>
    static void apply(const Input& in, ExtractState& s) {
        log::dev("l {}", in.string_view());
    }
};

template <>
struct Extract<grammar::Template> {
    template <typename Input>
    static void apply(const Input& in, ExtractState& s) {
        log::dev("t {}", in.string_view());
    }
};

}  // namespace


std::vector<std::string> extract_definitions(std::string_view input) {
    log::info("{}", input);

    ExtractState state;
    tao::pegtl::parse<tao::pegtl::must<grammar::Article>, Extract>(tao::pegtl::memory_input{input, ""},
                                                                        state);
    return state.word;
}

}  // namespace komankondi::dictgen::wikitext
