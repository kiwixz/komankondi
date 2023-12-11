find_package(CLI11 REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(range-v3 REQUIRED)
find_package(SQLite3 REQUIRED)

add_library(utils
    "cli.cpp"
    "config.cpp"
    "database.cpp"
    "file.cpp"
    "hasher.cpp"
    "hex.cpp"
    "iequal.cpp"
    "log.cpp"
    "path.cpp"
    "platform.cpp"
    "zstring_view.cpp"
)
add_library(komankondi::utils ALIAS utils)
target_link_libraries(utils
    CLI11::CLI11
    fmt::fmt
    OpenSSL::Crypto
    range-v3::range-v3
    SQLite::SQLite3
)

if (BUILD_TESTING)
    find_package(Catch2 REQUIRED)
    add_executable(test_utils
        "test/hasher.cpp"
        "test/log.cpp"
        "test/scope_exit.cpp"
        "test/zstring_view.cpp"
    )
    target_link_libraries(test_utils utils Catch2::Catch2WithMain)
    add_test(NAME utils COMMAND test_utils)
endif ()
