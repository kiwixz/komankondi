#pragma once

#include <filesystem>
#include <optional>

#include <boost/interprocess/sync/file_lock.hpp>

#include "utils/file.hpp"

namespace komankondi::dictgen {

struct Cacher {
    Cacher() = default;
    Cacher(std::filesystem::path path);
    ~Cacher();
    Cacher(const Cacher&) = delete;
    Cacher& operator=(const Cacher&) = delete;
    Cacher(Cacher&&) noexcept = default;
    Cacher& operator=(Cacher&&) noexcept = default;

    void save();

    template <typename T>
    void write(std::span<const T> data) {
        tmp_file_.write(data);
    }

private:
    std::filesystem::path path_;
    std::filesystem::path tmp_path_;
    File tmp_file_;
    boost::interprocess::file_lock tmp_lock_;
};


std::optional<File> try_load_cache(const std::filesystem::path& path);

}  // namespace komankondi::dictgen
