#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

namespace komankondi {

/// Thread-safe
template <typename Element>
struct ConsumeQueue {
    ConsumeQueue() = default;
    explicit ConsumeQueue(int max_size) :
            max_size_{max_size} {
        assert(max_size_ > 0);
    }

    void push(Element&& element) {
        {
            std::unique_lock lock{mutex_};
            condvar_push_.wait(lock, [&] { return std::ssize(queue_) < max_size_; });
            queue_.push(std::move(element));
        }
        condvar_pop_.notify_one();
    }

    void close() {
        {
            std::lock_guard lock{mutex_};
            closed_ = true;
        }
        condvar_pop_.notify_all();
    }

    std::optional<Element> pop() {
        std::optional<Element> r;
        {
            std::unique_lock lock{mutex_};
            condvar_pop_.wait(lock, [&] { return closed_ || !queue_.empty(); });
            if (closed_ && queue_.empty())
                return {};
            r = std::move(queue_.front());
            queue_.pop();
        }
        condvar_push_.notify_one();
        return r;
    }

private:
    int max_size_ = std::min<int>(std::thread::hardware_concurrency(), 8);

    bool closed_ = false;
    std::mutex mutex_;
    std::condition_variable condvar_push_;
    std::condition_variable condvar_pop_;
    std::queue<Element> queue_;
};

}  // namespace komankondi
