#pragma once

#include <string>
#include <string_view>

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

}  // namespace komankondi
