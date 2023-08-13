#pragma once

#include <span>

#include <fmt/core.h>

namespace komankondi::log {

enum class Level {
    trace,
    debug,
    info,
    status,
    warning,
    error,
    dev,
};


Level verbosity();
void set_verbosity(Level level);


bool vlog(Level level, fmt::string_view fmt, const fmt::format_args& args);

template <typename... Args>
bool log(Level level, fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(level, fmt, fmt::make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
bool trace(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::trace, fmt, fmt::make_format_args(std::forward<Args>(args)...));
}
template <typename... Args>
bool debug(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::debug, fmt, fmt::make_format_args(std::forward<Args>(args)...));
}
template <typename... Args>
bool info(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::info, fmt, fmt::make_format_args(std::forward<Args>(args)...));
}
template <typename... Args>
bool status(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::status, fmt, fmt::make_format_args(std::forward<Args>(args)...));
}
template <typename... Args>
bool warn(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::warning, fmt, fmt::make_format_args(std::forward<Args>(args)...));
}
template <typename... Args>
bool error(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::error, fmt, fmt::make_format_args(std::forward<Args>(args)...));
}
template <typename... Args>
bool dev(fmt::format_string<Args...> fmt, Args&&... args) {
    return vlog(Level::dev, fmt, fmt::make_format_args(std::forward<Args>(args)...));
}


struct Hexdump {
    std::span<const std::byte> data;

    template <typename T>
    Hexdump(std::span<const T> data_) :
            data{std::as_bytes(data_)} {
    }
    template <typename T>
    Hexdump(const T* data_, size_t size) :
            data{std::as_bytes(std::span<const T>{data_, size})} {
    }
};

}  // namespace komankondi::log


template <>
struct fmt::formatter<komankondi::log::Hexdump> {
    const char* parse(const format_parse_context& ctx);
    fmt::appender format(const komankondi::log::Hexdump& a, format_context& ctx);
};
