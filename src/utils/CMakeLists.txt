find_package(CLI11 REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(range-v3 REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(strong_type REQUIRED)

file(GLOB_RECURSE src "*.cpp")
add_library(utils ${src})
add_library(komankondi::utils ALIAS utils)
target_link_libraries(utils PUBLIC
    CLI11::CLI11
    fmt::fmt
    OpenSSL::Crypto
    range-v3::range-v3
    SQLite::SQLite3
    strong_type::strong_type
)
