#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <span>

#include "utils/file.hpp"

namespace komankondi::dictgen {

struct Cacher {
    Cacher(std::filesystem::path path, std::chrono::file_clock::time_point date);
    ~Cacher();
    Cacher(const Cacher&) = delete;
    Cacher(Cacher&&) = delete;
    Cacher& operator=(const Cacher&) = delete;
    Cacher& operator=(Cacher&&) = delete;

    void save();

    template <typename T>
    void write(std::span<const T> data) {
        tmp_file_->write(data);
    }

private:
    std::filesystem::path path_;
    std::chrono::file_clock::time_point date_;
    std::filesystem::path tmp_path_;
    std::optional<File> tmp_file_;
};


std::optional<File> try_load_cache(const std::filesystem::path& path, std::chrono::file_clock::time_point date);

}  // namespace komankondi::dictgen
