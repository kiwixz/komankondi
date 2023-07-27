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

int Iterate::buffer_size() const {
    return buffer_.size();
}

}  // namespace komankondi::dictgen::xml
