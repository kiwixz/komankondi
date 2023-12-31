#include "platform.hpp"

#include <cstdio>
#include <string>

#include "utils/exception.hpp"
#include "utils/zstring_view.hpp"

#ifdef _WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif

namespace komankondi {
namespace {

int fsync(int fd) {
#ifdef _WIN32
    return _commit(fd);
#else
    return ::fsync(fd);
#endif
}

}  // namespace


FILE* fopen(const std::filesystem::path& path, ZStringView mode) {
#ifdef _WIN32
    return _wfopen(path.c_str(), std::wstring{mode.begin(), mode.end()}.c_str());
#else
    return std::fopen(path.c_str(), mode.data());
#endif
}

void fsync(FILE* stream) {
    int fd = fileno(stream);
    if (fd == -1)
        throw SystemException{"Could not get file descriptor of stream"};
    if (fsync(fd))
        throw SystemException{"Could not synchronize file"};
}

bool isatty(FILE* stream) {
    int fd = fileno(stream);
    if (fd == -1)
        return false;
    return ::isatty(fd);
}

}  // namespace komankondi
