#include "gzip.hpp"

#include <span>
#include <vector>

#include <zlib.h>

#include "utils/config.hpp"
#include "utils/exception.hpp"
#include "utils/log.hpp"

namespace komankondi::dictgen {

GzipDecompressor::~GzipDecompressor() {
    if (!running_)
        return;

    if (int err = inflateEnd(&stream_); err)
        log::error("Could not cleanup gzip decompressor stream, error {}", err);
}

bool GzipDecompressor::finished() const {
    return !running_;
}

std::vector<std::byte> GzipDecompressor::operator()(std::span<const std::byte> data) {
    std::vector<std::byte> r;
    (*this)(data, r);
    return r;
}

void GzipDecompressor::operator()(std::span<const std::byte> data, std::vector<std::byte>& out) {
    stream_.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(data.data()));
    stream_.avail_in = data.size();

    while (stream_.avail_in > 0) {
        size_t out_offset = out.size();
        out.resize(out_offset + default_buffer_size);
        stream_.next_out = reinterpret_cast<unsigned char*>(out.data() + out_offset);
        stream_.avail_out = default_buffer_size;

        if (!running_) {
            if (int err = inflateInit2(&stream_, 15 + 16); err)  // max window size + 16 for gzip mode
                throw Exception{"Could not open gzip decompressor stream, error {}", err};
            running_ = true;
        }
        int ret = inflate(&stream_, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            if (int err = inflateEnd(&stream_); err)
                throw Exception{"Could not close gzip decompressor stream, error {}", err};
            running_ = false;
        }
        else if (ret) {
            throw Exception{"Could not decompress gzip data, error {}", ret};
        }

        out.resize(out.size() - stream_.avail_out);
    }
}

}  // namespace komankondi::dictgen
