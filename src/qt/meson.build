qt_deps = [
    qt,
]

qt_lib = static_library('qt', [
        module_qt.compile_resources(sources: 'main.qrc'),
    ],
    dependencies: qt_deps)

executable('qt', 'main.cpp', dependencies: qt_deps, link_with: qt_lib)

# test('qt', executable('qt-test', [
#     ],
#     dependencies: [qt_deps, catch2],
#     link_with: qt_lib))
