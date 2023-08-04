#include "wiktionary.hpp"

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/std.h>
#include <httplib.h>
#include <range/v3/algorithm/ends_with.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/delimit.hpp>
#include <range/v3/view/split.hpp>
#include <tao/pegtl/memory_input.hpp>

#include "dict/writer.hpp"
#include "dictgen/bzip.hpp"
#include "dictgen/cache.hpp"
#include "dictgen/hasher.hpp"
#include "dictgen/options.hpp"
#include "dictgen/pipeline.hpp"
#include "dictgen/xml.hpp"
#include "utils/exception.hpp"
#include "utils/hex.hpp"
#include "utils/log.hpp"

namespace komankondi::dictgen {
namespace {

constexpr std::string_view dump_base_fmt = "/{lang}wiktionary/latest/{lang}wiktionary-latest";
constexpr std::string_view dump_file = "-pages-articles.xml.bz2";

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

}  // namespace


void dictgen_wiktionary(std::string_view language, const Options& opt) {
    log::info("generating {} dictionary from wiktionary", language);

    httplib::SSLClient http{"dumps.wikimedia.org"};
    std::string dump_base = fmt::format(dump_base_fmt, fmt::arg("lang", language));

    std::vector<std::byte> latest_sha1 = fetch_latest_sha1(dump_base, http);
    log::debug("latest data sha1 is {}", to_hex(latest_sha1));

    Hasher hasher{"sha1"};
    BzipDecompressor unbzip;
    xml::Select dump_parser{"mediawiki/page", {"title", "ns", "revision/text"}};
    dict::Writer dict{opt.dictionary};

    Pipeline pipeline{
            make_pipe<std::vector<std::byte>>([&](std::vector<std::byte>&& data) {
                hasher.update(data);
                return std::move(data);
            }),
            make_pipe<std::vector<std::byte>>([&](std::span<const std::byte> data, auto sink) {
                std::vector<std::byte> r = unbzip(data);
                if (!r.empty())
                    sink(std::move(r));
            }),
            make_pipe<std::vector<std::byte>>([&](std::span<const std::byte> data, auto sink) {
                dump_parser({reinterpret_cast<const char*>(data.data()), data.size()},
                            [&](xml::Select::Element&& el) {
                                if (el.count("title") != 1
                                    || el.count("ns") != 1
                                    || el.count("revision/text") != 1)
                                {
                                    log::warn("ignoring dump page with missing data");
                                    return;
                                }

                                if (el.find("ns")->second != "0")
                                    return;

                                sink({std::move(el.find("title")->second), std::move(el.find("revision/text")->second)});
                            });
            }),
            make_pipe<std::pair<std::string, std::string>>([&](std::pair<std::string, std::string>&& page) {
                dict.add_word(page.first, page.second);
            }),
    };

    auto source = [&](auto sink) {
        httplib::Result res = http.Get(
                fmt::format("{}{}", dump_base, dump_file),
                [&](const httplib::Response& res) {
                    if (res.status != 200)
                        throw Exception{"could not get wiktionary data: http status {} ({})", res.status, res.reason};
                    return true;
                },
                [&](const char* ptr, size_t size) {
                    sink(std::as_bytes(std::span{ptr, size}) | ranges::to<std::vector>);
                    return true;
                });

        if (!res)
            throw Exception{"could not get wiktionary data: {}", httplib::to_string(res.error())};

        // wait for pipeline ?

        if (!unbzip.finished())
            throw Exception{"data ends with an unfinished bzip stream"};

        std::vector<std::byte> sha1 = hasher.finish();
        log::debug("pipeline sha1 is {}", to_hex(sha1));
        if (sha1 != latest_sha1)
            throw Exception{"downloaded data does not match expected hash (expected {}, got {})", to_hex(latest_sha1), to_hex(sha1)};
    };

    if (opt.cache) {
        std::string cache_name = fmt::format("wiktionary_{}.xml.bz2", language);
        cache_data(cache_name, "sha1", latest_sha1, source, pipeline);
    }
    else {
        source(pipeline);
    }
}

}  // namespace komankondi::dictgen
