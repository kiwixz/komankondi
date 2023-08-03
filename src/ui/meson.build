ui_deps = [
    qt,
]

ui_lib = static_library('ui', [
        module_qt.compile_resources(sources: 'main.qrc'),
    ],
    dependencies: ui_deps)

executable('komankondi', 'main.cpp', dependencies: ui_deps, link_with: ui_lib, install: true)
