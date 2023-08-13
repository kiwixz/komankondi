#include <exception>
#include <filesystem>
#include <optional>
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

        std::string_view source_placeholder = "<source>";
        if (size_t i = opt.dictionary.find(source_placeholder); i != std::string::npos)
            opt.dictionary.replace(i, source_placeholder.size(), source);

        std::filesystem::path dictionary_path = opt.dictionary;
        if (dictionary_path.has_parent_path())
            std::filesystem::create_directories(dictionary_path.parent_path());

        constexpr std::string_view wiktionary_suffix = "wiktionary";
        if (source.ends_with(wiktionary_suffix)) {
            dictgen_wiktionary(std::string_view{source}.substr(0, source.length() - wiktionary_suffix.length()), opt);
        }
        else {
            throw Exception{"Could not recognize source '{}'", source};
        }
    }
    catch (const std::exception& ex) {
        log::error("{}", ex.what());
        return 1;
    }
}
