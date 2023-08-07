#include "reader.hpp"

#include <random>
#include <string>
#include <tuple>

#include "dict/word.hpp"
#include "utils/zstring_view.hpp"

namespace komankondi::dict {

Reader::Reader(ZStringView path) :
        db_{path, true} {
    nr_words_ = std::get<0>(db_.exec<std::tuple<int>>("SELECT COUNT() FROM word"));
}

Word Reader::pick_word() {
    if (!op_pick_word_)
        op_pick_word_ = db_.prepare<std::tuple<std::string, std::string>, int>("SELECT * FROM word WHERE rowid=(SELECT rowid FROM word LIMIT 1 OFFSET ?)");
    std::random_device rng{};
    int index = std::uniform_int_distribution{0, nr_words_ - 1}(rng);
    return std::apply([](auto&&... a) { return Word{std::move(a)...}; }, op_pick_word_.exec(index));
}

}  // namespace komankondi::dict
