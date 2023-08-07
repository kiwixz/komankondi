#include "path.hpp"

#include <cstdlib>
#include <filesystem>

namespace komankondi {

std::filesystem::path get_cache_directory() {
    static std::filesystem::path r = [] {
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
    }();
    return r;
}

std::filesystem::path get_data_directory() {
    static std::filesystem::path r = [] {
        std::filesystem::path suffix = "komankondi";

        const char* data_dir = std::getenv("XDG_DATA_HOME");
        if (!data_dir)
            data_dir = std::getenv("APPDATA");
        if (data_dir)
            return data_dir / suffix;

        suffix = ".local" / ("share" / suffix);
        const char* home_dir = std::getenv("HOME");
        if (home_dir)
            return home_dir / suffix;

        return suffix;
    }();
    return r;
}

}  // namespace komankondi
