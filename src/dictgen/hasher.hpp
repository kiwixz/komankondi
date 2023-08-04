#pragma once

#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include <openssl/evp.h>

#include "utils/zstring_view.hpp"

namespace std {

template <>
struct default_delete<EVP_MD> {
    void operator()(EVP_MD* ptr) const {
        EVP_MD_free(ptr);
    }
};

template <>
struct default_delete<EVP_MD_CTX> {
    void operator()(EVP_MD_CTX* ptr) const {
        EVP_MD_CTX_free(ptr);
    }
};

}  // namespace std


namespace komankondi::dictgen {

struct Hasher {
    Hasher(ZStringView algorithm);

    void update(std::span<const std::byte> data);
    std::vector<std::byte> finish();
    void reset();

private:
    std::unique_ptr<EVP_MD> impl_;
    std::unique_ptr<EVP_MD_CTX> ctx_;
};

}  // namespace komankondi::dictgen
