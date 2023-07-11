#include <string>
#include <string_view>

#include <CLI/CLI.hpp>
#include <fmt/core.h>

#include "dictgen/wiktionary.hpp"

int main(int argc, char** argv) {
    using namespace komankondi::dictgen;

    CLI::App app;

    bool no_cache = false;
    app.add_flag("--no-cache", no_cache, "Do not cache downloaded data");

    std::string source;
    app.add_option("source", source)->required();

    CLI11_PARSE(app, argc, argv);

    constexpr std::string_view wiktionary_suffix = "wiktionary";
    if (source.ends_with(wiktionary_suffix)) {
        dictgen_wiktionary(std::string_view{source}.substr(0, source.length() - wiktionary_suffix.length()));
    }
    else {
        fmt::println(stderr, "could not recognize source '{}'", source);
        return 1;
    }
}
