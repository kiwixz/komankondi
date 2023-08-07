#include "game.hpp"

#include <string>
#include <string_view>

namespace komankondi::game {

Game::Game() {
    solution_ = dict_.pick_word();
}

const std::string& Game::description() const {
    return solution_.description;
}

bool Game::submit(std::string_view word) {
    if (word != solution_.word)
        return false;
    solution_ = dict_.pick_word();
    return true;
}

std::string Game::give_up() {
    std::string r = std::move(solution_.word);
    solution_ = dict_.pick_word();
    return r;
}

}  // namespace komankondi::game
