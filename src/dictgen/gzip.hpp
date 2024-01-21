#pragma once

#include <span>
#include <vector>

#include <zlib.h>

namespace komankondi::dictgen {

struct GzipDecompressor {
    GzipDecompressor() = default;
    ~GzipDecompressor();
    GzipDecompressor(const GzipDecompressor&) = delete;
    GzipDecompressor& operator=(const GzipDecompressor&) = delete;
    GzipDecompressor(GzipDecompressor&&) noexcept = delete;
    GzipDecompressor& operator=(GzipDecompressor&&) noexcept = delete;

    bool finished() const;

    std::vector<std::byte> operator()(std::span<const std::byte> data);
    void operator()(std::span<const std::byte> data, std::vector<std::byte>& out);

private:
    bool running_ = false;
    z_stream stream_{};
};

}  // namespace komankondi::dictgen
