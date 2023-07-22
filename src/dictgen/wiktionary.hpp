#pragma once

#include <string_view>

#include "dictgen/options.hpp"

namespace komankondi::dictgen {

void dictgen_wiktionary(std::string_view language, const Options& opt);

}  // namespace komankondi::dictgen
