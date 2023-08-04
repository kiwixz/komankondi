#pragma once

#include <string>

namespace komankondi::dictgen {

struct Options {
    bool cache = true;
    std::string dictionary = "<source>.dict";
};

}  // namespace komankondi::dictgen
