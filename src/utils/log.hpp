#pragma once

#include <span>

#include <fmt/core.h>

namespace komankondi::log {

enum class Level {
    trace,
    debug,
    info,
    warn,
    error,
    dev,
};


Level verbosity();
void set_verbosity(Level level);


bool vlog(Level level, fmt::string_view fmt, const fmt::format_args& args);

template <typename... Args>
bool log(Level level, fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(level, fmt, fmt::make_format_args(args...));
}

template <typename... Args>
bool trace(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::trace, fmt, fmt::make_format_args(args...));
}
template <typename... Args>
bool debug(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::debug, fmt, fmt::make_format_args(args...));
}
template <typename... Args>
bool info(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::info, fmt, fmt::make_format_args(args...));
}
template <typename... Args>
bool warn(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::warn, fmt, fmt::make_format_args(args...));
}
template <typename... Args>
bool error(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::error, fmt, fmt::make_format_args(args...));
}
template <typename... Args>
bool dev(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::dev, fmt, fmt::make_format_args(args...));
}


struct Hexdump {
    std::span<const std::byte> data;

    template <typename T>
    Hexdump(std::span<T> _data) :
            data{std::as_bytes(_data)} {
    }
    template <typename T>
    Hexdump(const T* _data, size_t size) :
            data{std::as_bytes(std::span<const T>{_data, size})} {
    }
};

}  // namespace komankondi::log


template <>
struct fmt::formatter<komankondi::log::Hexdump> {
    const char* parse(const format_parse_context& ctx);
    fmt::appender format(const komankondi::log::Hexdump& a, format_context& ctx);
};
