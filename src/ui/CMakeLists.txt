find_package(fmt REQUIRED)
find_package(Qt6 REQUIRED Quick)
find_package(range-v3 REQUIRED)

file(GLOB_RECURSE src "*.cpp")
list(REMOVE_ITEM src "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
add_library(ui_ STATIC ${src})
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

add_executable(ui WIN32 "main.cpp")
set_target_properties(ui PROPERTIES OUTPUT_NAME "komankondi")
target_link_libraries(ui PRIVATE ui_ ui_plugin)
install(TARGETS ui)
