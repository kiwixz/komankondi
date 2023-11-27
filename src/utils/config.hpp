#pragma once

#include <algorithm>
#include <cstdio>

namespace komankondi {

constexpr int default_buffer_size = std::max(BUFSIZ, 65536);

int default_parallel_queue_size();

}  // namespace komankondi
