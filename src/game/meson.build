game_deps = [
    dict,
    utils,
]

game_lib = library('game', [
        'game.cpp',
        'profile.cpp',
    ],
    dependencies: game_deps)

game = declare_dependency(dependencies: game_deps, link_with: game_lib)
