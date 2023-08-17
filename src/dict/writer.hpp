#pragma once

#include <string_view>

#include "dict/word.hpp"
#include "utils/database.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi::dict {

struct Writer {
    Writer(ZStringView path);

    void add_word(const Word& word);
    void save();

private:
    Database db_;
    Database::Operation<void, std::string_view, std::string_view> op_add_word_;
};

}  // namespace komankondi::dict
