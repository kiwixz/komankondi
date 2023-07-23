#include "wiktionary.hpp"

#include <filesystem>
#include <string>
#include <string_view>

#include <fmt/std.h>
#include <httplib.h>
#include <range/v3/algorithm/ends_with.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/view/delimit.hpp>
#include <range/v3/view/split.hpp>
#include <tao/pegtl/memory_input.hpp>

#include "dictgen/digest.hpp"
#include "dictgen/options.hpp"
#include "dictgen/xml.hpp"
#include "utils/exception.hpp"
#include "utils/hex.hpp"
#include "utils/log.hpp"
#include "utils/path.hpp"

namespace komankondi::dictgen {
namespace {

constexpr std::string_view dump_base_fmt = "/{lang}wiktionary/latest/{lang}wiktionary-latest";
// constexpr std::string_view dump_file = "-pages-articles.xml.bz2";
constexpr std::string_view dump_file = "-flow.xml.bz2";

std::vector<std::byte> fetch_latest_sha1(std::string_view dump_base, httplib::SSLClient& http) {
    httplib::Result res = http.Get(fmt::format("{}-sha1sums.txt", dump_base));
    if (!res)
        throw Exception{"could not get wiktionary sha1sums: {}", httplib::to_string(res.error())};
    if (res->status != 200)
        throw Exception{"could not get wiktionary sha1sums: http status {} ({})", res->status, res->reason};
    log::trace("wiktionary sha1sums:\n{}", res->body);

    auto sha1sums = res->body | ranges::views::split('\n');
    auto sha1sums_it = ranges::find_if(sha1sums, [](auto line) {
        return ranges::ends_with(line, dump_file);
    });
    if (sha1sums_it == sha1sums.end())
        throw Exception{"could not find the data in the lastest dump"};
    auto hex = *sha1sums_it | ranges::views::delimit(' ');
    return from_hex(std::string_view{&*hex.begin(), static_cast<size_t>(ranges::distance(hex))});
}

void process(std::span<const std::byte> data) {
    (void)data;
}

}  // namespace


void dictgen_wiktionary(std::string_view language, const Options& opt) {
    log::info("generating {} dictionary from wiktionary", language);

    std::string dump_base = fmt::format(dump_base_fmt, fmt::arg("lang", language));

    httplib::SSLClient http{"dumps.wikimedia.org"};

    std::vector<std::byte> latest_sha1 = fetch_latest_sha1(dump_base, http);
    log::dev("latest data should have the following sha1: {}", to_hex(latest_sha1));

    Digest file_sha1{"sha1"};

    std::filesystem::path cache_path = get_cache_directory() / fmt::format("wiktionary_{}.xml.bz2", language);
    log::debug("cache path is {}", cache_path);
    if (opt.cache && std::filesystem::exists(cache_path)) {
        log::info("found cached data");
    }

    httplib::Result res = http.Get(fmt::format("{}{}", dump_base, dump_file), [&](const char* ptr, size_t size) {
        std::span<const std::byte> data = std::as_bytes(std::span{ptr, size});
        file_sha1.update(data);
        process(data);
        return true;
    });

    log::dev("file hash {}", to_hex(file_sha1.finish()));
}

}  // namespace komankondi::dictgen
