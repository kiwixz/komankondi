#pragma once

#include <string_view>

#include "dict/database.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi::dict {

struct Writer {
    Writer(ZStringView path);
    ~Writer();
    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;
    Writer(Writer&&) = default;
    Writer& operator=(Writer&&) = default;

    void add_word(std::string_view word, std::string_view description);

private:
    Database db_;
    Database::Operation<void, std::string_view, std::string_view> op_add_word_;
};

}  // namespace komankondi::dict
