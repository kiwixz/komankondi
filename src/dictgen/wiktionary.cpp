#include "wiktionary.hpp"

#include <atomic>
#include <chrono>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <httplib.h>
#include <range/v3/algorithm/find.hpp>
#include <tbb/parallel_pipeline.h>

#include "dict/writer.hpp"
#include "dictgen/bzip.hpp"
#include "dictgen/cache.hpp"
#include "dictgen/downloader.hpp"
#include "dictgen/options.hpp"
#include "dictgen/xml.hpp"
#include "utils/config.hpp"
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
        throw Exception{"Could not parse http date from unsupported format: '{}'", str};
    }

    std::array months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    auto months_it = ranges::find(months, str.substr(8, 3));
    if (months_it == months.end())
        throw Exception{"Unknown month: '{}'", str.substr(8, 3)};

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
    log::info("Generating {} dictionary from wiktionary", language);

    std::string dump_host = "dumps.wikimedia.org";
    std::string dump_url = fmt::format("/{0}wiktionary/latest/{0}wiktionary-latest-pages-articles.xml.bz2", language);

    httplib::Result res = httplib::SSLClient{dump_host}.Head(dump_url);
    if (!res)
        throw Exception{"Could not check latest wiktionary dump: {}", httplib::to_string(res.error())};
    if (res->status != 200)
        throw Exception{"Could not check latest wiktionary dump: HTTP status {} ({})", res->status, res->reason};

    std::string dump_date_str = res->get_header_value("last-modified");
    if (dump_date_str.empty())
        throw Exception{"Could not get latest wiktionary dump date"};

    log::info("Using latest dump from {}", dump_date_str);
    std::chrono::file_clock::time_point dump_date = parse_dump_date(dump_date_str);

    std::optional<File> cached_file;
    std::optional<Downloader> downloader;
    std::optional<Cacher> cacher;
    std::function<std::optional<std::vector<std::byte>>()> fetch;
    if (opt.cache) {
        std::filesystem::path cache_path = get_cache_directory() / fmt::format("{}wiktionary.xml.bz2", language);
        cached_file = try_load_cache(cache_path, dump_date);
        if (cached_file) {
            fetch = [&cached_file]() -> std::optional<std::vector<std::byte>> {
                if (cached_file->eof())
                    return {};
                return cached_file->read();
            };
        }
        else {
            downloader.emplace(dump_host, dump_url);
            cacher.emplace(cache_path, dump_date);
            fetch = [&downloader, &cacher] {
                std::optional<std::vector<std::byte>> r = downloader->read();
                if (r) {
                    cacher->write<std::byte>(*r);
                }
                else {
                    cacher->save();
                }
                return r;
            };
        }
    }
    else {
        downloader.emplace(dump_host, dump_url);
        fetch = [&downloader] { return downloader->read(); };
    }


    std::atomic<int> total_bytes = 0;

    BzipDecompressor unbzip;
    xml::Select dump_parser{"mediawiki/page", {"title", "ns", "revision/text"}};
    dict::Writer dict{opt.dictionary};

    int total_words = 0;
    std::chrono::steady_clock::time_point last_stat_time = std::chrono::steady_clock::now();
    int last_stat_bytes = 0;
    int last_stat_words = 0;

    tbb::parallel_pipeline(default_parallel_queue_size(),
                           tbb::make_filter<void, std::vector<std::byte>>(
                                   tbb::filter_mode::serial_in_order,
                                   [&total_bytes, &fetch](tbb::flow_control& fc) {
                                       std::optional<std::vector<std::byte>> data = fetch();
                                       if (!data) {
                                           fc.stop();
                                           return std::vector<std::byte>{};
                                       }
                                       total_bytes.fetch_add(data->size(), std::memory_order::relaxed);
                                       return std::move(*data);
                                   })
                                   & tbb::make_filter<std::vector<std::byte>, std::vector<std::byte>>(
                                           tbb::filter_mode::serial_in_order,
                                           [&unbzip](const std::vector<std::byte>& data) {
                                               std::vector<std::byte> r = unbzip(data);
                                               while (true) {
                                                   std::vector<std::byte> more = unbzip();
                                                   if (more.empty())
                                                       break;
                                                   r.insert(r.end(), more.begin(), more.end());
                                               }
                                               return r;
                                           })
                                   & tbb::make_filter<std::vector<std::byte>, std::vector<std::pair<std::string, std::string>>>(
                                           tbb::filter_mode::serial_in_order,
                                           [&dump_parser](const std::vector<std::byte>& data) {
                                               std::vector<std::pair<std::string, std::string>> r;
                                               dump_parser({reinterpret_cast<const char*>(data.data()), data.size()},
                                                           [&](xml::Select::Element&& el) {
                                                               if (el.count("title") != 1
                                                                   || el.count("ns") != 1
                                                                   || el.count("revision/text") != 1)
                                                               {
                                                                   log::warn("Ignoring dump page with missing data");
                                                                   return;
                                                               }

                                                               if (el.find("ns")->second != "0")  // not a word
                                                                   return;

                                                               r.emplace_back(std::move(el.find("title")->second),
                                                                              std::move(el.find("revision/text")->second));
                                                           });
                                               return r;
                                           })
                                   & tbb::make_filter<std::vector<std::pair<std::string, std::string>>, void>(
                                           tbb::filter_mode::serial_out_of_order,
                                           [&total_words, &total_bytes, &last_stat_time, &last_stat_bytes, &last_stat_words, &dict](
                                                   const std::vector<std::pair<std::string, std::string>>& words) {
                                               for (const auto& [word, description] : words) {
                                                   ++total_words;
                                                   dict.add_word(word, description);

                                                   std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                                                   if (now > last_stat_time + std::chrono::seconds{2}) {
                                                       int total_bytes_now = total_bytes.load(std::memory_order::relaxed);
                                                       double delta = std::chrono::duration<double>(now - last_stat_time).count();
                                                       log::info("{} KiB ({:.0f}/s) -> {} words ({:.0f}/s)",
                                                                 total_bytes_now / 1024, (total_bytes_now - last_stat_bytes) / delta / 1024,
                                                                 total_words, (total_words - last_stat_words) / delta);
                                                       last_stat_time = now;
                                                       last_stat_bytes = total_bytes_now;
                                                       last_stat_words = total_words;
                                                   }
                                               }
                                           }));

    if (!unbzip.finished())
        throw Exception{"Wiktionary data ends with an unfinished bzip stream"};
    if (!dump_parser.finished())
        throw Exception{"Wiktionary data ends with an unfinished xml"};
}

}  // namespace komankondi::dictgen
