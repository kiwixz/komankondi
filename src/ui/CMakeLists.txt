find_package(fmt REQUIRED)
find_package(Qt5 REQUIRED Quick)
find_package(range-v3 REQUIRED)

add_library(ui_
    "resources/main.qrc"

    "context.cpp"
)
set_target_properties(ui_ PROPERTIES
    AUTOMOC ON
    AUTORCC ON
)
target_link_libraries(ui_
    fmt::fmt
    Qt5::Quick
    range-v3::range-v3

    komankondi::game
)

add_executable(ui "main.cpp")
set_target_properties(ui PROPERTIES OUTPUT_NAME "komankondi")
target_link_libraries(ui ui_)
install(TARGETS ui)
