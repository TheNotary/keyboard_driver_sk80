# Overlay port template.
#
# The @REPO@ / @REF@ / @SHA512@ placeholders are substituted by
# .github/workflows/release.yaml when a tag is published; the rendered result
# is attached to the GitHub release as blink-vcpkg-port.tar.gz.
#
# To try an unreleased revision locally, replace vcpkg_from_github below with a
# SOURCE_PATH pointing at your working copy.

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO @REPO@
    REF "@REF@"
    SHA512 @SHA512@
    HEAD_REF main
)

# vcpkg builds one linkage at a time; the project defaults to building both.
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" BLINK_SHARED)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" BLINK_STATIC)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBLINK_BUILD_TESTS=OFF
        -DBLINK_BUILD_BENCHMARKS=OFF
        -DBLINK_BUILD_DEMO=OFF
        -DBLINK_BUILD_SHARED=${BLINK_SHARED}
        -DBLINK_BUILD_STATIC=${BLINK_STATIC}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME blink CONFIG_PATH share/blink)
vcpkg_copy_pdbs()

# gif_to_sk80 drives the library's internal classes, so it is only built
# alongside the static archive.
if(BLINK_STATIC)
    vcpkg_copy_tools(TOOL_NAMES gif_to_sk80 AUTO_CLEAN)
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
# The release archives ship a LICENSE next to the CMake package files; vcpkg
# expects the copyright at its own canonical location instead.
file(REMOVE "${CURRENT_PACKAGES_DIR}/share/blink/LICENSE")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
