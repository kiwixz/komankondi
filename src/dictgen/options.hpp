#pragma once

#include <string>

#include "utils/path.hpp"

namespace komankondi::dictgen {

struct Options {
    bool cache = true;
    std::string dictionary = (get_data_directory() / "<source>.dict").string();
};

}  // namespace komankondi::dictgen
