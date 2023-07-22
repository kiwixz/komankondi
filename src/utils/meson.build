utils_deps = [
    fmt,
]

utils_lib = library('utils', [
        'log.cpp',
    ],
    dependencies: utils_deps)

utils = declare_dependency(dependencies: utils_deps, link_with: utils_lib)
