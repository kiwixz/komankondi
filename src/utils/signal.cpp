#include <atomic>
#include <csignal>

#include "utils/exception.hpp"

namespace komankondi {
namespace {

std::atomic_flag& terminating_mutable() {
    static std::atomic_flag r;
    return r;
}

extern "C" void handler(int /*signal*/) noexcept {
    terminating_mutable().test_and_set(std::memory_order::relaxed);
}

}  // namespace


void catch_termination_signal() {
    for (int signal : {SIGTERM, SIGINT}) {
        if (std::signal(signal, &handler) == SIG_ERR)  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
            throw Exception{"Could not set termination signal handlers"};
    }
}

bool terminating() {
    return terminating_mutable().test(std::memory_order::relaxed);
}

}  // namespace komankondi
