utils_deps = [
    cli11,
    fmt,
    range_v3,
    sqlite,
]

utils_lib = library('utils', [
        'cli.cpp',
        'database.cpp',
        'file.cpp',
        'hex.cpp',
        'iequal.cpp',
        'log.cpp',
        'path.cpp',
        'platform.cpp',
        'zstring_view.cpp',
    ],
    dependencies: utils_deps)

utils = declare_dependency(dependencies: utils_deps, link_with: utils_lib)

test('utils', executable('utils-test', [
        'test/log.cpp',
        'test/zstring_view.cpp',
    ],
    dependencies: [utils_deps, catch2],
    link_with: utils_lib))
