#pragma once

#include <optional>
#include <string>

#include <CLI/CLI.hpp>
#include <fmt/format.h>

namespace komankondi {

struct Cli {
    Cli();

    std::optional<bool> parse(int argc, char** argv);

    template <typename T>
    CLI::Option* add_flag(std::string names, T& variable, std::string description) {
        return app_.add_flag(std::move(names), variable, std::move(description))->default_str(format_default(variable));
    }

    template <typename T>
    CLI::Option* add_option(std::string names, T& variable, std::string description) {
        return app_.add_option_no_stream(std::move(names), variable, std::move(description))->default_str(format_default(variable));
    }

private:
    CLI::App app_;

    template <typename T>
    std::string format_default(const T& variable) {
        return fmt::to_string(variable);
    }

    std::string format_default(bool variable);
};

}  // namespace komankondi
