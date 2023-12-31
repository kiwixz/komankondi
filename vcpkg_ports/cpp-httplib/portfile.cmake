vcpkg_from_github(OUT_SOURCE_PATH SOURCE_PATH
    REPO "yhirose/cpp-httplib"
    REF "v${VERSION}"
    SHA512 "0e7955fc74b87550e260739abf2503b2b0aabb2e2925953956bef8ead9718367d075d37fb5468a40aa340d7bdafb06274e0770baab86b08c6a25020d96033b88"
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
