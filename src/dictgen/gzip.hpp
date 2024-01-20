#pragma once

#include <span>
#include <vector>

#include <zlib.h>

#include "utils/buffer.hpp"

namespace komankondi::dictgen {

struct GzipDecompressor {
    GzipDecompressor() = default;
    ~GzipDecompressor();
    GzipDecompressor(const GzipDecompressor&) = delete;
    GzipDecompressor& operator=(const GzipDecompressor&) = delete;
    GzipDecompressor(GzipDecompressor&&) noexcept = delete;
    GzipDecompressor& operator=(GzipDecompressor&&) noexcept = delete;

    bool finished() const;
    bool in_stream() const;

    std::vector<std::byte> operator()(std::span<const std::byte> data = {});

private:
    bool running_ = false;
    Buffer<std::byte> buf_;
    z_stream stream_;
};

}  // namespace komankondi::dictgen
