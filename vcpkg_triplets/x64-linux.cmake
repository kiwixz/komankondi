set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CXX_FLAGS "-ffile-prefix-map=${CURRENT_BUILDTREES_DIR}/=")
set(VCPKG_C_FLAGS "${VCPKG_CXX_FLAGS}")

set(VCPKG_CMAKE_SYSTEM_NAME Linux)
