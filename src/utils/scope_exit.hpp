#pragma once

#include <utility>

namespace komankondi {

template <typename T>
struct [[nodiscard]] ScopeExit {
    explicit ScopeExit(T&& function) :
            function_{std::move(function)} {
    }

    ~ScopeExit() {
        function_();
    }

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit(ScopeExit&& other) noexcept = delete;
    ScopeExit& operator=(ScopeExit&& other) noexcept = delete;

private:
    T function_;
};

}  // namespace komankondi
