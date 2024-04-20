#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <fmt/core.h>

#include "dictgen/wiktionary.hpp"
#include "utils/cli.hpp"
#include "utils/log.hpp"
#include "utils/path.hpp"
#include "utils/signal.hpp"

int main(int argc, char** argv) {
    using namespace komankondi;
    using namespace dictgen;

    try {
        catch_termination_signal();

        Cli cli;
        bool cache = true;
        cli.add_flag("--cache,!--no-cache", cache, "Cache downloaded data");
        std::string dictionary = fmt::format("{}/<language>.dict", get_data_directory());
        cli.add_option("-o,--dictionary", dictionary, "Path to the dictionary");

        std::string language;
        cli.add_option("language", language, "Language of the dictionary to extract from Wiktionary")->required();

        if (std::optional<bool> ok = cli.parse(argc, argv); ok)
            return !*ok;

        LanguageSpec language_spec = find_language_spec(language);

        std::string_view language_placeholder = "<language>";
        if (size_t i = dictionary.find(language_placeholder); i != std::string::npos)
            dictionary.replace(i, language_placeholder.size(), language_spec.name);

        std::filesystem::path dictionary_path = dictionary;
        if (dictionary_path.has_parent_path())
            std::filesystem::create_directories(dictionary_path.parent_path());

        generate_dictionary(dictionary, language_spec, cache);
    }
    catch (const std::exception& ex) {
        log::error("{}", ex.what());
        return 1;
    }
}
