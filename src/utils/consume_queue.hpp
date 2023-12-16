#pragma once

#include <cassert>
#include <condition_variable>
#include <optional>
#include <queue>

#include "utils/config.hpp"
#include "utils/guarded.hpp"

namespace komankondi {

/// Thread-safe multi-producers multi-consumers queue.
template <typename Element>
struct ConsumeQueue {
    ConsumeQueue() = default;
    explicit ConsumeQueue(int max_size) :
            max_size_{max_size} {
        assert(max_size_ > 0);
    }

    void close() {
        shared_.lock()->closed = true;
        condvar_push_.notify_all();
        condvar_pop_.notify_all();
    }

    bool push(Element&& element) {
        {
            GuardedHandle<Shared> s = shared_.lock();
            s.wait(condvar_push_, [&] { return s->closed || std::ssize(s->queue) < max_size_; });
            if (s->closed)
                return false;
            s->queue.push(std::move(element));
        }
        condvar_pop_.notify_one();
        return true;
    }

    std::optional<Element> pop() {
        std::optional<Element> r;
        {
            GuardedHandle<Shared> s = shared_.lock();
            s.wait(condvar_pop_, [&] { return s->closed || !s->queue.empty(); });
            if (s->closed && s->queue.empty())
                return {};
            r = std::move(s->queue.front());
            s->queue.pop();
        }
        condvar_push_.notify_one();
        return r;
    }

private:
    int max_size_ = default_parallel_queue_size();

    struct Shared {
        bool closed = false;
        std::queue<Element> queue;
    };
    Guarded<Shared> shared_;

    std::condition_variable condvar_push_;
    std::condition_variable condvar_pop_;
};

}  // namespace komankondi
