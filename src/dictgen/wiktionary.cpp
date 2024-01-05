#include "wiktionary.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#include <boost/regex.hpp>
#include <fmt/core.h>
#include <httplib.h>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/transform.hpp>
#include <tbb/parallel_pipeline.h>

#include "dict/writer.hpp"
#include "dictgen/cache.hpp"
#include "dictgen/downloader.hpp"
#include "dictgen/gzip.hpp"
#include "dictgen/options.hpp"
#include "dictgen/tarcat.hpp"
#include "utils/config.hpp"
#include "utils/exception.hpp"
#include "utils/find_last.hpp"
#include "utils/log.hpp"
#include "utils/signal.hpp"

namespace komankondi::dictgen {

void dictgen_wiktionary(std::string_view language, const Options& opt) {
    log::info("Generating {} dictionary from wiktionary", language);

    std::string host = "dumps.wikimedia.org";
    httplib::SSLClient http{host};

    httplib::Result index_res = http.Get("/other/enterprise_html/runs/");
    if (!index_res)
        throw Exception{"Could not get dumps index: {}", httplib::to_string(index_res.error())};
    if (index_res->status != 200)
        throw Exception{"Could not get dumps index: HTTP status {} ({})", index_res->status, index_res->reason};

    boost::regex re_dump_dates{R"("([0-9]{8})/")"};

    ranges::subrange dump_dates{boost::sregex_token_iterator{index_res->body.begin(), index_res->body.end(),
                                                             re_dump_dates, 1},
                                boost::sregex_token_iterator{}};
    if (dump_dates.empty())
        throw Exception{"Could not find any available dump"};

    std::string dump_date = ranges::max(dump_dates);
    log::info("Using latest dump from {}", dump_date);

    std::string dump_url = fmt::format("/other/enterprise_html/runs/20240101/{}wiktionary-NS0-{}-ENTERPRISE-HTML.json.tar.gz", language, dump_date);

    std::optional<File> cached_file;
    std::optional<Downloader> downloader;
    std::optional<Cacher> cacher;
    std::function<std::optional<std::vector<std::byte>>()> fetch;
    if (opt.cache) {
        std::filesystem::path cache_path = get_cache_directory() / fmt::format("{}wiktionary_{}.tgz", language, dump_date);
        cached_file = try_load_cache(cache_path);
        if (cached_file) {
            fetch = [&cached_file]() -> std::optional<std::vector<std::byte>> {
                if (cached_file->eof())
                    return {};
                return cached_file->read();
            };
        }
        else {
            downloader.emplace(host, dump_url);
            cacher.emplace(cache_path);
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
        downloader.emplace(host, dump_url);
        fetch = [&downloader] { return downloader->read(); };
    }


    std::atomic<size_t> total_bytes = 0;

    GzipDecompressor unzip;
    TarCat tarcat;
    std::vector<std::byte> partial_line;
    dict::Writer dict{opt.dictionary};

    size_t total_words = 0;
    std::chrono::steady_clock::time_point last_stat_time = std::chrono::steady_clock::now();
    size_t last_stat_bytes = 0;
    size_t last_stat_words = 0;

    tbb::parallel_pipeline(default_parallel_queue_size(),
                           tbb::make_filter<void, std::vector<std::byte>>(
                                   tbb::filter_mode::serial_in_order,
                                   [&total_bytes, &fetch](tbb::flow_control& fc) {
                                       std::optional<std::vector<std::byte>> data;
                                       if (!terminating())
                                           data = fetch();
                                       if (!data) {
                                           fc.stop();
                                           return std::vector<std::byte>{};
                                       }
                                       total_bytes.fetch_add(data->size(), std::memory_order::relaxed);
                                       return std::move(*data);
                                   })
                                   & tbb::make_filter<std::vector<std::byte>, std::vector<std::byte>>(
                                           tbb::filter_mode::serial_in_order,
                                           [&unzip](std::vector<std::byte>&& data) {
                                               std::vector<std::byte> r = unzip(data);
                                               while (true) {
                                                   std::vector<std::byte> more = unzip();
                                                   if (more.empty())
                                                       break;
                                                   r.insert(r.end(), more.begin(), more.end());
                                               }
                                               return r;
                                           })
                                   & tbb::make_filter<std::vector<std::byte>, std::vector<std::byte>>(
                                           tbb::filter_mode::serial_in_order,
                                           [&tarcat, &partial_line](std::vector<std::byte>&& data) {
                                               std::vector<std::byte> r = std::move(partial_line);
                                               tarcat(data, r);
                                               auto it = find_last(r, std::byte{'\n'});
                                               if (it == r.end()) {
                                                   partial_line = std::move(r);
                                                   return std::vector<std::byte>{};
                                               }
                                               partial_line.assign(it + 1, r.end());
                                               r.erase(it + 1, r.end());
                                               return r;
                                           })
                                   & tbb::make_filter<std::vector<std::byte>, std::vector<std::pair<std::string, std::string>>>(
                                           tbb::filter_mode::parallel,
                                           [](std::vector<std::byte>&& data) {
                                               std::vector<std::pair<std::string, std::string>> r;
                                               std::string_view remaining{reinterpret_cast<char*>(data.data()), data.size()};
                                               while (!remaining.empty()) {
                                                   int size = remaining.find('\n');
                                                   std::string_view line = remaining.substr(0, size);
                                                   remaining = remaining.substr(size + 1);

                                                   boost::json::value json = boost::json::parse(line);
                                                   std::string_view word = json.at("name").as_string();
                                                   std::string_view html = json.at("article_body").at("html").as_string();

                                                   r.emplace_back(word, html);
                                               }
                                               return r;
                                           })
                                   & tbb::make_filter<std::vector<std::pair<std::string, std::string>>, void>(
                                           tbb::filter_mode::serial_out_of_order,
                                           [&total_words, &total_bytes, &last_stat_time, &last_stat_bytes, &last_stat_words, &dict](
                                                   const std::vector<std::pair<std::string, std::string>>& words) {
                                               for (const auto& [word, description] : words) {
                                                   ++total_words;

                                                   try {
                                                       dict.add_word(word, description);
                                                   }
                                                   catch (const std::exception& ex) {
                                                       // dumps currently have duplicates: https://phabricator.wikimedia.org/T305407
                                                       log::debug("Could not add word {}: {}", word, ex.what());
                                                   }

                                                   std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                                                   if (now > last_stat_time + std::chrono::seconds{2}) {
                                                       size_t total_bytes_now = total_bytes.load(std::memory_order::relaxed);
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
    if (terminating())
        return;

    if (!unzip.finished())
        throw Exception{"Data ends with an unfinished gzip stream"};
    if (!tarcat.finished())
        throw Exception{"Data ends with an unfinished tar file"};
    if (!partial_line.empty())
        throw Exception{"Data ends with a partial line"};

    dict.save();
    log::info("Successfully saved new dictionary with {} words", total_words);
}

}  // namespace komankondi::dictgen
