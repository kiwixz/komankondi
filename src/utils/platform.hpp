#pragma once

#include <cstdio>
#include <filesystem>
#include <string>

namespace komankondi {

FILE* fopen(const std::filesystem::path& path, const std::string& mode);
void fsync(FILE* stream);

}  // namespace komankondi
