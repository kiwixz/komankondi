project('komankondi', 'cpp',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
    ])


cflags = []

if get_option('debug')
    cflags += '-DDEBUG'
endif

if build_machine.system() == 'windows'
    cflags += ['-D_CRT_SECURE_NO_WARNINGS', '-D_CRT_NONSTDC_NO_WARNINGS']
endif

if get_option('warning_level') == '3' and get_option('extra_warnings')
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

if meson.version().version_compare('>=0.63.0')
    add_project_dependencies(declare_dependency(
            compile_args: cflags,
            include_directories: ['src'],
        ), language: 'cpp')
else
    add_project_arguments(cflags, '-I' + meson.project_source_root() / 'src', language: 'cpp')
endif


module_qt = import('qt5')

bzip2 = dependency('BZip2', include_type: 'system')
catch2 = dependency('Catch2', modules: 'Catch2::Catch2WithMain', include_type: 'system')
cli11 = dependency('CLI11', include_type: 'system')
fmt = dependency('fmt', include_type: 'system')
httplib = dependency('httplib', include_type: 'system')
openssl = dependency('openssl', include_type: 'system')
pegtl = dependency('pegtl', modules: 'taocpp::pegtl', include_type: 'system')
qt = dependency('qt5', modules: ['Core', 'Gui', 'Qml', 'Quick'], include_type: 'system')
range_v3 = dependency('range-v3', include_type: 'system')
sqlite = dependency('sqlite3', 'unofficial-sqlite3', modules: 'unofficial::sqlite3::sqlite3', include_type: 'system')


subdir('src/utils')
subdir('src/dict')
subdir('src/game')

subdir('src/dictgen')
subdir('src/ui')
