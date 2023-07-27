#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <range/v3/to_container.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/single.hpp>
#include <range/v3/view/transform.hpp>
#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/memory_input.hpp>
#include <tao/pegtl/parse.hpp>
#include <tao/pegtl/rules.hpp>

#include "utils/exception.hpp"

namespace komankondi::dictgen::xml {
namespace detail::using_pegtl {

using namespace tao::pegtl;
namespace grammar {

struct Declaration : seq<one<'?'>,
                         until<one<'>'>>> {};

struct TagNameChar : not_one<'/', '>', ' ', '\t', '\r', '\n'> {};
struct TagName : plus<TagNameChar> {};

struct StartTag : one<'>'> {};
struct EmptyElementTag : string<'/', '>'> {};
struct EndTag : seq<one<'/'>,
                    TagName,
                    until<one<'>'>>> {};

struct Tag : seq<one<'<'>,
                 sor<EndTag,
                     Declaration,
                     seq<TagName,
                         until<sor<StartTag,
                                   EmptyElementTag>>>>> {};

struct Text : seq<not_one<'<'>,
                  until<at<one<'<'>>>> {};

struct Standalone : sor<Text,
                        Tag> {};

struct Content : plus<Standalone> {};

}  // namespace grammar
}  // namespace detail::using_pegtl


struct XmlException : Exception {
    template <typename... Args>
    XmlException(const tao::pegtl::position& position, fmt::format_string<Args...> format, Args&&... args) :
            Exception{"could not parse xml at {}:{}, {}", position.line, position.column,
                      fmt::format(format, std::forward<Args>(args)...)} {
    }
};


namespace detail {

using namespace detail::using_pegtl::grammar;

struct IterateState {
    struct Element {
        std::string name;
        std::string text;

        Element(std::string&& name_);
    };

    std::vector<Element> stack;
    int consumed;

    std::string_view last_tag_name;
};


template <typename T>
struct Iterate : tao::pegtl::nothing<T> {};

template <>
struct Iterate<Text> {
    template <typename Input, typename Predicate>
    static void apply(const Input& in, IterateState& s, Predicate&& /*pred*/) {
        if (!s.stack.empty())  // ignore text outside of root element
            s.stack.back().text += in.string_view();
    }
};

template <>
struct Iterate<TagName> {
    template <typename Input, typename Predicate>
    static void apply(const Input& in, IterateState& s, Predicate&& /*pred*/) {
        s.last_tag_name = in.string_view();
    }
};

template <>
struct Iterate<StartTag> {
    template <typename Predicate>
    static void apply0(IterateState& s, Predicate&& /*pred*/) {
        s.stack.emplace_back(std::string{s.last_tag_name});
    }
};

template <>
struct Iterate<EmptyElementTag> {
    template <typename Predicate>
    static void apply0(IterateState& s, Predicate&& pred) {
        pred(ranges::views::concat(s.stack | ranges::views::transform([](const auto& el) -> std::string_view { return el.name; }),
                                   ranges::views::single(s.last_tag_name))
                     | ranges::views::join('/')
                     | ranges::to<std::string>,
             "");
    }
};

template <>
struct Iterate<EndTag> {
    template <typename Input, typename Predicate>
    static void apply(const Input& in, IterateState& s, Predicate&& pred) {
        if (s.stack.empty())
            throw XmlException{in.position(), "unexpected end tag"};
        if (s.last_tag_name != s.stack.back().name)
            throw XmlException{in.position(), "end tag does not match current element (expected '{}', got '{}')", s.stack.back().name, s.last_tag_name};
        pred(s.stack | ranges::views::transform([](const auto& el) -> std::string_view { return el.name; })
                     | ranges::views::join('/')
                     | ranges::to<std::string>,
             std::move(s.stack.back().text));
        s.stack.pop_back();
    }
};

template <>
struct Iterate<Standalone> {
    template <typename Input, typename Predicate>
    static void apply(const Input& in, IterateState& s, Predicate&& /*pred*/) {
        s.consumed = in.position().byte + in.size();
    }
};

}  // namespace detail


struct Iterate {
    template <typename Predicate>
    void operator()(std::string_view input, Predicate&& pred) {
        static_assert(std::is_invocable_v<Predicate, std::string&&, std::string&&>);
        static_assert(std::is_same_v<std::invoke_result_t<Predicate, std::string&&, std::string&&>, void>);

        if (!buffer_.empty()) {
            buffer_ += input;
            input = buffer_;
        }

        state_.consumed = 0;
        tao::pegtl::parse<detail::Content, detail::Iterate>(tao::pegtl::memory_input{input, ""},
                                                            state_,
                                                            std::forward<Predicate>(pred));

        if (buffer_.empty()) {
            buffer_ += input.substr(state_.consumed);
        }
        else {
            buffer_.erase(0, state_.consumed);
        }
    }

    bool finished() const;
    int buffer_size() const;

private:
    std::string buffer_;
    detail::IterateState state_;
};


template <typename Predicate>
void iterate(std::string_view input, Predicate&& pred) {
    Iterate parser;
    parser(input, std::forward<Predicate>(pred));
    if (!parser.finished())
        throw Exception{"could not fully parse xml input"};
}

}  // namespace komankondi::dictgen::xml
