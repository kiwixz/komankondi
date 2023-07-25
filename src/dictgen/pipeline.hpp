#pragma once

#include <type_traits>
#include <utility>

#include "utils/traits.hpp"

namespace komankondi::dictgen {

template <typename Input, typename Step, typename... NextSteps>
struct Pipe {
    static_assert(function_nr_args<Step> == 1);
    static_assert(std::is_convertible_v<Input, FunctionArg<Step, 0>>);

    Pipe(Step step, NextSteps... next_steps) :
            step_{std::move(step)},
            next_{std::move(next_steps)...} {
    }

    void operator()(Input input) {
        next_.operator()(step_(std::forward<Input>(input)));
    }

private:
    Step step_;
    Pipe<FunctionReturn<Step>, NextSteps...> next_;
};


template <typename Input, typename Step>
struct Pipe<Input, Step> {
    static_assert(function_nr_args<Step> == 1);
    static_assert(std::is_convertible_v<Input, FunctionArg<Step, 0>>);
    static_assert(std::is_same_v<FunctionReturn<Step>, void>);

    Pipe(Step step) :
            step_{std::move(step)} {
    }

    void operator()(Input input) {
        step_(std::forward<Input>(input));
    }

private:
    Step step_;
};


template <typename Input, typename... Steps>
Pipe<Input, Steps...> make_pipeline(Steps&&... steps) {
    return {std::forward<Steps>(steps)...};
}

}  // namespace komankondi::dictgen
