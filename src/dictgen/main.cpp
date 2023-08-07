#include <exception>
#include <string>
#include <string_view>

#include <fmt/core.h>

#include "dictgen/options.hpp"
#include "dictgen/wiktionary.hpp"
#include "utils/cli.hpp"
#include "utils/exception.hpp"
#include "utils/log.hpp"

int main(int argc, char** argv) {
    using namespace komankondi;
    using namespace dictgen;

    try {
        Options opt;
        Cli cli;
        cli.add_flag("--cache,!--no-cache", opt.cache, "Cache downloaded data");
        cli.add_option("-o,--dictionary", opt.dictionary, "Path to the dictionary");

        std::string source;
        cli.add_option("source", source, "Where to extract the dictionary from")->required();

        if (std::optional<bool> ok = cli.parse(argc, argv); ok)
            return !*ok;

        if (opt.dictionary == "<source>.dict")
            opt.dictionary = fmt::format("{}.dict", source);

        constexpr std::string_view wiktionary_suffix = "wiktionary";
        if (source.ends_with(wiktionary_suffix)) {
            dictgen_wiktionary(std::string_view{source}.substr(0, source.length() - wiktionary_suffix.length()), opt);
        }
        else {
            throw Exception{"could not recognize source '{}'", source};
        }
    }
    catch (const std::exception& ex) {
        log::error("{}", ex.what());
        return 1;
    }
}
