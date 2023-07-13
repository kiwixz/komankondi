dictgen = static_library('dictgen', [
        'main.cpp',
        'wiktionary.cpp',
    ],
    link_with: [utils],
    dependencies: [fmt, httplib, pegtl, range_v3])

executable('dictgen', '../main.cpp', link_with: dictgen)

test('dictgen', executable('dictgen-test', [
        'test/xml.cpp',
    ],
    link_with: dictgen,
    dependencies: catch2))
