executable('dictgen', [
        'main.cpp',
        'wiktionary.cpp',
    ],
    link_with: [utils],
    dependencies: [cli11, fmt, httplib, pegtl])

test('dictgen', executable('dictgen-test', [
        'test/xml.cpp',
    ],
    dependencies: [catch2]))
