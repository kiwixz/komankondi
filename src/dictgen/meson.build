dictgen_deps = [
    utils,

    cli11,
    fmt,
    httplib,
    pegtl,
    range_v3,
]

dictgen_lib = static_library('dictgen', [
        'hasher.cpp',
        'wiktionary.cpp',
    ],
    dependencies: dictgen_deps)

executable('dictgen', 'main.cpp', dependencies: dictgen_deps, link_with: dictgen_lib)

test('dictgen', executable('dictgen-test', [
        'test/pipeline.cpp',
        'test/xml.cpp',
    ],
    dependencies: [dictgen_deps, catch2],
    link_with: dictgen_lib))
