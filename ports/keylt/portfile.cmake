# Overlay port template.
#
# The @REPO@ / @REF@ / @SHA512@ placeholders are substituted by
# .github/workflows/release.yaml when a tag is published; the rendered result
# is attached to the GitHub release as keylt-vcpkg-port.tar.gz.
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
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" KEYLT_SHARED)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" KEYLT_STATIC)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DKEYLT_BUILD_TESTS=OFF
        -DKEYLT_BUILD_BENCHMARKS=OFF
        -DKEYLT_BUILD_DEMO=OFF
        -DKEYLT_BUILD_SHARED=${KEYLT_SHARED}
        -DKEYLT_BUILD_STATIC=${KEYLT_STATIC}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME keylt CONFIG_PATH share/keylt)
# The install rules emit a keylt.pc for consumers that cannot run CMake, such as
# the Rust crates. vcpkg's post-build check rejects one whose paths have not
# been rewritten for the package layout.
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

# The keylt CLI drives the library's internal classes, so it is only built
# alongside the static archive.
if(KEYLT_STATIC)
    vcpkg_copy_tools(TOOL_NAMES keylt AUTO_CLEAN)
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
# The release archives ship a LICENSE next to the CMake package files; vcpkg
# expects the copyright at its own canonical location instead.
file(REMOVE "${CURRENT_PACKAGES_DIR}/share/keylt/LICENSE")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
