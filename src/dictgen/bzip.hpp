#pragma once

#include <span>
#include <vector>

#include <bzlib.h>

#include "utils/buffer.hpp"

namespace komankondi::dictgen {

struct BzipDecompressor {
    BzipDecompressor() = default;
    ~BzipDecompressor();
    BzipDecompressor(const BzipDecompressor&) = delete;
    BzipDecompressor& operator=(const BzipDecompressor&) = delete;
    BzipDecompressor(BzipDecompressor&&) = delete;
    BzipDecompressor& operator=(BzipDecompressor&&) = delete;

    bool finished() const;
    bool in_stream() const;

    std::vector<std::byte> operator()(std::span<const std::byte> data = {});

private:
    bool running_ = false;
    Buffer<std::byte> buf_;
    bz_stream stream_{};
};

}  // namespace komankondi::dictgen
