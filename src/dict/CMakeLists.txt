file(GLOB_RECURSE src "*.cpp")
add_library(dict ${src})
add_library(komankondi::dict ALIAS dict)
target_link_libraries(dict PUBLIC
    komankondi::utils
)
