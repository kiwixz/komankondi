#pragma once

#include <chrono>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

#include <fmt/core.h>
#include <fmt/std.h>

#include "utils/file.hpp"
#include "utils/log.hpp"
#include "utils/path.hpp"

namespace komankondi::dictgen {

/// Returns true if we read data from cache instead of calling source
template <typename Source, typename Sink>
bool cache_data(std::string_view name, std::chrono::file_clock::time_point date,
                Source&& source, Sink&& sink) {
    static_assert(std::is_invocable_v<Source, void (*)(std::vector<std::byte>)>);
    static_assert(std::is_same_v<std::invoke_result_t<Source, void (*)(std::vector<std::byte>)>, void>);
    static_assert(std::is_invocable_v<Sink, std::vector<std::byte>>);
    static_assert(std::is_same_v<std::invoke_result_t<Sink, std::vector<std::byte>>, void>);

    std::filesystem::path path = get_cache_directory() / name;
    log::debug("cache path is {}", path);

    try {
        if (std::filesystem::exists(path)) {
            log::info("found cache");

            if (std::filesystem::last_write_time(path) >= date - std::chrono::seconds{1}) {
                File cache_file{path, File::Mode::read | File::Mode::binary};
                while (!cache_file.eof()) {
                    sink(cache_file.read<std::byte>());
                }
                return true;
            }

            log::info("cache is stale");
        }
    }
    catch (const std::exception& ex) {
        log::warn("could not look for cache: {}", ex.what());
    }

    std::string tmp_name = fmt::format("komankondi_{}", std::chrono::steady_clock::now().time_since_epoch().count());
    std::filesystem::path tmp_path = std::filesystem::temp_directory_path() / tmp_name;
    log::debug("temporary cache path is {}", tmp_path);

    {
        File tmp_file = File{tmp_path, File::Mode::write | File::Mode::binary};
        source([&](std::vector<std::byte>&& data) {
            tmp_file.write<std::byte>(data);
            sink(std::move(data));
        });
        tmp_file.sync();
    }

    std::filesystem::last_write_time(tmp_path, date);
    std::filesystem::create_directories(path.parent_path());
    std::filesystem::rename(tmp_path, path);

    log::info("successfully saved cached data");
    return false;
}

}  // namespace komankondi::dictgen
