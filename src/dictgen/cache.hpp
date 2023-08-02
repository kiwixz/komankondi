#pragma once

#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

#include <fmt/core.h>
#include <fmt/std.h>
#include <range/v3/algorithm/equal.hpp>

#include "utils/file.hpp"
#include "utils/hex.hpp"
#include "utils/log.hpp"
#include "utils/path.hpp"

namespace komankondi::dictgen {

template <typename Source, typename Sink>
void cache_data(std::string_view name,
                std::string_view hash_algorithm, std::span<const std::byte> latest_hash,
                Source&& source, Sink&& sink) {
    static_assert(std::is_invocable_v<Source, void (*)(std::vector<std::byte>)>);
    static_assert(std::is_same_v<std::invoke_result_t<Source, void (*)(std::vector<std::byte>)>, void>);
    static_assert(std::is_invocable_v<Sink, std::vector<std::byte>>);
    static_assert(std::is_same_v<std::invoke_result_t<Sink, std::vector<std::byte>>, void>);

    std::filesystem::path path = get_cache_directory() / name;
    std::filesystem::path hash_path = get_cache_directory() / fmt::format("{}.{}", name, hash_algorithm);
    log::debug("cache path is {}", path);

    if (std::filesystem::exists(path)) {
        log::info("found cache");

        try {
            std::vector<std::byte> cache_hash = File{hash_path, File::Mode::read | File::Mode::binary}.read<std::byte>(latest_hash.size() + 1);
            log::debug("cache hash is {}", to_hex(cache_hash));
            if (ranges::equal(cache_hash, latest_hash)) {
                File cache_file{path, File::Mode::read | File::Mode::binary};
                while (!cache_file.eof()) {
                    sink(cache_file.read<std::byte>());
                }
                return;
            }

            log::info("cache is stale");
        }
        catch (const std::exception& ex) {
            log::warn("found cache but could not use it: {}", ex.what());
        }
    }

    std::string tmp_name = fmt::format("komankondi_{}", std::chrono::steady_clock::now().time_since_epoch().count());
    std::filesystem::path tmp_path = std::filesystem::temp_directory_path() / tmp_name;
    log::debug("temporary cache path is {}", tmp_path);

    {
        File tmp_file = File{tmp_path, File::Mode::truncate | File::Mode::binary};
        source([&](std::vector<std::byte>&& data) {
            tmp_file.write<std::byte>(data);
            sink(std::move(data));
        });
    }

    std::filesystem::create_directories(path.parent_path());
    std::filesystem::rename(tmp_path, path);

    File hash_file{hash_path, File::Mode::truncate | File::Mode::binary};
    hash_file.write(latest_hash);

    log::info("successfully saved cached data");
}

}  // namespace komankondi::dictgen
