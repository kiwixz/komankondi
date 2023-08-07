#pragma once

#include <memory>
#include <string_view>
#include <type_traits>

#include <range/v3/iterator/operations.hpp>
#include <range/v3/view/split.hpp>
#include <sqlite3.h>

#include "utils/always_false.hpp"
#include "utils/exception.hpp"
#include "utils/function_traits.hpp"
#include "utils/zstring_view.hpp"

template <>
struct std::default_delete<sqlite3> {
    void operator()(sqlite3* ptr) const;
};

template <>
struct std::default_delete<sqlite3_stmt> {
    void operator()(sqlite3_stmt* ptr) const;
};


namespace komankondi {

struct Database {
    struct OperationAny {
        OperationAny() = default;
        OperationAny(std::unique_ptr<sqlite3_stmt>&& op);

        explicit operator bool() const {
            return static_cast<bool>(handle_);
        }

        template <typename Result = void, typename... Args>
        Result exec(const Args&... args) {
            reset();
            bind(1, args...);
            if constexpr (std::is_same_v<Result, void>) {
                if (step())
                    throw Exception{"operation returned a row, expected none"};
            }
            else {
                if (!step())
                    throw Exception{"operation did not return a row, expected one"};
                Result r = fetch<Result>();
                if (step())
                    throw Exception{"operation returned more than one row, expected only one"};
                return r;
            }
        }

        template <typename Visitor, typename... Args>
        void exec(Visitor&& visitor, const Args&... args) {
            static_assert(std::is_same_v<decltype(std::apply(visitor, std::declval<FunctionArgsTuple<Visitor>>())), void>);

            reset();
            bind(1, args...);
            while (step()) {
                std::apply(visitor, fetch<FunctionArgsTuple<Visitor>>());
            }
        }

    private:
        std::unique_ptr<sqlite3_stmt> handle_;

        void reset();
        bool step();

        void bind(int /*index*/) {
        }
        template <typename T>
        void bind(int index, const T& value) {
            if ([&] {
                    if constexpr (std::is_integral_v<T>) {
                        return sqlite3_bind_int64(handle_.get(), index, value);
                    }
                    else if constexpr (std::is_floating_point_v<T>) {
                        return sqlite3_bind_double(handle_.get(), index, value);
                    }
                    else if constexpr (std::is_same_v<T, std::string_view>) {
                        return sqlite3_bind_text(handle_.get(), index, value.data(), value.size(), SQLITE_STATIC);
                    }
                    else {
                        static_assert(always_false<T>);
                    }
                }()
                != SQLITE_OK)
            {
                throw Exception{"could not bind argument to database operation: {}", sqlite3_errmsg(sqlite3_db_handle(handle_.get()))};
            }
        }
        template <typename T, typename... Args>
        void bind(int first_index, const T& value, const Args&... next) {
            bind(first_index, value);
            bind(first_index + 1, next...);
        }

        template <typename T>
        void fetch(int index, T& value) {
            if constexpr (std::is_integral_v<T>) {
                value = sqlite3_column_int64(handle_.get(), index);
            }
            else if constexpr (std::is_floating_point_v<T>) {
                value = sqlite3_column_double(handle_.get(), index);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                const char* ptr = reinterpret_cast<const char*>(sqlite3_column_text(handle_.get(), index));
                int size = sqlite3_column_bytes(handle_.get(), index);
                if (!ptr && size > 0)
                    throw Exception{"could not fetch string from database operation: {}", sqlite3_errmsg(sqlite3_db_handle(handle_.get()))};
                value = {ptr, static_cast<size_t>(size)};
            }
            else {
                static_assert(always_false<T>);
            }
        }
        template <typename T, typename... Args>
        void fetch(int first_index, T& value, Args&... next) {
            fetch(first_index, value);
            fetch(first_index + 1, next...);
        }
        template <typename T>
        T fetch() {
            T r;
            std::apply([&](auto&... a) { fetch(0, a...); }, r);
            return r;
        }
    };

    template <typename Result = void, typename... Args>
    struct Operation {
        Operation() = default;
        Operation(OperationAny&& op) :
                op_{std::move(op)} {
        }

        explicit operator bool() const {
            return static_cast<bool>(op_);
        }

        Result exec(const Args&... args) {
            return op_.exec<Result>(args...);
        }

        template <typename Visitor>
        void exec(Visitor&& visitor, const Args&... args) {
            static_assert(std::is_same_v<decltype(std::apply(visitor, std::declval<Result>())), void>);

            return op_.exec(std::forward<Visitor>(visitor), args...);
        }

    private:
        OperationAny op_;
    };

    Database(ZStringView path, bool read_only);

    template <typename Result = void, typename... Args>
    Operation<Result, Args...> prepare(std::string_view query, bool persistent = true) {
        return prepare_any(query, persistent);
    }

    template <typename Result = void, typename... Args>
    Result exec(std::string_view query, const Args&... args) {
        if (!std::is_same_v<Result, void>)
            return prepare<Result, Args...>(query, false).exec(args...);

        for (const auto& subquery_view : query | ranges::views::split(';')) {
            std::string_view subquery{&*subquery_view.begin(), static_cast<size_t>(ranges::distance(subquery_view))};
            prepare<void, Args...>(subquery, false).exec(args...);
        }
    }

    template <typename Visitor, typename... Args>
    void exec(std::string_view query, Visitor&& visitor, const Args&... args) {
        static_assert(std::is_same_v<decltype(std::apply(visitor, std::declval<FunctionArgsTuple<Visitor>>())), void>);

        return prepare<FunctionArgsTuple<Visitor>, Args...>(query, false).exec(std::forward<Visitor>(visitor), args...);
    }

private:
    std::unique_ptr<sqlite3> handle_;

    OperationAny prepare_any(std::string_view query, bool persistent);
};

}  // namespace komankondi
