#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <range/v3/algorithm/contains.hpp>
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

struct ParsingException : Exception {
    template <typename... Args>
    ParsingException(const tao::pegtl::position& position, fmt::format_string<Args...> format, Args&&... args) :
            Exception{"Could not parse xml at {}:{}, {}", position.line, position.column,
                      fmt::format(format, std::forward<Args>(args)...)} {
    }
};

namespace detail {

namespace using_pegtl {
using namespace tao::pegtl;
namespace _ {

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

}  // namespace _
}  // namespace using_pegtl
using namespace detail::using_pegtl::_;


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
            throw ParsingException{in.position(), "Unexpected end tag"};
        if (s.last_tag_name != s.stack.back().name)
            throw ParsingException{in.position(), "End tag does not match current element (expected '{}', got '{}')", s.stack.back().name, s.last_tag_name};
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
        static_assert(std::is_invocable_v<Predicate, std::string&& /*path*/, std::string&& /*text*/>);
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

private:
    std::string buffer_;
    detail::IterateState state_;
};

template <typename Predicate>
void iterate(std::string_view input, Predicate&& pred) {
    Iterate parser;
    parser(input, std::forward<Predicate>(pred));
    if (!parser.finished())
        throw Exception{"Could not fully parse XML input"};
}


struct Select {
    using Element = std::unordered_multimap<std::string /*subpath*/, std::string /*text*/>;

    Select(std::string&& path, std::vector<std::string>&& subpaths);

    template <typename Predicate>
    void operator()(std::string_view input, Predicate&& pred) {
        static_assert(std::is_invocable_v<Predicate, Element&&>);
        static_assert(std::is_same_v<std::invoke_result_t<Predicate, Element&&>, void>);

        parser_(input, [&](std::string&& path, std::string&& text) {
            if (!path.starts_with(select_path_))
                return;

            if (path.size() == select_path_.size()) {
                if (ranges::contains(select_subpaths_, ""))
                    current_.emplace("", std::move(text));
                pred(std::move(current_));
                current_ = {};
                return;
            }

            if (path[select_path_.size()] != '/')
                return;

            std::string_view subpath = std::string_view{path}.substr(select_path_.size() + 1);
            if (!ranges::contains(select_subpaths_, subpath))
                return;

            current_.emplace(subpath, std::move(text));
        });
    }

    bool finished() const;

private:
    std::string select_path_;
    std::vector<std::string> select_subpaths_;

    Iterate parser_;
    Element current_;
};

template <typename Predicate>
void select(std::string_view input, std::string&& path, std::vector<std::string>&& subpaths, Predicate&& pred) {
    Select parser{std::move(path), std::move(subpaths)};
    parser(input, std::forward<Predicate>(pred));
    if (!parser.finished())
        throw Exception{"Could not fully parse XML input"};
}

}  // namespace komankondi::dictgen::xml
