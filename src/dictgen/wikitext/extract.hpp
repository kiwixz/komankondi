#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace komankondi::dictgen::wikitext {

std::vector<std::string> extract_definitions(std::string_view input);

}  // namespace komankondi::dictgen::wikitext
