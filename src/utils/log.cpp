#include "log.hpp"

#include <cstdio>

#include <fmt/color.h>
#include <fmt/core.h>

namespace komankondi::log {
namespace {

void format_level(fmt::appender out, Level level) {
    switch (level) {
    case Level::trace: fmt::format_to(out, fmt::fg(fmt::terminal_color::cyan), "TRACE: "); break;
    case Level::debug: fmt::format_to(out, fmt::fg(fmt::terminal_color::bright_blue), "DEBUG: "); break;
    case Level::info: break;
    case Level::warn: fmt::format_to(out, fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_yellow), "WARNING: "); break;
    case Level::error: fmt::format_to(out, fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_red), "ERROR: "); break;
    case Level::dev: fmt::format_to(out, fmt::emphasis::bold | fmt::fg(fmt::terminal_color::bright_magenta), "DEV: "); break;
    }
}

Level& verbosity_mutable() {
    static Level r = Level::info;
    return r;
}

}  // namespace


Level verbosity() {
    return verbosity_mutable();
}

void set_verbosity(Level level) {
    verbosity_mutable() = level;
}


bool vlog(Level level, fmt::string_view fmt, const fmt::format_args& args) {
    if (level < verbosity())
        return false;

    fmt::memory_buffer buf;
    format_level(fmt::appender{buf}, level);
    fmt::vformat_to(fmt::appender{buf}, fmt, args);
    buf.push_back('\n');
    (void)std::fwrite(buf.data(), 1, buf.size(), stderr);
    return true;
}

}  // namespace komankondi::log


const char* fmt::formatter<komankondi::log::Hexdump>::parse(const format_parse_context& ctx) {
    if (ctx.begin() != ctx.end() && *ctx.begin() != '}')
        throw format_error{"invalid hexdump format specification"};
    return ctx.begin();
}

fmt::appender fmt::formatter<komankondi::log::Hexdump>::format(const komankondi::log::Hexdump& a, format_context& ctx) {
    constexpr size_t bytes_per_line = 16;

    for (size_t offset = 0; offset < a.data.size(); offset += bytes_per_line) {
        std::span<const std::byte> line = a.data.subspan(offset, std::min(bytes_per_line, a.data.size() - offset));
        fmt::format_to(ctx.out(), "  {:08x}  ", offset);

        int nr_bytes = 0;
        for (std::byte c : line) {
            fmt::format_to(ctx.out(), "{:02x}", c);
            if (nr_bytes % 2 == 1)
                *ctx.out() = ' ';
            ++nr_bytes;
        }

        size_t nr_bytes_padding = bytes_per_line - nr_bytes;
        for (size_t i = 0; i < nr_bytes_padding / 2 * 5 + nr_bytes_padding % 2 * 3; ++i) {
            *ctx.out() = ' ';
        }

        *ctx.out() = ' ';
        for (std::byte c : line) {
            *ctx.out() = std::isprint(static_cast<unsigned char>(c)) ? static_cast<char>(c) : '.';
        }

        *ctx.out() = '\n';
    }

    return ctx.out();
}
