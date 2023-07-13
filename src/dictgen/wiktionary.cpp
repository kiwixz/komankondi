#include "dictgen/wiktionary.hpp"

#include <cassert>
#include <string_view>

#include <fmt/core.h>
#include <httplib.h>

#include "dictgen/xml.hpp"
#include "utils/exception.hpp"

namespace komankondi::dictgen {

void dictgen_wiktionary(std::string_view language) {
    httplib::SSLClient http{"dumps.wikimedia.org"};
    constexpr std::string_view metadata_url = "/{language}wiktionary/latest/{language}wiktionary-latest-pages-articles.xml.bz2-rss.xml";
    httplib::Result res = http.Get(fmt::format(metadata_url, fmt::arg("language", language)));
    if (!res)
        throw Exception{"could not get wiktionary metadata: {}", httplib::to_string(res.error())};
    if (res->status != 200)
        throw Exception{"could not get wiktionary metadata: http status {} ({})", res->status, res->reason};

    std::string link;
    xml::actions::iterate(tao::pegtl::memory_input{res->body, ""},
                          [&](std::string&& path, std::string&& text) {
                              if (path == "rss/channel/link")
                                  link = text;
                          });

    size_t last_slash = link.find_last_of('/');
    if (last_slash == std::string::npos)
        throw Exception{"unexpected wiktionary dump link: '{}'", link};

    std::string version = link.substr(last_slash + 1);
    fmt::println("latest version is {}", version);
}

}  // namespace komankondi::dictgen
