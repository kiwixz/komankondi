#pragma once

#include <string>
#include <tuple>

#include "dict/database.hpp"
#include "dict/word.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi::dict {

struct Reader {
    Reader(ZStringView path);

    Word pick_word();

private:
    Database db_;
    Database::Operation<std::tuple<std::string, std::string>, int> op_pick_word_;
    int nr_words_;
};

}  // namespace komankondi::dict
