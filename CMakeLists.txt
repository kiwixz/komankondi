cmake_minimum_required(VERSION 3.15)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/.gitignore" "*")

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Release, RelWithDebInfo, Debug or None")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "")

set(cxx_flags_base "-Wall -Wextra -Wpedantic -march=x86-64-v3")
set(cxx_flags_harden "-fstack-protector-strong -D_GLIBCXX_ASSERTIONS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3")
if (NOT WIN32)
    set(cxx_flags_sanitize "-fsanitize=address,undefined")
endif ()
set(CMAKE_CXX_FLAGS_RELEASE "${cxx_flags_base} -Ofast -flto=auto -DNDEBUG" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${cxx_flags_base} ${cxx_flags_harden} -Ofast -g3 -fno-omit-frame-pointer" CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG "${cxx_flags_base} ${cxx_flags_harden} ${cxx_flags_sanitize} -Og -g3 -fno-omit-frame-pointer" CACHE STRING "")

option(VCPKG "Install dependencies with vcpkg" ON)
if (VCPKG)
    if (WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "")
    endif ()
    set(VCPKG_BOOTSTRAP_OPTIONS "-disableMetrics" CACHE STRING "")
    set(VCPKG_INSTALL_OPTIONS "--no-print-usage" CACHE STRING "")
    set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
endif ()

project(komankondi CXX)

include(CheckCXXCompilerFlag)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)  # cmake 3.25
set(CMAKE_OPTIMIZE_DEPENDENCIES ON)  # cmake 3.19

include_directories("src")

foreach (warning IN ITEMS
    "alloc-zero"
    "cast-qual"
    "date-time"
    "duplicated-branches" "duplicated-cond" "logical-op"
    "format=2"
    "mismatched-tags" "redundant-tags"
    # "missing-declarations"
    # "null-dereference"
    "unused-macros"
)
    string(REGEX REPLACE "[^a-z0-9]" "_" var "has_warning_${warning}")
    check_cxx_compiler_flag("-W${warning}" ${var})
    if (${var})
        add_compile_options("-W${warning}")
    endif ()
endforeach ()

if (WIN32)
    add_compile_definitions("_CRT_SECURE_NO_WARNINGS" "_CRT_NONSTDC_NO_WARNINGS")
endif ()

add_subdirectory("src/utils")
add_subdirectory("src/dict")
add_subdirectory("src/game")

add_subdirectory("src/dictgen")
add_subdirectory("src/ui")

add_subdirectory("test")
