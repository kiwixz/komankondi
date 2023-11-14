#include "wiktionary.hpp"

#include <atomic>
#include <chrono>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <httplib.h>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/range/conversion.hpp>

#include "dict/writer.hpp"
#include "dictgen/bzip.hpp"
#include "dictgen/cache.hpp"
#include "dictgen/options.hpp"
#include "dictgen/pipeline.hpp"
#include "dictgen/xml.hpp"
#include "utils/exception.hpp"
#include "utils/log.hpp"
#include "utils/parse.hpp"

namespace komankondi::dictgen {
namespace {

std::chrono::file_clock::time_point parse_dump_date(std::string_view str) {
    // only supports the "preferred" http format
    if (str.size() != 29
        || str.substr(3, 2) != ", "
        || str[7] != ' '
        || str[11] != ' '
        || str[16] != ' '
        || str[19] != ':' || str[22] != ':'
        || str.substr(25) != " GMT")
    {
        throw Exception{"could not parse http date from unsupported format: '{}'", str};
    }

    std::array months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    auto months_it = ranges::find(months, str.substr(8, 3));
    if (months_it == months.end())
        throw Exception{"unknown month: '{}'", str.substr(8, 3)};

    std::chrono::system_clock::time_point r = std::chrono::sys_days{std::chrono::year_month_day{
                                                      std::chrono::year{parse<int>(str.substr(12, 4))},
                                                      std::chrono::month{static_cast<unsigned>(months_it - months.begin()) + 1},
                                                      std::chrono::day{parse<unsigned>(str.substr(5, 2))}}}
                                              + std::chrono::hours{parse<int>(str.substr(17, 2))}
                                              + std::chrono::minutes{parse<int>(str.substr(20, 2))}
                                              + std::chrono::seconds{parse<int>(str.substr(23, 2))};

#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE < 13
    return std::chrono::file_clock::from_sys(r);
#else
    return std::chrono::clock_cast<std::chrono::file_clock>(r);
#endif
}

}  // namespace


void dictgen_wiktionary(std::string_view language, const Options& opt) {
    log::info("generating {} dictionary from wiktionary", language);

    httplib::SSLClient http{"dumps.wikimedia.org"};

    std::string dump_url = fmt::format("/{0}wiktionary/latest/{0}wiktionary-latest-pages-articles.xml.bz2", language);

    httplib::Result res = http.Head(dump_url);
    if (!res)
        throw Exception{"could not check latest wiktionary dump: {}", httplib::to_string(res.error())};
    if (res->status != 200)
        throw Exception{"could not check latest wiktionary dump: http status {} ({})", res->status, res->reason};

    std::string dump_date_str = res->get_header_value("last-modified");
    if (dump_date_str.empty())
        throw Exception{"could not get latest wiktionary dump date"};

    log::info("using latest dump from {}", dump_date_str);
    std::chrono::file_clock::time_point dump_date = parse_dump_date(dump_date_str);

    std::atomic<int> total_bytes = 0;
    int total_words = 0;
    std::chrono::steady_clock::time_point last_stat_time = std::chrono::steady_clock::now();
    int last_stat_bytes = 0;
    int last_stat_words = 0;

    BzipDecompressor unbzip;
    xml::Select dump_parser{"mediawiki/page", {"title", "ns", "revision/text"}};
    dict::Writer dict{opt.dictionary};

    Pipeline pipeline{
            make_pipe<std::vector<std::byte>>([&](std::vector<std::byte>&& data) {
                total_bytes += data.size();
                return std::move(data);
            }),
            make_pipe<std::vector<std::byte>>([&](std::span<const std::byte> data, auto&& sink) {
                std::vector<std::byte> r = unbzip(data);
                while (true) {
                    if (r.empty())
                        break;
                    sink(std::move(r));
                    r = unbzip();
                }
            }),
            make_pipe<std::vector<std::byte>>([&](std::span<const std::byte> data, auto&& sink) {
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
                ++total_words;
                dict.add_word(page.first, page.second);

                std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                if (now > last_stat_time + std::chrono::seconds{1}) {
                    int total_bytes_now = total_bytes.load();
                    double delta = std::chrono::duration<double>(now - last_stat_time).count();
                    log::info("{} KiB ({:.0f}/s) -> {} words ({:.0f}/s)",
                              total_bytes_now / 1024, (total_bytes_now - last_stat_bytes) / delta / 1024,
                              total_words, (total_words - last_stat_words) / delta);
                    last_stat_time = now;
                    last_stat_bytes = total_bytes_now;
                    last_stat_words = total_words;
                }
            }),
    };

    auto validate = [&] {
        pipeline.join();
        if (!unbzip.finished())
            throw Exception{"wiktionary data ends with an unfinished bzip stream"};
        if (!dump_parser.finished())
            throw Exception{"wiktionary data ends with an unfinished xml"};
    };

    auto source = [&](auto&& sink) {
        httplib::Result res = http.Get(
                dump_url,
                [&](const httplib::Response& res) {
                    if (res.status != 200)
                        throw Exception{"could not download wiktionary dump: http status {} ({})", res.status, res.reason};
                    return true;
                },
                [&](const char* ptr, size_t size) {
                    sink(std::as_bytes(std::span{ptr, size}) | ranges::to<std::vector>);
                    return true;
                });

        if (!res)
            throw Exception{"could not download wiktionary dump: {}", httplib::to_string(res.error())};

        validate();
    };

    if (opt.cache) {
        std::string cache_name = fmt::format("{}wiktionary.xml.bz2", language);
        if (cache_data(cache_name, dump_date, source, pipeline))
            validate();
    }
    else {
        source(pipeline);
    }
}

}  // namespace komankondi::dictgen
