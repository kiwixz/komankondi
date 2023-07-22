#include "cli.hpp"

#include <string>
#include <string_view>

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

#include "utils/log.hpp"

namespace komankondi {
namespace {

struct CliFormatter : CLI::Formatter {
    std::string make_option_desc(const CLI::Option* opt) const override {
        if (opt->get_required())
            return fmt::format("{} (required)", opt->get_description());
        if (!opt->get_default_str().empty())
            return fmt::format("{} (default: {})", opt->get_description(), opt->get_default_str());
        return opt->get_description();
    }

    std::string make_option_name(const CLI::Option* opt, bool positional) const override {
        if (positional)
            return opt->get_name(true);

        auto short_names = opt->get_snames() | ranges::views::transform([](std::string_view s) { return ranges::views::concat(std::string_view{"-"}, s); });
        auto long_names = opt->get_lnames() | ranges::views::transform([](std::string_view s) { return ranges::views::concat(std::string_view{"--"}, s); });
        return ranges::views::concat(short_names, long_names)
               | ranges::views::join(std::string_view{", "})
               | ranges::to<std::string>;
    }

    std::string make_option_opts(const CLI::Option* opt) const override {
        if (!opt->get_option_text().empty())
            return fmt::format(" {}", opt->get_option_text());
        if (opt->get_type_size() == 0)
            return "";
        if (opt->get_expected() <= 1)
            return fmt::format(" {}", opt->get_type_name());
        if (opt->get_expected_max() == opt->get_expected_min())
            return fmt::format(" {}x{}", opt->get_type_name(), opt->get_expected());
        return fmt::format(" {}...", opt->get_type_name(), opt->get_expected());
    }
};

}  // namespace


Cli::Cli() {
    app_.formatter(std::make_shared<CliFormatter>());
}

std::optional<bool> Cli::parse(int argc, char** argv) {
    try {
        app_.parse(argc, argv);
        return {};
    }
    catch (const CLI::CallForHelp& ex) {
        fmt::print("{}", app_.help());
        return true;
    }
    catch (const CLI::CallForAllHelp& ex) {
        fmt::print("{}", app_.help("", CLI::AppFormatMode::All));
        return true;
    }
    catch (const CLI::CallForVersion& ex) {
        fmt::print("{}", app_.version());
        return true;
    }
    catch (const CLI::ParseError& ex) {
        log::error("bad options: {}", ex.what());
        return false;
    };
}

std::string Cli::format_default(bool variable) {
    return variable ? "on" : "off";
}

}  // namespace komankondi
