#pragma once

#include <filesystem>

namespace komankondi {

std::filesystem::path get_cache_directory();
std::filesystem::path get_data_directory();

}  // namespace komankondi
