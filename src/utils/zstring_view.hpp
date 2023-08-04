#pragma once

#include <string>
#include <string_view>

#include <fmt/core.h>

namespace komankondi {

struct ZStringView {
    ZStringView(const std::string& str);
    ZStringView(const char* str);

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

}  // namespace komankondi
