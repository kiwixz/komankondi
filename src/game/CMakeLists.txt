find_package(fmt REQUIRED)

file(GLOB_RECURSE src "*.cpp")
add_library(game ${src})
add_library(komankondi::game ALIAS game)
target_link_libraries(game PUBLIC
    fmt::fmt

    komankondi::dict
    komankondi::utils
)
