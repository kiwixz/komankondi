#pragma once

#include <type_traits>
#include <utility>

#include "utils/traits.hpp"

namespace komankondi::dictgen {

template <typename Step, typename... NextSteps>
struct Pipeline {
    using Input = FunctionArg<Step, 0>;
    using NextPipe = Pipeline<NextSteps...>;

    static_assert(std::is_same_v<FunctionReturn<Step>, typename NextPipe::Input>);
    static_assert(function_nr_args<Step> == 1);

    Pipeline(Step step, NextSteps... next_steps) :
            step_{std::move(step)},
            next_{std::move(next_steps)...} {
    }

    void in(Input&& input) {
        next_.in(step_(std::forward<Input>(input)));
    }

private:
    Step step_;
    NextPipe next_;
};


template <typename Step>
struct Pipeline<Step> {
    using Input = FunctionArg<Step, 0>;

    static_assert(std::is_same_v<FunctionReturn<Step>, void>);
    static_assert(function_nr_args<Step> == 1);

    Pipeline(Step step) :
            step_{std::move(step)} {
    }

    void in(Input&& input) {
        step_(std::forward<Input>(input));
    }

private:
    Step step_;
};

}  // namespace komankondi
