#pragma once

#include <string>
#include <string_view>

#include <boost/regex.hpp>

#include "utils/zstring_view.hpp"

namespace komankondi::dictgen {

struct LanguageSpec {
    std::string name;
    std::string code;

    boost::regex re_language;
    boost::regex re_form;
    boost::regex re_definition;
};

LanguageSpec find_language_spec(std::string_view query);
void generate_dictionary(ZStringView path, const LanguageSpec& language_spec, bool cache);

}  // namespace komankondi::dictgen
