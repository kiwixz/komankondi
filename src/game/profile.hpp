#pragma once

#include <string>

#include "utils/database.hpp"

namespace komankondi::game {

struct Profile {
    Profile();

    std::string dictionary();

private:
    Database db_;
};

}  // namespace komankondi::game
