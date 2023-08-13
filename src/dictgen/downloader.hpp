#pragma once

#include <future>
#include <optional>
#include <string>
#include <vector>

#include "utils/consume_queue.hpp"

namespace komankondi::dictgen {

struct Downloader {
    Downloader(std::string host, std::string url);

    std::optional<std::vector<std::byte>> read();

private:
    ConsumeQueue<std::vector<std::byte>> queue_;
    std::future<void> future_;
};

}  // namespace komankondi::dictgen
