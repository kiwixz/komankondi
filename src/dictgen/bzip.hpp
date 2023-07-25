#pragma once

#include <span>
#include <vector>

#include <bzlib.h>

namespace komankondi::dictgen {

struct BzipDecompressor {
    BzipDecompressor() = default;
    ~BzipDecompressor();
    BzipDecompressor(const BzipDecompressor&) = delete;
    BzipDecompressor& operator=(const BzipDecompressor&) = delete;
    BzipDecompressor(BzipDecompressor&&) = delete;
    BzipDecompressor& operator=(BzipDecompressor&&) = delete;

    std::vector<std::byte> operator()(std::span<const std::byte> data);

    bool finished();

private:
    std::vector<std::byte> buffer_;
    bool running_ = false;
    bz_stream stream_{};
};

}  // namespace komankondi::dictgen
