#pragma once

namespace komankondi {

#ifdef _WIN32
constexpr bool is_windows = true;
#else
constexpr bool is_windows = false;
#endif

}  // namespace komankondi
