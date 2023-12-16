#include "dictgen/downloader.hpp"

#include <future>
#include <optional>
#include <span>
#include <string>

#include <httplib.h>
#include <range/v3/range/conversion.hpp>

#include "utils/exception.hpp"
#include "utils/scope_exit.hpp"

namespace komankondi::dictgen {
namespace {

void download(std::string&& host, std::string&& url, ConsumeQueue<std::vector<std::byte>>& queue) {
    ScopeExit queue_closer{[&] { queue.close(); }};

    httplib::Result res = httplib::SSLClient{host}.Get(
            url,
            [&](const httplib::Response& res) {
                if (res.status != 200)
                    throw Exception{"Could not download file: HTTP status {} ({})", res.status, res.reason};
                return true;
            },
            [&](const char* ptr, size_t size) {
                return queue.push(std::as_bytes(std::span{ptr, size}) | ranges::to<std::vector>);
            });

    if (!res)
        throw Exception{"Could not download file: {}", httplib::to_string(res.error())};
}

}  // namespace


Downloader::Downloader(std::string host, std::string url) :
        future_{std::async(std::launch::async, download, std::move(host), std::move(url), std::ref(queue_))} {
}

Downloader::~Downloader() {
    queue_.close();
}

std::optional<std::vector<std::byte>> Downloader::read() {
    std::optional<std::vector<std::byte>> r = queue_.pop();
    if (!r && future_.valid())
        future_.get();
    return r;
}

}  // namespace komankondi::dictgen
