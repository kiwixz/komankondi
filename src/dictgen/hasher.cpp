#include "hasher.hpp"

#include <memory>

#include <openssl/evp.h>

#include "utils/exception.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi::dictgen {

Hasher::Hasher(ZStringView algorithm) {
    impl_.reset(EVP_MD_fetch(nullptr, algorithm.data(), nullptr));
    if (!impl_)
        throw Exception{"could not fetch digest implementation"};

    ctx_.reset(EVP_MD_CTX_new());
    if (!ctx_)
        throw Exception{"could not initialize digest context"};

    if (!EVP_DigestInit_ex(ctx_.get(), impl_.get(), nullptr))
        throw Exception{"could not initialize digest"};
}

void Hasher::update(std::span<const std::byte> data) {
    if (!EVP_DigestUpdate(ctx_.get(), data.data(), data.size()))
        throw Exception{"could not update digest"};
}

std::vector<std::byte> Hasher::finish() {
    std::vector<std::byte> r;
    r.resize(EVP_MAX_MD_SIZE);
    unsigned size = r.size();
    if (!EVP_DigestFinal_ex(ctx_.get(), reinterpret_cast<unsigned char*>(r.data()), &size))
        throw Exception{"could not update digest"};
    r.resize(size);
    return r;
}

void Hasher::reset() {
    if (!EVP_DigestInit_ex(ctx_.get(), impl_.get(), nullptr))
        throw Exception{"could not reset digest"};
}

}  // namespace komankondi::dictgen


void std::default_delete<EVP_MD>::operator()(EVP_MD* ptr) const {
    EVP_MD_free(ptr);
}

void std::default_delete<EVP_MD_CTX>::operator()(EVP_MD_CTX* ptr) const {
    EVP_MD_CTX_free(ptr);
}
