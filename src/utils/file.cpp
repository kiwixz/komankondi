#include "file.hpp"

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>
#include <system_error>

#include <fmt/format.h>

#include "utils/exception.hpp"
#include "utils/log.hpp"
#include "utils/platform.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi {
namespace {

bool operator&(File::Mode a, File::Mode b) {
    return static_cast<int>(a) & static_cast<int>(b);
}

std::string compute_mode(File::Mode mode) {
    std::string r;
    if (mode & (File::Mode::write | File::Mode::truncate | File::Mode::append)) {
        assert(!(mode & File::Mode::truncate && mode & File::Mode::append));
        r = mode & File::Mode::append ? 'a' : 'w';
        if (mode & File::Mode::read)
            r += '+';
        if (mode & File::Mode::binary)
            r += 'b';
        if (!(mode & (File::Mode::truncate | File::Mode::append)))
            r += 'x';
    }
    else {
        assert(mode & File::Mode::read);
        r = 'r';
        if (mode & File::Mode::binary)
            r += 'b';
    }
    return r;
}

}  // namespace


File::File(ZStringView path, Mode mode) {
    stream_.reset(std::fopen(path.data(), compute_mode(mode).c_str()));
    if (!stream_)
        throw SystemException{"Could not open file '{}'", path};
    if (std::setvbuf(stream_.get(), nullptr, _IOFBF, default_buffer_size))
        throw SystemException{"Could not set file buffer '{}'", path};
}

File::File(const std::filesystem::path& path, Mode mode) {
    stream_.reset(fopen(path, compute_mode(mode)));
    if (!stream_)
        throw SystemException{"Could not open file '{}'", path.string()};
}

File::operator bool() const {
    return static_cast<bool>(stream_);
}

bool File::eof() const {
    return std::feof(stream_.get());
}

void File::sync() {
    if (std::fflush(stream_.get()))
        throw SystemException{"Could not flush file"};
    fsync(stream_.get());
}

File::Mode operator|(File::Mode a, File::Mode b) {
    return static_cast<File::Mode>(static_cast<int>(a) | static_cast<int>(b));
}

}  // namespace komankondi


void std::default_delete<FILE>::operator()(FILE* ptr) const {
    if (int err = std::fclose(ptr); err) {
        komankondi::log::error("Could not close file: {}", std::system_error{errno, std::system_category()}.what());
        assert(false);
    }
}
