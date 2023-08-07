#pragma once

#include <exception>
#include <functional>
#include <thread>
#include <type_traits>
#include <utility>

#include "utils/consume_queue.hpp"
#include "utils/log.hpp"

namespace komankondi::dictgen {

static_assert(sizeof(std::vector<std::byte>));

template <typename Input, typename Function>
struct Worker {
    static_assert(std::is_invocable_v<Function, Input>);
    static_assert(std::is_same_v<std::invoke_result_t<Function, Input>, void>);

    Worker(Function func) :
            func_{std::move(func)},
            thread_{std::bind_front(&Worker::loop, this)} {
    }

    ~Worker() {
        join();
    }

    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;
    Worker(Worker&&) = delete;
    Worker& operator=(Worker&&) = delete;

    void operator()(Input&& in) {
        queue_.push(std::move(in));
    }

    void join() {
        if (!thread_.joinable())
            return;
        queue_.close();
        thread_.join();
    }

private:
    Function func_;

    ConsumeQueue<Input> queue_;
    std::thread thread_;

    void loop() noexcept {
        try {
            while (true) {
                std::optional<Input> in = queue_.pop();
                if (!in)
                    break;
                func_(std::move(*in));
            }
            log::dev("worker exited");
        }
        catch (const std::exception& ex) {
            log::error("worker terminated: {}", ex.what());
        }
    }
};

template <typename Input, typename Function>
Worker<Input, Function> make_worker(Function&& func) {
    return std::forward<Function>(func);
}


template <typename Input_, typename Function_>
struct Pipe {
    using Input = Input_;
    using Function = Function_;

    Function func;
};

template <typename Input, typename Function>
Pipe<Input, Function> make_pipe(Function&& func) {
    return {std::forward<Function>(func)};
}


template <typename Pipe, typename... NextPipes>
struct Pipeline {
    using Input = typename Pipe::Input;
    using Function = typename Pipe::Function;
    using Next = Pipeline<NextPipes...>;
    using NextInput = typename Next::Input;

    static_assert(std::is_invocable_v<Function, Input> || std::is_invocable_v<Function, Input, void (*)(NextInput&&)>);

    Pipeline(Pipe&& pipe, NextPipes&&... next) :
            func_{std::move(pipe.func)},
            next_{std::move(next)...},
            worker_{std::bind_front(&Pipeline::work, this)} {
    }

    ~Pipeline() = default;
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;

    void operator()(Input&& in) {
        worker_(std::move(in));
    }

    void join() {
        worker_.join();
        next_.join();
    }

private:
    void work(Input&& in) {
        if constexpr (std::is_invocable_v<Function, Input>) {
            static_assert(std::is_convertible_v<std::invoke_result_t<Function, Input>, NextInput>);
            next_(func_(std::move(in)));
        }
        else {
            static_assert(std::is_same_v<std::invoke_result_t<Function, Input, void (*)(NextInput &&)>, void>);
            func_(std::move(in), [&](NextInput&& next_in) { next_(std::move(next_in)); });
        }
    }

    Function func_;

    Next next_;
    Worker<Input, decltype(std::bind_front(&Pipeline::work, std::declval<Pipeline*>()))> worker_;
};


template <typename Pipe>
struct Pipeline<Pipe> : Worker<typename Pipe::Input, typename Pipe::Function> {
    using Input = typename Pipe::Input;

    Pipeline(Pipe&& pipe) :
            Worker<typename Pipe::Input, typename Pipe::Function>{std::move(pipe.func)} {
    }
};

}  // namespace komankondi::dictgen
