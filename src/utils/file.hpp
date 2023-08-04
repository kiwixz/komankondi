#pragma once

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "utils/exception.hpp"
#include "utils/zstring_view.hpp"

namespace std {

template <>
struct default_delete<FILE> {
    void operator()(FILE* ptr) const {
        [[maybe_unused]] int r = std::fclose(ptr);
        assert(r == 0);
    }
};

}  // namespace std


namespace komankondi {

struct File {
    enum class Mode {
        read = 1 << 0,
        write = 1 << 1,
        truncate = 1 << 2,  // implies write
        append = 1 << 3,    // implies write

        binary = 1 << 4,
    };

    File(ZStringView path, Mode mode);
    File(const std::filesystem::path& path, Mode mode);

    bool eof() const;

    void sync();

    template <typename T>
    std::vector<T> read(int size = 2000000 / sizeof(T)) {
        std::vector<T> r;
        r.resize(size);
        r.resize(std::fread(r.data(), sizeof(T), r.size(), stream_.get()));
        if (std::ferror(stream_.get()))
            throw SystemException{"could not read from file"};
        return r;
    }

    template <typename T>
    void write(std::span<const T> data) {
        if (std::fwrite(data.data(), sizeof(T), data.size(), stream_.get()) < data.size())
            throw SystemException{"could not write to file"};
    }

private:
    std::unique_ptr<FILE> stream_;
};

File::Mode operator|(File::Mode a, File::Mode b);

}  // namespace komankondi
