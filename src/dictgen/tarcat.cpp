#include "tarcat.hpp"

#include <algorithm>
#include <span>
#include <string_view>
#include <vector>

#include "utils/parse.hpp"

namespace komankondi::dictgen {

bool TarCat::finished() const {
    return remaining_ == 0 && buf_.empty();
}

std::vector<std::byte> TarCat::operator()(std::span<const std::byte> data) {
    std::vector<std::byte> r;
    (*this)(data, r);
    return r;
}

void TarCat::operator()(std::span<const std::byte> data, std::vector<std::byte>& out) {
    constexpr int tar_block_size = 512;

    out.reserve(out.size() + data.size());

    int offset = 0;
    while (offset < std::ssize(data)) {
        int unparsed_size = data.size() - offset;

        if (unparsed_size <= remaining_) {
            remaining_ -= unparsed_size;
            out.insert(out.end(), data.begin() + offset, data.end());
            break;
        }
        out.insert(out.end(), data.begin() + offset, data.begin() + offset + remaining_);
        offset += remaining_;
        unparsed_size -= remaining_;
        remaining_ = 0;

        if (unparsed_size <= padding_) {
            padding_ -= unparsed_size;
            break;
        }
        offset += padding_;
        unparsed_size -= remaining_;
        padding_ = 0;

        if (buf_.size() + unparsed_size < tar_block_size) {
            buf_.insert(buf_.end(), data.begin() + offset, data.end());
            break;
        }
        int header_size_in_data = tar_block_size - buf_.size();
        std::span<const std::byte> header = [&]() -> std::span<const std::byte> {
            if (buf_.empty())
                return data.subspan(offset, tar_block_size);
            buf_.insert(buf_.end(), data.begin() + offset, data.begin() + offset + header_size_in_data);
            return buf_;
        }();

        std::string_view file_size{reinterpret_cast<const char*>(header.data()) + 124, 11};
        if (file_size[0] != '\0') {
            remaining_ = parse<int>(file_size, 8);
            padding_ = tar_block_size - remaining_ % tar_block_size;
        }
        buf_.clear();

        offset += header_size_in_data;
    }
}

void TarCat::inplace(std::vector<std::byte>& data) {
    constexpr int tar_block_size = 512;

    int content_size = 0;
    int offset = 0;
    while (offset < std::ssize(data)) {
        int unparsed_size = data.size() - offset;

        if (unparsed_size <= remaining_) {
            remaining_ -= unparsed_size;
            if (offset == 0)
                return;
            std::copy(data.begin() + offset, data.end(), data.begin() + content_size);
            content_size += unparsed_size;
            break;
        }
        if (offset != 0)
            std::copy_n(data.begin() + offset, remaining_, data.begin() + content_size);
        content_size += remaining_;
        offset += remaining_;
        unparsed_size -= remaining_;
        remaining_ = 0;

        if (unparsed_size <= padding_) {
            padding_ -= unparsed_size;
            break;
        }
        offset += padding_;
        unparsed_size -= remaining_;
        padding_ = 0;

        if (buf_.size() + unparsed_size < tar_block_size) {
            buf_.insert(buf_.end(), data.begin() + offset, data.end());
            break;
        }
        int header_size_in_data = tar_block_size - buf_.size();
        std::span<const std::byte> header = [&]() {
            if (buf_.empty())
                return std::span{data}.subspan(offset, tar_block_size);
            buf_.insert(buf_.end(), data.begin() + offset, data.begin() + offset + header_size_in_data);
            return std::span{buf_};
        }();

        std::string_view file_size{reinterpret_cast<const char*>(header.data()) + 124, 11};
        if (file_size[0] != '\0') {
            remaining_ = parse<int>(file_size, 8);
            padding_ = tar_block_size - remaining_ % tar_block_size;
        }
        buf_.clear();

        offset += header_size_in_data;
    }

    data.resize(content_size);
}

}  // namespace komankondi::dictgen
