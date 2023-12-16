#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <optional>

namespace komankondi {

template <typename Type_, typename Mutex_ = std::mutex>
struct Guarded {
    using Type = Type_;
    using Mutex = Mutex_;

    struct Handle {
        Handle(Type& native, std::unique_lock<Mutex>&& lock) :
                native_{&native}, lock_{std::move(lock)} {
            assert(lock_);
        }

        explicit operator bool() const {
            return native_;
        }

        const Type& operator*() const {
            return *native_;
        }
        Type& operator*() {
            return *native_;
        }

        const Type* operator->() const {
            return native_;
        }
        Type* operator->() {
            return native_;
        }

        template <typename T>
        void wait(std::condition_variable& convar, T&& predicate) {
            convar.wait(lock_, std::forward<T>(predicate));
        }

    private:
        Type* native_;
        std::unique_lock<Mutex> lock_;
    };

    template <typename... Args>
    explicit Guarded(Args&&... args) :
            native_{std::forward<Args>(args)...} {
    }

    ~Guarded() = default;
    Guarded(const Guarded&) = delete;
    Guarded& operator=(const Guarded&) = delete;
    Guarded(Guarded&&) noexcept = delete;
    Guarded& operator=(Guarded&&) noexcept = delete;

    Handle lock() {
        return {native_, std::unique_lock{mutex_}};
    }

    std::optional<Handle> try_lock() {
        std::unique_lock<Mutex> lock{mutex_, std::try_to_lock};
        if (!lock)
            return {};
        return {native_, std::move(lock)};
    }

private:
    Type native_;
    Mutex mutex_;
};


template <typename T>
using GuardedHandle = typename Guarded<T>::Handle;

}  // namespace komankondi
