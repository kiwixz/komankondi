utils_deps = [
    cli11,
    fmt,
]

utils_lib = library('utils', [
        'cli.cpp',
        'log.cpp',
    ],
    dependencies: utils_deps)

utils = declare_dependency(dependencies: utils_deps, link_with: utils_lib)
