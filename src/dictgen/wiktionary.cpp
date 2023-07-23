#include "wiktionary.hpp"

#include <filesystem>
#include <string>
#include <string_view>

#include <fmt/std.h>
#include <httplib.h>
#include <tao/pegtl/memory_input.hpp>

#include "dictgen/options.hpp"
#include "dictgen/xml.hpp"
#include "utils/exception.hpp"
#include "utils/log.hpp"
#include "utils/path.hpp"

namespace komankondi::dictgen {

void dictgen_wiktionary(std::string_view language, const Options& opt) {
    log::info("generating {} dictionary from wiktionary", language);

    httplib::SSLClient http{"dumps.wikimedia.org"};
    constexpr std::string_view metadata_url = "/{lang}wiktionary/latest/{lang}wiktionary-latest-pages-articles.xml.bz2-rss.xml";
    httplib::Result res = http.Get(fmt::format(metadata_url, fmt::arg("lang", language)));
    if (!res)
        throw Exception{"could not get wiktionary metadata: {}", httplib::to_string(res.error())};
    if (res->status != 200)
        throw Exception{"could not get wiktionary metadata: http status {} ({})", res->status, res->reason};
    log::trace("wiktionary metadata:\n{}", res->body);

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
    log::info("latest wiktionary version is {}", version);

    std::filesystem::path cache_path = get_cache_directory() / fmt::format("wiktionary_{}_{}.xml.bz2", language, version);
    log::debug("cache path is {}", cache_path);
    if (opt.cache && std::filesystem::exists(cache_path)) {
        log::info("found cached data");
    }
}

}  // namespace komankondi::dictgen
