#include "utils/config.hpp"

#include <thread>

namespace komankondi {

int default_parallel_queue_size() {
    static const int r = std::thread::hardware_concurrency() + 1;
    return r;
}

}  // namespace komankondi
