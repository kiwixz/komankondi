find_package(Boost REQUIRED json regex)
find_package(fmt REQUIRED)
find_package(httplib REQUIRED)
find_package(range-v3 REQUIRED)
find_package(TBB REQUIRED)
find_package(ZLIB REQUIRED)

add_library(dictgen_
    "cache.cpp"
    "downloader.cpp"
    "gzip.cpp"
    "tarcat.cpp"
    "wiktionary.cpp"
)
target_link_libraries(dictgen_
    Boost::json Boost::regex
    fmt::fmt
    httplib::httplib
    range-v3::range-v3
    TBB::tbb
    ZLIB::ZLIB

    komankondi::dict
)

add_executable(dictgen "main.cpp")
set_target_properties(dictgen PROPERTIES OUTPUT_NAME "komankondi-dictgen")
target_link_libraries(dictgen dictgen_)
install(TARGETS dictgen)
