#pragma once

#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/rules.hpp>

#include "utils/static_string.hpp"

namespace komankondi::dictgen::wikitext::grammar {
namespace _ {
using namespace tao::pegtl;

template <StaticString str>
struct String : decltype([]<size_t... i>(std::index_sequence<i...>) {
    return string<str[i]...>{};
}(std::make_index_sequence<str.size>{})) {};

template <StaticString begin, typename T, StaticString end = begin>
struct Scope : seq<String<begin>, must<T, String<end>>> {};

template <typename... T>
struct PadSpace : pad<seq<T...>, one<' '>> {};


namespace inner {

struct TemplateName : plus<not_one<'}', '|'>> {};
struct TemplateParameter : plus<not_one<'}', '|'>> {};
struct Template : seq<Scope<"{{",
                            seq<TemplateName,
                                star<one<'|'>,
                                     TemplateParameter>>,
                            "}}">> {};

struct LinkTarget : plus<not_one<']', '|'>> {};
struct LinkText : plus<not_one<']', '|'>> {};
struct Link : seq<Scope<"[[",
                        sor<seq<LinkTarget,
                                one<'|'>,
                                LinkText>,
                            LinkText>,  // emit LinkText with text from LinkTarget
                        "]]">> {};

struct Text : sor<Link,
                  Template,
                  any> {};

struct ListOrdered : until<at<one<'\n'>>,
                           Text> {};

template <int depth>
struct Heading : until<at<star<one<' '>>,
                          rep<depth,
                              one<'='>>>> {};

template <int depth>
struct HeadingAnyImpl : sor<Scope<"=",
                                  HeadingAnyImpl<depth + 1>>,
                            PadSpace<Heading<depth>>> {};
template <>
struct HeadingAnyImpl<6> : seq<not_at<one<'='>>,
                               PadSpace<Heading<6>>> {};
struct HeadingAny : seq<Scope<"==",
                              HeadingAnyImpl<2>>> {};

struct Article : plus<sor<HeadingAny,
                          seq<String<"# ">,
                              ListOrdered>,
                          until<one<'\n'>>>> {};

}  // namespace inner
}  // namespace _


using namespace _::inner;

}  // namespace komankondi::dictgen::wikitext::grammar
