project('maketest', 'cpp',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
    ])


cflags = []

if get_option('debug')
    cflags += '-DDEBUG'
endif

if get_option('warning_level') == '3'
    cflags += meson.get_compiler('cpp').get_supported_arguments([
        '-Walloc-zero',
        '-Wcast-qual',
        '-Wdate-time',
        '-Wduplicated-branches', '-Wduplicated-cond', '-Wlogical-op',
        '-Wformat=2',
        '-Wmismatched-tags', '-Wredundant-tags',
        '-Wmissing-declarations',
        '-Wnull-dereference',
        '-Wunused-macros',
    ])
endif

add_project_dependencies(declare_dependency(
        compile_args: cflags,
        include_directories: ['src'],
    ), language: 'cpp')


cli11 = dependency('CLI11')
fmt = dependency('fmt')

subdir('src/dictgen')
