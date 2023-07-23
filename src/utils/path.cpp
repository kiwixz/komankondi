#include "path.hpp"

#include <cstdlib>
#include <filesystem>

namespace komankondi {

std::filesystem::path get_cache_directory() {
    std::filesystem::path suffix = "komankondi";

    const char* cache_dir = std::getenv("XDG_CACHE_HOME");
    if (!cache_dir)
        cache_dir = std::getenv("LOCALAPPDATA");
    if (cache_dir)
        return cache_dir / suffix;

    suffix = ".cache" / suffix;
    const char* home_dir = std::getenv("HOME");
    if (home_dir)
        return home_dir / suffix;

    return suffix;
}

}  // namespace komankondi
