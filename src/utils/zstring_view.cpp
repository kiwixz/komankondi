#include "zstring_view.hpp"

#include <string>
#include <string_view>

namespace komankondi {

ZStringView::ZStringView(const char* str) :
        view_{str} {
}

ZStringView::ZStringView(const std::string& str) :
        view_{str} {
}

ZStringView::operator std::string_view() const {
    return view_;
}

const char* ZStringView::begin() const {
    return data();
}

const char* ZStringView::end() const {
    return data() + size();
}

const char* ZStringView::data() const {
    return view_.data();
}

size_t ZStringView::size() const {
    return view_.size();
}


std::string_view format_as(ZStringView str) {
    return str;
}

}  // namespace komankondi
