#pragma once

#include <filesystem>
#include <optional>

#include <boost/interprocess/sync/file_lock.hpp>

#include "utils/file.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi::dictgen {

struct Cacher {
    Cacher() = default;
    Cacher(std::string path);
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
    std::string path_;
    std::string tmp_path_;
    File tmp_file_;
    boost::interprocess::file_lock tmp_lock_;
};


std::optional<File> try_load_cache(ZStringView path);

}  // namespace komankondi::dictgen
