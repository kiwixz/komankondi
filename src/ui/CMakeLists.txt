find_package(fmt REQUIRED)
find_package(Qt6 REQUIRED Quick)
find_package(range-v3 REQUIRED)

add_library(ui_ STATIC
    "context.cpp"
    "context.hpp"
)
set_target_properties(ui_ PROPERTIES AUTOMOC ON)
target_link_libraries(ui_ PUBLIC
    fmt::fmt
    Qt6::Quick
    range-v3::range-v3

    komankondi::game
)

qt_add_qml_module(ui_
    URI komankondi
    VERSION 1.0
    RESOURCE_PREFIX "qt/qml"
    OUTPUT_DIRECTORY "komankondi"
    QML_FILES
        "main.qml"
)

add_executable(ui "main.cpp")
set_target_properties(ui PROPERTIES OUTPUT_NAME "komankondi")
target_link_libraries(ui ui_ ui_plugin)
install(TARGETS ui)
