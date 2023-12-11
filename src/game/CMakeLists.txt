find_package(fmt REQUIRED)

add_library(game
    "game.cpp"
    "profile.cpp"
)
add_library(komankondi::game ALIAS game)
target_link_libraries(game
    fmt::fmt

    komankondi::dict
    komankondi::utils
)
