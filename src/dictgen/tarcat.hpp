#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace komankondi::dictgen {

struct TarCat {
    bool finished() const;

    std::vector<std::byte> operator()(std::span<const std::byte> data);
    void operator()(std::span<const std::byte> data, std::vector<std::byte>& out);
    void inplace(std::vector<std::byte>& data);

private:
    int64_t remaining_ = 0;
    int padding_ = 0;
    std::vector<std::byte> buf_;
};

}  // namespace komankondi::dictgen
