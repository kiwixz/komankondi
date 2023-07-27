#include "bzip.hpp"

#include <span>
#include <vector>

#include <bzlib.h>
#include <range/v3/algorithm/copy.hpp>

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
    buffer_.resize(buffer_.size() + data.size());
    ranges::copy(data, buffer_.end() - data.size());

    stream_.next_in = const_cast<char*>(reinterpret_cast<const char*>(buffer_.data()));
    stream_.avail_in = buffer_.size();

    std::vector<std::byte> r;
    r.resize(2000000);
    stream_.next_out = reinterpret_cast<char*>(r.data());
    stream_.avail_out = r.size();

    while (true) {
        if (!running_) {
            if (int err = BZ2_bzDecompressInit(&stream_, 0, 0); err)
                throw Exception{"could not open bzip decompressor stream, error {}", err};
            running_ = true;
        }

        int ret = BZ2_bzDecompress(&stream_);
        if (ret == BZ_STREAM_END) {
            if (int err = BZ2_bzDecompressEnd(&stream_); err)
                throw Exception{"could not close bzip decompressor stream, error {}", err};
            running_ = false;

            if (stream_.avail_in == 0)
                break;
        }
        else if (ret) {
            throw Exception{"could not decompress bzip data, error {}", ret};
        }
        else if (stream_.avail_out == 0) {
            int old_size = r.size();
            r.resize(old_size * 2);
            stream_.next_out = reinterpret_cast<char*>(r.data() + old_size);
            stream_.avail_out += old_size;
        }
        else {
            break;
        }
    }

    r.resize(r.size() - stream_.avail_out);

    int consumed = buffer_.size() - stream_.avail_in;
    std::copy_backward(buffer_.begin() + consumed, buffer_.end(), buffer_.end() - consumed);
    buffer_.resize(buffer_.size() - consumed);

    return r;
}

bool BzipDecompressor::finished() const {
    return buffer_.empty();
}

}  // namespace komankondi::dictgen
