#pragma once

#include <span>
#include <vector>

#include "utils/buffer.hpp"

namespace komankondi::dictgen {

struct TarCat {
    bool finished() const;

    std::vector<std::byte> operator()(std::span<const std::byte> data);
    void operator()(std::span<const std::byte> data, std::vector<std::byte>& out);
    void inplace(std::vector<std::byte>& data);

private:
    int remaining_ = 0;
    int padding_ = 0;
    Buffer<std::byte> buf_;
};

}  // namespace komankondi::dictgen
