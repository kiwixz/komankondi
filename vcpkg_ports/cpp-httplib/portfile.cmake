vcpkg_from_github(OUT_SOURCE_PATH SOURCE_PATH
    REPO "yhirose/cpp-httplib"
    REF "v${VERSION}"
    SHA512 "b4f315e174f8efb7884b64b45c500c8259c28379a6079c26747f754db7e1f16a118b1e6f83925b6740a1b5b3516158c1202737dc6385bcefe9c69f4cca57d07e"
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
