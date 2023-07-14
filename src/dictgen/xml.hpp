#pragma once

#include <range/v3/to_container.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>
#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/parse.hpp>
#include <tao/pegtl/rules.hpp>

namespace komankondi::dictgen::xml {
namespace detail {

using namespace tao::pegtl;
using tao::pegtl::eof;

template <typename... T>
struct trim : seq<star<space>, T..., star<space>> {};
template <typename... T>
struct triml : seq<star<space>, T...> {};
template <typename... T>
struct trimr : seq<T..., star<space>> {};

namespace grammar {

struct Element;

struct TagName : until<at<sor<one<'/', '>'>, space>>> {};
struct StartTagName : TagName {};
struct StartTag : seq<one<'<'>,
                      not_at<one<'/'>>,
                      StartTagName,
                      until<one<'>'>>> {};
struct EndTagName : TagName {};
struct EndTag : seq<string<'<', '/'>,
                    EndTagName,
                    until<one<'>'>>> {};

struct ElementText : seq<not_one<'<'>,
                         until<at<seq<star<space>,
                                      one<'<'>>>>> {};
struct ElementContent : trimr<star<triml<sor<Element,
                                             ElementText>>>> {};
struct Element : seq<StartTag,
                     ElementContent,
                     EndTag> {};

struct Declaration : seq<string<'<', '?'>,
                         until<one<'>'>>> {};

struct Document : seq<trim<opt<trimr<Declaration>>,
                           Element>,
                      eof> {};

}  // namespace grammar
}  // namespace detail

using namespace detail::grammar;


namespace actions {
namespace detail {

template <typename Predicate>
struct IterateState {
    struct Element {
        std::string_view name;
        std::string text;

        Element(std::string_view _name) :
                name{_name} {
        }
    };

    std::vector<Element> stack;
    Predicate predicate;

    IterateState(Predicate&& _predicate) :
            predicate{std::forward<Predicate>(_predicate)} {
    }
};


template <typename T>
struct Iterate : tao::pegtl::nothing<T> {};

template <>
struct Iterate<StartTagName> {
    template <typename Input, typename State>
    static void apply(const Input& in, State& s) {
        s.stack.emplace_back(in.string_view());
    }
};

template <>
struct Iterate<ElementText> {
    template <typename Input, typename State>
    static void apply(const Input& in, State& s) {
        assert(!s.stack.empty());
        s.stack.back().text += in.string_view();
    }
};

template <>
struct Iterate<EndTagName> {
    template <typename Input, typename State>
    static void apply([[maybe_unused]] const Input& in, State& s) {
        assert(!s.stack.empty());
        assert(in.string_view() == s.stack.back().name);
        s.predicate(s.stack | ranges::views::transform([](const auto& el) { return el.name; })
                            | ranges::views::join('/')
                            | ranges::to<std::string>,
                    std::move(s.stack.back().text));
        s.stack.pop_back();
    }
};

}  // namespace detail


template <typename Input, typename Predicate>
requires std::is_invocable_v<Predicate, std::string&& /*path*/, std::string&& /*text*/>
void iterate(Input&& in, Predicate&& pred) {
    tao::pegtl::parse<tao::pegtl::must<Document>, detail::Iterate>(std::forward<Input>(in),
                                                                   detail::IterateState{std::forward<Predicate>(pred)});
}

}  // namespace actions
}  // namespace komankondi::dictgen::xml
