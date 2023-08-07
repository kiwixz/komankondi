dict_deps = [
    utils,
]

dict_lib = library('dict', [
        'reader.cpp',
        'writer.cpp',
    ],
    dependencies: dict_deps)

dict = declare_dependency(dependencies: dict_deps, link_with: dict_lib)
