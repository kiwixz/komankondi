#include "dictgen/cache.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <span>
#include <string>

#include <fmt/core.h>
#include <fmt/std.h>

#include "utils/file.hpp"
#include "utils/log.hpp"

namespace komankondi::dictgen {

Cacher::Cacher(std::filesystem::path path, std::chrono::file_clock::time_point date) :
        path_{std::move(path)}, date_{date} {
    tmp_path_ = path_;
    tmp_path_ += ".new";
    log::debug("Saving temporary cache to {}", tmp_path_);

    if (std::filesystem::exists(tmp_path_)) {
        log::warn("Found incomplete cache file, assuming it to not be used anymore");
        tmp_file_ = {tmp_path_, File::Mode::truncate | File::Mode::binary};
    }
    else {
        std::filesystem::create_directories(path_.parent_path());
        tmp_file_ = {tmp_path_, File::Mode::write | File::Mode::binary};
    }
}

Cacher::~Cacher() {
    if (!tmp_file_)
        return;
    tmp_file_.reset();
    try {
        std::filesystem::remove(tmp_path_);
    }
    catch (const std::exception& ex) {
        log::warn("Could not clean incomplete cache file: {}", ex.what());
    }
}

void Cacher::save() {
    tmp_file_->sync();
    tmp_file_.reset();
    std::filesystem::last_write_time(tmp_path_, date_);
    std::filesystem::rename(tmp_path_, path_);
    log::info("Successfully saved cache");
}

std::optional<File> try_load_cache(const std::filesystem::path& path, std::chrono::file_clock::time_point date) {
    log::debug("Cache path is {}", path);

    try {
        if (std::filesystem::exists(path)) {
            log::info("Found cache");

            if (std::filesystem::last_write_time(path) >= date - std::chrono::seconds{1})
                return File{path, File::Mode::read | File::Mode::binary};

            log::info("Cache is stale");
        }
    }
    catch (const std::exception& ex) {
        log::warn("Could not look for cache: {}", ex.what());
    }
    return {};
}

}  // namespace komankondi::dictgen
