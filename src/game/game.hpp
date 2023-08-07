#pragma once

#include <string>
#include <string_view>

#include "dict/reader.hpp"
#include "dict/word.hpp"
#include "game/profile.hpp"
#include "utils/path.hpp"

namespace komankondi::game {

struct Game {
    Game();

    const std::string& description() const;

    bool submit(std::string_view word);
    std::string give_up();

private:
    Profile profile_;
    dict::Reader dict_{make_zstring_view(get_data_directory() / fmt::format("{}.dict", profile_.dictionary()))};
    dict::Word solution_;
};

}  // namespace komankondi::game
