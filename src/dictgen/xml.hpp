#pragma once

#include <tao/pegtl.hpp>

namespace komankondi::dictgen::xml {

namespace pegtl = tao::pegtl;

struct element;

struct ws : pegtl::one<' ', '\t', '\n', '\r'> {};
struct lt : pegtl::one<'<'> {};
struct gt : pegtl::one<'>'> {};
struct div : pegtl::one<'/'> {};

struct tag_name : pegtl::until<pegtl::at<pegtl::sor<gt, ws>>> {};
struct start_tag_name : tag_name {};
struct start_tag : pegtl::seq<lt, pegtl::not_at<div>, start_tag_name, pegtl::until<gt>> {};
struct end_tag_name : tag_name {};
struct end_tag : pegtl::seq<pegtl::seq<lt, div>, end_tag_name, pegtl::until<gt>> {};

struct element_text : pegtl::seq<pegtl::not_one<'<'>, pegtl::until<pegtl::at<lt>>> {};
struct element_content : pegtl::star<pegtl::sor<element, element_text>> {};
struct element : pegtl::pad<pegtl::seq<start_tag, element_content, end_tag>, ws> {};

struct declaration : pegtl::pad<pegtl::seq<lt, pegtl::one<'?'>, pegtl::until<gt>>, ws> {};

struct document : pegtl::seq<pegtl::opt<declaration>, element_content, pegtl::eof> {};

}  // namespace komankondi::dictgen::xml
