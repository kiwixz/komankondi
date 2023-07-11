#include "dictgen/wiktionary.hpp"

#include <string_view>

#include <fmt/core.h>
#include <httplib.h>

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

    fmt::println("{}", res->body);
}

}  // namespace komankondi::dictgen
