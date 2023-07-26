#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace komankondi::dictgen {

template <typename Input_, typename Function>
struct Worker {
    using Input = Input_;

    static_assert(std::is_invocable_v<Function, Input>);
    static_assert(std::is_same_v<std::invoke_result_t<Function, Input>, void>);

    Worker(Function func) :
            func_{std::move(func)} {
    }

    void operator()(Input&& in) {
        func_(std::move(in));
    }

private:
    Function func_;
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

    Pipeline(Pipe pipe, NextPipes... next) :
            func_{std::move(pipe.func)},
            worker_{std::bind_front(&Pipeline::work, this)},
            next_{std::move(next)...} {
    }

    void operator()(Input&& in) {
        worker_(std::move(in));
    }

private:
    void work(Input&& in) {
        if constexpr (std::is_invocable_v<Function, Input>) {
            static_assert(std::is_convertible_v<std::invoke_result_t<Function, Input>, NextInput>);
            next_(func_(std::move(in)));
        }
        else {
            static_assert(std::is_same_v<std::invoke_result_t<Function, Input, void (*)(NextInput &&)>, void>);
            func_(std::move(in), std::bind_front(&Next::operator(), &next_));
        }
    }

    Function func_;
    Worker<Input, decltype(std::bind_front(&Pipeline::work, std::declval<Pipeline*>()))> worker_;
    Next next_;
};


template <typename Pipe>
struct Pipeline<Pipe> : Worker<typename Pipe::Input, typename Pipe::Function> {
    Pipeline(Pipe pipe) :
            Worker<typename Pipe::Input, typename Pipe::Function>{std::move(pipe.func)} {
    }
};

}  // namespace komankondi::dictgen
