dictgen_deps = [
    dict,
    utils,

    bzip2,
    cli11,
    fmt,
    httplib,
    openssl,
    pegtl,
    range_v3,
    tbb,
]

dictgen_lib = static_library('dictgen', [
        'bzip.cpp',
        'cache.cpp',
        'downloader.cpp',
        'hasher.cpp',
        'wiktionary.cpp',
        'xml.cpp',
    ],
    dependencies: dictgen_deps)

executable('komankondi-dictgen', 'main.cpp', dependencies: dictgen_deps, link_with: dictgen_lib, install: true)

test('dictgen', executable('dictgen-test', [
        'test/xml.cpp',
    ],
    dependencies: [dictgen_deps, catch2],
    link_with: dictgen_lib))
