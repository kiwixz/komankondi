#include "cache.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>

#include <range/v3/algorithm/equal.hpp>

#include "utils/hex.hpp"

namespace komankondi::dictgen::detail_cache {

std::optional<std::ifstream> open_cache(const std::filesystem::path& path,
                                        const std::filesystem::path& hash_path, std::span<const std::byte> latest_hash) {
    if (!std::filesystem::exists(path)) {
        log::debug("cached data not found");
        return {};
    }

    log::info("found cached data");

    std::ifstream hash_file{hash_path};
    if (!hash_file) {
        log::warn("found cached data but could not open its digest");
        return {};
    }

    std::vector<std::byte> cache_sha1;
    cache_sha1.resize(latest_hash.size() + 1);
    hash_file.read(reinterpret_cast<char*>(cache_sha1.data()), cache_sha1.size());
    cache_sha1.resize(hash_file.gcount());
    log::debug("cached data hash is {}", to_hex(cache_sha1));

    if (!ranges::equal(cache_sha1, latest_hash)) {
        log::info("cached data is stale");
        return {};
    }

    std::ifstream r{path, std::ios::binary};
    if (!r) {
        log::warn("found fresh cached data but could not open it");
        return {};
    }

    return r;
}

void save_cache(const std::filesystem::path& path,
                const std::filesystem::path& hash_path, std::span<const std::byte> latest_hash,
                const std::filesystem::path& tmp_path) {
    std::filesystem::create_directories(path.parent_path());
    std::filesystem::rename(tmp_path, path);

    std::ofstream hash_file{hash_path};
    if (!hash_file) {
        log::warn("could not open the file to write cached data hash");
        return;
    }
    hash_file.write(reinterpret_cast<const char*>(latest_hash.data()), latest_hash.size());
    hash_file.close();

    log::info("successfully saved cached data");
}

}  // namespace komankondi::dictgen::detail_cache
