#pragma once

#include <cstdio>
#include <memory>
#include <span>
#include <vector>

#include "utils/config.hpp"
#include "utils/exception.hpp"
#include "utils/zstring_view.hpp"

template <>
struct std::default_delete<FILE> {
    void operator()(FILE* ptr) const;
};


namespace komankondi {

struct File {
    enum class Mode {
        read = 1 << 0,

        write = 1 << 1,
        truncate = 1 << 2,
        append = 1 << 3,

        binary = 1 << 4,
    };

    File() = default;
    File(ZStringView path, Mode mode);

    explicit operator bool() const;

    bool eof() const;

    void sync();

    template <typename T = std::byte>
    std::vector<T> read(int size = default_buffer_size / sizeof(T)) {
        std::vector<T> r;
        r.resize(size);
        r.resize(std::fread(r.data(), sizeof(T), r.size(), stream_.get()));
        if (std::ferror(stream_.get()))
            throw SystemException{"Could not read from file"};
        return r;
    }

    template <typename T>
    void write(std::span<const T> data) {
        if (std::fwrite(data.data(), sizeof(T), data.size(), stream_.get()) < data.size())
            throw SystemException{"Could not write to file"};
    }

private:
    std::unique_ptr<FILE> stream_;
};

File::Mode operator|(File::Mode a, File::Mode b);

}  // namespace komankondi
