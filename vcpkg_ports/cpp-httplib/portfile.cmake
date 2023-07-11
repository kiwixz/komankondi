vcpkg_from_github(OUT_SOURCE_PATH SOURCE_PATH
    REPO "yhirose/cpp-httplib"
    REF "v0.13.1"
    SHA512 "4a70ebafd0920116a78ea18982606f0bec396e5cdcea9ba583c1da4fd77fa45c5bf30a6ac14eeee9424f3e445a882a560345d731a7113ab4e7dff88f4ef0a436"
)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DHTTPLIB_COMPILE=ON
        -DHTTPLIB_REQUIRE_OPENSSL=ON
        -DHTTPLIB_USE_ZLIB_IF_AVAILABLE=OFF
        -DHTTPLIB_USE_BROTLI_IF_AVAILABLE=OFF
        -DCMAKE_CXX_FLAGS="-DCPPHTTPLIB_NO_DEFAULT_USER_AGENT"

        # workaround meson evaluating empty BOOL cmake generator expression to true
        -DHTTPLIB_IS_USING_ZLIB=OFF
        -DHTTPLIB_IS_USING_BROTLI=OFF
)
vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME "httplib" CONFIG_PATH "lib/cmake/httplib")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME "copyright")
