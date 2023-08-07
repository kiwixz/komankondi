dict_deps = [
    utils,

    sqlite,
]

dict_lib = library('dict', [
        'database.cpp',
        'reader.cpp',
        'writer.cpp',
    ],
    dependencies: dict_deps)

dict = declare_dependency(dependencies: dict_deps, link_with: dict_lib)
