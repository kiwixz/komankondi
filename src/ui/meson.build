ui_deps = [
    utils,

    qt,
    range_v3,
]

ui_lib = static_library('ui', [
        module_qt.compile_moc(headers: 'context.hpp'),
        module_qt.compile_resources(sources: 'resources/main.qrc'),
    ],
    dependencies: ui_deps)

executable('komankondi', 'main.cpp', dependencies: ui_deps, link_with: ui_lib, install: true)
