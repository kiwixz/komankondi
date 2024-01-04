#include "gzip.hpp"

#include <span>
#include <vector>

#include <zlib.h>

#include "utils/config.hpp"
#include "utils/exception.hpp"
#include "utils/log.hpp"

namespace komankondi::dictgen {

GzipDecompressor::~GzipDecompressor() {
    if (running_) {
        if (int err = inflateEnd(&stream_); err)
            log::error("Could not cleanup gzip decompressor stream, error {}", err);
    }
}

bool GzipDecompressor::finished() const {
    return !running_ && buf_.empty();
}

bool GzipDecompressor::in_stream() const {
    return running_;
}

std::vector<std::byte> GzipDecompressor::operator()(std::span<const std::byte> data) {
    buf_.push(data);
    if (buf_.empty())
        return {};

    stream_.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(buf_.data()));
    stream_.avail_in = buf_.size();

    if (!running_) {
        stream_.zalloc = nullptr;
        stream_.zfree = nullptr;
        if (int err = inflateInit2(&stream_, 15 + 16); err)  // max window size + 16 for gzip mode
            throw Exception{"Could not open gzip decompressor stream, error {}", err};
        running_ = true;
    }

    std::vector<std::byte> r;
    r.resize(default_buffer_size);
    stream_.next_out = reinterpret_cast<unsigned char*>(r.data());
    stream_.avail_out = r.size();

    int ret = inflate(&stream_, Z_NO_FLUSH);
    buf_.consume(buf_.size() - stream_.avail_in);
    if (ret == Z_STREAM_END) {
        if (int err = inflateEnd(&stream_); err)
            throw Exception{"Could not close gzip decompressor stream, error {}", err};
        running_ = false;
    }
    else if (ret) {
        throw Exception{"Could not decompress gzip data, error {}", ret};
    }

    r.resize(r.size() - stream_.avail_out);
    return r;
}

}  // namespace komankondi::dictgen
