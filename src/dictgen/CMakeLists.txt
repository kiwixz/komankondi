find_package(BZip2 REQUIRED)
find_package(fmt REQUIRED)
find_package(httplib REQUIRED)
find_package(pegtl REQUIRED)
find_package(range-v3 REQUIRED)
find_package(TBB REQUIRED)

add_library(dictgen_
    "wikitext/extract.cpp"

    "bzip.cpp"
    "cache.cpp"
    "downloader.cpp"
    "wiktionary.cpp"
    "xml.cpp"
)
target_link_libraries(dictgen_
    BZip2::BZip2
    fmt::fmt
    httplib::httplib
    taocpp::pegtl
    range-v3::range-v3
    TBB::tbb

    komankondi::dict
)

add_executable(dictgen "main.cpp")
set_target_properties(dictgen PROPERTIES OUTPUT_NAME "komankondi-dictgen")
target_link_libraries(dictgen dictgen_)
install(TARGETS dictgen)

if (BUILD_TESTING)
    find_package(Catch2 REQUIRED)
    add_executable(test_dictgen
        "test/wikitext.cpp"
        "test/xml.cpp"
    )
    target_link_libraries(test_dictgen dictgen_ Catch2::Catch2WithMain)
    add_test(NAME dictgen COMMAND test_dictgen)
endif ()
