#include "bzip.hpp"

#include <span>
#include <vector>

#include <bzlib.h>

#include "utils/config.hpp"
#include "utils/exception.hpp"
#include "utils/log.hpp"

namespace komankondi::dictgen {

BzipDecompressor::~BzipDecompressor() {
    if (running_) {
        if (int err = BZ2_bzDecompressEnd(&stream_); err)
            log::error("could not cleanup bzip decompressor stream, error {}", err);
    }
}

std::vector<std::byte> BzipDecompressor::operator()(std::span<const std::byte> data) {
    buf_.push(data);

    if (!running_) {
        if (buf_.empty())
            return {};

        if (int err = BZ2_bzDecompressInit(&stream_, 0, 0); err)
            throw Exception{"could not open bzip decompressor stream, error {}", err};
        running_ = true;
    }

    stream_.next_in = const_cast<char*>(reinterpret_cast<const char*>(buf_.data()));
    stream_.avail_in = buf_.size();

    std::vector<std::byte> r;
    r.resize(default_buffer_size);
    stream_.next_out = reinterpret_cast<char*>(r.data());
    stream_.avail_out = r.size();

    int ret = BZ2_bzDecompress(&stream_);
    buf_.consume(buf_.size() - stream_.avail_in);
    if (ret == BZ_STREAM_END) {
        if (int err = BZ2_bzDecompressEnd(&stream_); err)
            throw Exception{"could not close bzip decompressor stream, error {}", err};
        running_ = false;
    }
    else if (ret) {
        throw Exception{"could not decompress bzip data, error {}", ret};
    }

    r.resize(r.size() - stream_.avail_out);
    return r;
}

bool BzipDecompressor::finished() const {
    return !running_ && buf_.empty();
}

bool BzipDecompressor::in_stream() const {
    return running_;
}

}  // namespace komankondi::dictgen
