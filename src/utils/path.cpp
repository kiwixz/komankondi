#include "path.hpp"

#include <cstdlib>
#include <string>

#include <fmt/core.h>

namespace komankondi {

std::string get_cache_directory() {
    static std::string r = []() -> std::string {
        const char* cache_dir = std::getenv("XDG_CACHE_HOME");
        if (!cache_dir)
            cache_dir = std::getenv("LOCALAPPDATA");
        if (cache_dir)
            return fmt::format("{}/komankondi", cache_dir);

        const char* home_dir = std::getenv("HOME");
        if (home_dir)
            return fmt::format("{}/.cache/komankondi", home_dir);

        return ".komankondi/cache";
    }();
    return r;
}

std::string get_data_directory() {
    static std::string r = []() -> std::string {
        const char* data_dir = std::getenv("XDG_DATA_HOME");
        if (!data_dir)
            data_dir = std::getenv("APPDATA");
        if (data_dir)
            return fmt::format("{}/komankondi", data_dir);

        const char* home_dir = std::getenv("HOME");
        if (home_dir)
            return fmt::format("{}/.local/share/komankondi", home_dir);

        return ".komankondi";
    }();
    return r;
}

}  // namespace komankondi
