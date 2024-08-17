vcpkg_from_github(OUT_SOURCE_PATH SOURCE_PATH
    REPO "yhirose/cpp-httplib"
    REF "v${VERSION}"
    SHA512 "1a0d40f17b526db74dfa51903f0d15876b20c836a438be8f87ac2b18e535c3fad1822fbad9cf97053705a2cedc8171ab648e2e8c823eeb2e180c347283f3de9a"
)

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DHTTPLIB_COMPILE=ON
        -DHTTPLIB_REQUIRE_OPENSSL=ON
        -DHTTPLIB_USE_ZLIB_IF_AVAILABLE=OFF
        -DHTTPLIB_USE_BROTLI_IF_AVAILABLE=OFF
        -DCMAKE_CXX_FLAGS="-DCPPHTTPLIB_NO_DEFAULT_USER_AGENT"
)
vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME "httplib" CONFIG_PATH "lib/cmake/httplib")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
