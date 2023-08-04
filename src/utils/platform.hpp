#pragma once

#include <cstdio>
#include <filesystem>

#include "utils/zstring_view.hpp"

namespace komankondi {

FILE* fopen(const std::filesystem::path& path, ZStringView mode);
void fsync(FILE* stream);

}  // namespace komankondi
