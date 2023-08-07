#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include <fmt/core.h>

namespace komankondi {

struct ZStringView {
    ZStringView(const char* str);
    ZStringView(const std::string& str);

    operator std::string_view() const;

    const char* begin() const;
    const char* end() const;

    const char* data() const;
    size_t size() const;

    friend bool operator<=>(ZStringView a, ZStringView b) = default;

private:
    std::string_view view_;
};


std::string_view format_as(ZStringView str);

inline decltype(auto) make_zstring_view(const std::filesystem::path& str) {
    if constexpr (std::is_same_v<std::filesystem::path::value_type, char>) {
        return str.native();
    }
    else {
        return str.string();
    }
}

}  // namespace komankondi
