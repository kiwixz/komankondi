#pragma once

#include <tuple>
#include <utility>

namespace komankondi {

template <int n, typename T, typename... Tail>
struct NthElement_ {
    using Type = typename NthElement_<n - 1, Tail...>::Type;
};

template <typename T, typename... Tail>
struct NthElement_<0, T, Tail...> {
    using Type = T;
};

template <int n, typename... T>
using NthElement = NthElement_<n, T...>;


template <typename T>
struct FunctionTraits : FunctionTraits<decltype(&T::operator())> {};

template <typename R, typename T, typename... Args>
struct FunctionTraits<R (T::*)(Args...) const> : FunctionTraits<R (T::*)(Args...)> {};

template <typename R, typename T, typename... Args>
struct FunctionTraits<R (T::*)(Args...)> : FunctionTraits<R(Args...)> {
    using Parent = T;
};

template <typename R, typename... Args>
struct FunctionTraits<R (*)(Args...)> : FunctionTraits<R(Args...)> {};

template <typename R, typename... Args>
struct FunctionTraits<R(Args...)> : FunctionTraits<R()> {
    using Return = R;

    static constexpr int nr_args = sizeof...(Args);

    template <int n>
    using Arg = typename NthElement<n, Args...>::Type;

    using ArgsTuple = std::tuple<Args...>;
};

template <typename T>
using FunctionReturn = typename FunctionTraits<T>::Return;

template <typename T>
static constexpr int function_nr_args = FunctionTraits<T>::nr_args;

template <typename T, int n>
using FunctionArg = typename FunctionTraits<T>::template Arg<n>;

template <typename T>
using FunctionArgsTuple = typename FunctionTraits<T>::ArgsTuple;

}  // namespace komankondi
