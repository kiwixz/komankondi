add_library(dict
    "reader.cpp"
    "writer.cpp"
)
add_library(komankondi::dict ALIAS dict)
target_link_libraries(dict
    komankondi::utils
)
