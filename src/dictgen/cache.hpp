#pragma once

#include <filesystem>
#include <fstream>
#include <span>
#include <string_view>
#include <vector>

#include <fmt/core.h>
#include <fmt/std.h>

#include "utils/log.hpp"
#include "utils/path.hpp"

namespace komankondi::dictgen {
namespace detail_cache {

std::optional<std::ifstream> open_cache(const std::filesystem::path& path,
                                        const std::filesystem::path& hash_path, std::span<const std::byte> latest_hash);
void save_cache(const std::filesystem::path& path,
                const std::filesystem::path& hash_path, std::span<const std::byte> latest_hash,
                const std::filesystem::path& tmp_path);

}  // namespace detail_cache


template <typename Source, typename Sink>
void cache_data(std::string_view name,
                std::string_view hash_algorithm, std::span<const std::byte> latest_hash,
                Source&& source, Sink&& sink) {
    static_assert(std::is_invocable_r_v<void, Source, void (*)(std::vector<std::byte>)>);
    static_assert(std::is_invocable_r_v<void, Sink, std::vector<std::byte>>);

    std::filesystem::path path = get_cache_directory() / name;
    std::filesystem::path hash_path = get_cache_directory() / fmt::format("{}.{}", name, hash_algorithm);
    log::debug("cache path is {}", path);

    if (std::optional<std::ifstream> file = detail_cache::open_cache(path, hash_path, latest_hash); file) {
        while (*file) {
            std::vector<std::byte> buffer;
            buffer.resize(2000000);
            file->read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            buffer.resize(file->gcount());
            sink(std::move(buffer));
        }
        return;
    }

    std::string tmp_name = fmt::format("komankondi_{}", std::chrono::steady_clock::now().time_since_epoch().count());
    std::filesystem::path tmp_path = std::filesystem::temp_directory_path() / tmp_name;
    log::debug("temporary cache path is {}", tmp_path);

    std::ofstream tmp_file = std::ofstream{tmp_path, std::ios::binary};
    if (!tmp_file) {
        log::warn("could not open a file to cache data");
        source(sink);
        return;
    }

    source([&](std::vector<std::byte>&& data) {
        tmp_file.write(reinterpret_cast<const char*>(data.data()), data.size());
        sink(std::move(data));
    });

    tmp_file.close();
    detail_cache::save_cache(path, hash_path, latest_hash, tmp_path);
}

}  // namespace komankondi::dictgen
