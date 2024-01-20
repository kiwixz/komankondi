#include "dictgen/cache.hpp"

#include <filesystem>
#include <optional>
#include <string>

#include <boost/interprocess/sync/file_lock.hpp>
#include <fmt/core.h>
#include <fmt/std.h>

#include "utils/file.hpp"
#include "utils/log.hpp"

namespace komankondi::dictgen {

Cacher::Cacher(std::filesystem::path path) :
        path_{std::move(path)} {
    tmp_path_ = path_;
    tmp_path_ += ".new";
    log::debug("Saving temporary cache to {}", tmp_path_);

    auto lock = [&] {
        tmp_lock_ = {tmp_path_.c_str()};
        if (!tmp_lock_.try_lock())
            throw Exception{"Could not gain exclusive access to the temporary cache file"};
    };

    if (std::filesystem::exists(tmp_path_)) {
        log::debug("Temporary cache file already exists");
        lock();
        tmp_file_ = {tmp_path_, File::Mode::truncate | File::Mode::binary};
    }
    else {
        std::filesystem::create_directories(path_.parent_path());
        tmp_file_ = {tmp_path_, File::Mode::write | File::Mode::binary};
        lock();
    }
}

Cacher::~Cacher() {
    if (!tmp_file_)
        return;
    tmp_lock_ = {};
    tmp_file_ = {};
    try {
        std::filesystem::remove(tmp_path_);
    }
    catch (const std::exception& ex) {
        log::warn("Could not clean incomplete cache file: {}", ex.what());
    }
}

void Cacher::save() {
    tmp_file_.sync();
    std::filesystem::rename(tmp_path_, path_);
    tmp_lock_ = {};
    tmp_file_ = {};
    log::info("Successfully saved cache");
}


std::optional<File> try_load_cache(const std::filesystem::path& path) {
    log::debug("Cache path is {}", path);

    try {
        if (std::filesystem::exists(path)) {
            log::info("Found cache");
            return File{path, File::Mode::read | File::Mode::binary};
        }
    }
    catch (const std::exception& ex) {
        log::warn("Could not look for cache: {}", ex.what());
    }
    return {};
}

}  // namespace komankondi::dictgen
