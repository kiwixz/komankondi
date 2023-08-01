#include "xml.hpp"

#include <string>

namespace komankondi::dictgen::xml {
namespace detail {

IterateState::Element::Element(std::string&& name_) :
        name{std::move(name_)} {
}

}  // namespace detail


bool Iterate::finished() const {
    return buffer_.empty() && state_.stack.empty();
}


Select::Select(std::string&& path, std::vector<std::string>&& subpaths) :
        select_path_{std::move(path)},
        select_subpaths_{std::move(subpaths)} {
}

bool Select::finished() const {
    return parser_.finished();
}

}  // namespace komankondi::dictgen::xml
