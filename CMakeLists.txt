cmake_minimum_required(VERSION 3.16)

# Overridable so the release pipeline can stamp the git tag onto the build
# without anyone having to edit this file before tagging.
set(KEYLT_VERSION "0.2.0" CACHE STRING "Version stamped onto the keylt library")

# Release tags may carry a SemVer pre-release suffix (v1.0.1-rc.3), but
# project(VERSION) only accepts numeric major[.minor[.patch[.tweak]]] and errors
# out on anything else. Split the two apart: the numeric core drives project(),
# SOVERSION and the package version files, while the full string is what
# keylt_version_string() reports, so a release candidate stays identifiable at
# run time.
if(NOT KEYLT_VERSION MATCHES "^([0-9]+(\\.[0-9]+)*)")
    message(FATAL_ERROR
        "KEYLT_VERSION must begin with a numeric version such as 1.2.3, "
        "but is '${KEYLT_VERSION}'")
endif()
set(KEYLT_VERSION_CORE "${CMAKE_MATCH_1}")

project(keylt VERSION ${KEYLT_VERSION_CORE} LANGUAGES CXX)

if(NOT KEYLT_VERSION STREQUAL KEYLT_VERSION_CORE)
    message(STATUS "keylt ${KEYLT_VERSION} (pre-release of ${KEYLT_VERSION_CORE})")
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(KEYLT_BUILD_TESTS "Build the KeyboardTest suite" ON)
option(KEYLT_BUILD_BENCHMARKS "Build the KeyboardBenchmark suite" ON)
option(KEYLT_BUILD_DEMO "Build the interactive demo application" ON)
# Package managers build one linkage at a time; release archives ship both.
option(KEYLT_BUILD_SHARED "Build the shared library" ON)
option(KEYLT_BUILD_STATIC "Build the static library" ON)
# The C ABI in keylt.h is the supported interface. The internal C++ headers are
# installed as well by default, because the CLI and other static consumers
# still drive the classes directly; turn this off for an FFI-only package.
option(KEYLT_INSTALL_CXX_HEADERS "Install the internal C++ headers alongside the C API" ON)

if(NOT KEYLT_BUILD_SHARED AND NOT KEYLT_BUILD_STATIC)
    message(FATAL_ERROR "At least one of KEYLT_BUILD_SHARED / KEYLT_BUILD_STATIC must be ON")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options("/utf-8")
endif()

if(WIN32)
    # platform.h defines this before its own <windows.h> include, but a
    # translation unit that reaches <windows.h> through a third-party header
    # first would still get the min/max macros and break std::min/std::max.
    add_compile_definitions(NOMINMAX)
endif()

set(KEYLT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/main")
set(KEYLT_DEMO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/demo")

# ---------------------------------------------------------------------------
# Platform USB backend
# ---------------------------------------------------------------------------
if(WIN32)
    # Windows talks to the device through the native SetupAPI/hidsdi stack.
    set(USB_SOURCES "src/libkeylt/usb_functions_win.cpp")
    set(KEYLT_HIDAPI_PROVIDER "none")
else()
    set(USB_SOURCES "src/libkeylt/usb_functions_linux.cpp")
    # vcpkg installs a CMake package config; prefer it so every CI leg resolves
    # hidapi identically regardless of what the host distro happens to ship.
    find_package(hidapi CONFIG QUIET)
    if(hidapi_FOUND)
        set(KEYLT_HIDAPI_PROVIDER "config")
        if(TARGET hidapi::libusb)
            set(KEYLT_HIDAPI_TARGET hidapi::libusb)
        else()
            set(KEYLT_HIDAPI_TARGET hidapi::hidapi)
        endif()
    else()
        # Fallback for distro builds against libhidapi-dev.
        set(KEYLT_HIDAPI_PROVIDER "pkgconfig")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(HIDAPI REQUIRED IMPORTED_TARGET hidapi-libusb)
        set(KEYLT_HIDAPI_TARGET PkgConfig::HIDAPI)
    endif()
endif()

# stb is header-only and is consumed only from keylt's own translation units.
find_path(STB_INCLUDE_DIRS "stb_image.h")
if(NOT STB_INCLUDE_DIRS)
    find_package(Stb REQUIRED)
    set(STB_INCLUDE_DIRS "${Stb_INCLUDE_DIR}")
endif()

# Main is kind of a demo app meant for showcasing the project and testing keylt.dll
# It's nice keeping main completely isolated from the source files involved with keylt and vice versa
if(KEYLT_BUILD_DEMO)
add_executable(main "src/demo/main.cpp"
	"src/demo/keylt_loader.cpp" "src/demo/keylt_loader.h"
	"src/demo/activities/push_to_light.cpp" "src/demo/activities/push_to_light.h"
	"src/demo/activities/cycle_keyids.cpp" "src/demo/activities/cycle_keyids.h"
	"include/main/misc.h" "src/demo/choose_keyboard.cpp" "src/demo/choose_keyboard.h"
	"src/demo/choose_activity.cpp" "src/demo/choose_activity.h"
	"src/demo/util.cpp" "src/demo/util.h"
	"src/demo/activities/test_single_key.cpp" "src/demo/activities/test_single_key.h")
target_include_directories(main PRIVATE "${KEYLT_INCLUDE_DIR}" "${KEYLT_DEMO_DIR}")
# The demo resolves every entry point through dlopen/LoadLibrary rather than
# linking, so keylt.h must not decorate the declarations with dllimport.
target_compile_definitions(main PRIVATE KEYLT_STATIC)
if(NOT WIN32)
    target_link_libraries(main PRIVATE dl)
endif()
endif()


# keylt is what other programs link against in order to perform USB work.
# The same translation units are compiled into a shared library (for the
# dlopen/LoadLibrary consumers this project ships) and a static archive.
set(KEYLT_SOURCES
	"src/libkeylt/keylt_capi.cpp" "include/main/keylt.h"
	"src/libkeylt/print.cpp" "include/main/print.h"
	"src/libkeylt/messages.cpp" "include/main/messages.h"
	${USB_SOURCES} "include/main/usb_functions.h"
	"src/libkeylt/keyboard.cpp" "include/main/keyboard.h"
	"include/main/keyboards/abstract_keyboard.h"
    "include/main/keyboards/sk80/sk80.h"

	"include/main/keyboards/sk80/constants_sk80.h" "include/main/keyboards/sk80/messages_sk80.h" "include/main/keyboards/sk80/key_mappings_sk80.h"
    "include/main/keyboards/rk84/constants_rk84.h"
	"src/libkeylt/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h"
	"include/main/keyboards/rk84/key_mappings_rk84.h"
	"include/main/misc.h"
	"src/libkeylt/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h"
	"src/libkeylt/keyboards/keyboard_spec_factory.cpp" "include/main/keyboards/keyboard_spec_factory.h" "include/main/keyboards/rk84/messages_rk84.h" "src/libkeylt/keyboards/sk80/sk80.cpp"
	"src/libkeylt/keyboard_lcd.cpp" "include/main/keyboard_lcd.h"
	"src/libkeylt/usb_io_real.cpp" "include/main/usb_io.h")

set(KEYLT_LIBRARY_TARGETS "")

if(KEYLT_BUILD_SHARED)
    add_library(keylt SHARED ${KEYLT_SOURCES})
    list(APPEND KEYLT_LIBRARY_TARGETS keylt)
    set_target_properties(keylt PROPERTIES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}")
    # Only the shared build exports symbols.
    target_compile_definitions(keylt PRIVATE BUILD_DLL)
endif()

if(KEYLT_BUILD_STATIC)
    add_library(keylt_static STATIC ${KEYLT_SOURCES})
    list(APPEND KEYLT_LIBRARY_TARGETS keylt_static)
    # libkeylt.so and libkeylt.a coexist happily, but on MSVC keylt.lib is
    # already taken by the shared target's import library.
    if(MSVC AND KEYLT_BUILD_SHARED)
        set_target_properties(keylt_static PROPERTIES OUTPUT_NAME "keylt_static")
    else()
        set_target_properties(keylt_static PROPERTIES OUTPUT_NAME "keylt")
    endif()
    # Consumers of the static archive must see plain declarations rather than
    # __declspec(dllimport).
    target_compile_definitions(keylt_static PUBLIC KEYLT_STATIC)
endif()

foreach(keylt_target IN LISTS KEYLT_LIBRARY_TARGETS)
    target_include_directories(${keylt_target}
        PUBLIC
            "$<BUILD_INTERFACE:${KEYLT_INCLUDE_DIR}>"
            # Both entries so that consumers may spell the include either
            # `#include <keylt/keylt.h>` or `#include <keylt.h>`.
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/keylt>"
        PRIVATE
            "${STB_INCLUDE_DIRS}")
    set_target_properties(${keylt_target} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
    # Reported by keylt_version_string() so a foreign caller can check what it
    # actually loaded. This is the full version, pre-release suffix included:
    # PROJECT_VERSION has had any such suffix stripped for project()'s sake.
    target_compile_definitions(${keylt_target} PRIVATE
        KEYLT_VERSION_STRING="${KEYLT_VERSION}")
endforeach()

if(NOT WIN32)
    # The shared library fully encapsulates hidapi; the static archive cannot,
    # so it propagates the dependency to whoever links it.
    if(KEYLT_BUILD_SHARED)
        target_link_libraries(keylt PRIVATE ${KEYLT_HIDAPI_TARGET})
        # vcpkg hands us static hidapi/libusb archives, whose objects carry
        # default visibility. Without this, libkeylt.so would re-export the
        # whole libusb ABI and could interpose on a consumer's own copy of it
        # at runtime.
        target_link_options(keylt PRIVATE "LINKER:--exclude-libs,ALL")
    endif()
    if(KEYLT_BUILD_STATIC)
        target_link_libraries(keylt_static PUBLIC ${KEYLT_HIDAPI_TARGET})
    endif()
endif()

# keylt CLI — a multi-function command-line tool.
# It drives the internal C++ classes (KeyboardLcd, RealUsbIO) rather than the
# extern "C" API, and those are deliberately not exported from the shared
# library, so it links the static archive and ships self-contained.
if(KEYLT_BUILD_STATIC)
    add_executable(keylt_cli "src/keylt/main.cpp")
    set_target_properties(keylt_cli PROPERTIES OUTPUT_NAME "keylt")
    target_link_libraries(keylt_cli PRIVATE keylt_static)
    if(NOT WIN32)
        target_link_libraries(keylt_cli PRIVATE dl)
    endif()
endif()

# ---------------------------------------------------------------------------
# Install / export so downstream projects can `find_package(keylt CONFIG)`
# ---------------------------------------------------------------------------
install(TARGETS ${KEYLT_LIBRARY_TARGETS}
    EXPORT keyltTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

if(KEYLT_BUILD_STATIC)
    install(TARGETS keylt_cli RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# The public C ABI. This is the only header a foreign-language consumer needs,
# and the only one guaranteed to match the exported symbols.
install(FILES "${KEYLT_INCLUDE_DIR}/keylt.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/keylt")

if(KEYLT_INSTALL_CXX_HEADERS)
    # The internal C++ headers. Usable only against the static archive: the
    # classes they declare are hidden in the shared library.
    install(DIRECTORY "${KEYLT_INCLUDE_DIR}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/keylt"
        FILES_MATCHING PATTERN "*.h")
endif()

install(EXPORT keyltTargets
    FILE keyltTargets.cmake
    NAMESPACE keylt::
    DESTINATION "${CMAKE_INSTALL_DATADIR}/keylt")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/keyltConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/keyltConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/keylt")

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/keyltConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/keyltConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/keyltConfigVersion.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/keylt")

# ---------------------------------------------------------------------------
# pkg-config, for consumers that cannot run CMake
#
# The CMake package above is the richer interface, but foreign-language
# toolchains generally speak pkg-config instead -- the Rust crates in rust/ use
# it as one of their discovery modes.
# ---------------------------------------------------------------------------
if(MSVC)
    set(KEYLT_PKGCONFIG_DEFAULT OFF)
else()
    set(KEYLT_PKGCONFIG_DEFAULT ON)
endif()
option(KEYLT_INSTALL_PKGCONFIG "Install a keylt.pc file for pkg-config"
    ${KEYLT_PKGCONFIG_DEFAULT})

if(KEYLT_INSTALL_PKGCONFIG)
    # The .pc file locates the prefix relative to itself, which keeps the
    # release archives usable after extraction and makes
    # `cmake --install --prefix` work: the configure-time prefix is not baked in.
    # Computing the hop count here rather than hard-coding "../.." keeps lib64
    # and Debian multiarch layouts correct.
    file(RELATIVE_PATH KEYLT_PC_PREFIX_FROM_PCFILEDIR
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig"
        "${CMAKE_INSTALL_PREFIX}")
    string(REGEX REPLACE "/$" "" KEYLT_PC_PREFIX_FROM_PCFILEDIR
        "${KEYLT_PC_PREFIX_FROM_PCFILEDIR}")

    # GNUInstallDirs allows either form, and a .pc file has to interpolate the
    # relative one against ${prefix} itself.
    if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
        set(KEYLT_PC_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
    else()
        set(KEYLT_PC_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
    endif()
    if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
        set(KEYLT_PC_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
    else()
        set(KEYLT_PC_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    endif()

    # These are what `keylt_static` re-exposes; the shared library hides them.
    if(WIN32)
        set(KEYLT_PC_REQUIRES_PRIVATE "")
        set(KEYLT_PC_LIBS_PRIVATE "-lsetupapi -lhid")
    else()
        set(KEYLT_PC_REQUIRES_PRIVATE "hidapi-libusb")
        set(KEYLT_PC_LIBS_PRIVATE "-lstdc++")
    endif()

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/keylt.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/keylt.pc"
        @ONLY)

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/keylt.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(KEYLT_BUILD_TESTS)
enable_testing()

# Add GoogleTest
add_executable(KeyboardTest
	"test/keyboard_test.cpp" "test/keyboard_rk84_test.cpp" "test/keyboard_sk80_test.cpp"
	"src/demo/keylt_loader.cpp" "src/demo/keylt_loader.h"
	"src/libkeylt/keylt_capi.cpp" "include/main/keylt.h"
	"src/libkeylt/keyboard.cpp" "include/main/keyboard.h"
	"src/libkeylt/print.cpp" "include/main/print.h"
	${USB_SOURCES} "include/main/usb_functions.h"
	"src/libkeylt/messages.cpp" "include/main/messages.h"
    "include/main/keyboards/sk80/sk80.h"
	"include/main/keyboards/sk80/constants_sk80.h" "include/main/keyboards/sk80/messages_sk80.h" "include/main/keyboards/sk80/key_mappings_sk80.h"
    "src/libkeylt/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h"
    "include/main/keyboards/rk84/constants_rk84.h" "include/main/keyboards/rk84/messages_rk84.h"
	"include/main/keyboards/abstract_keyboard.h"
	"include/main/misc.h"
	"src/libkeylt/keyboards/keyboard_spec_factory.cpp" "include/main/keyboards/keyboard_spec_factory.h"
	"src/libkeylt/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h"
    "src/demo/choose_keyboard.cpp" "src/demo/choose_keyboard.h" "src/demo/choose_activity.cpp" "src/demo/choose_activity.h"
    "src/demo/util.cpp" "src/demo/util.h" "src/demo/activities/test_single_key.cpp" "src/demo/activities/test_single_key.h" "src/libkeylt/keyboards/sk80/sk80.cpp"
	"test/gif_to_sk80_test.cpp"
	"src/libkeylt/keyboard_lcd.cpp" "include/main/keyboard_lcd.h" "include/main/usb_io.h"
	"src/libkeylt/usb_io_real.cpp")
target_include_directories(KeyboardTest PRIVATE
    "${KEYLT_INCLUDE_DIR}"
    "${KEYLT_DEMO_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/test"
    "${STB_INCLUDE_DIRS}")

find_package(GTest CONFIG REQUIRED)
target_link_libraries(KeyboardTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_compile_definitions(KeyboardTest PRIVATE
    TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    KEYLT_STATIC
    KEYLT_VERSION_STRING="${KEYLT_VERSION}")

if(NOT WIN32)
    target_link_libraries(KeyboardTest PRIVATE ${KEYLT_HIDAPI_TARGET} dl)
endif()

add_test(NAME AllTestsInMain COMMAND KeyboardTest)
endif()

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------
if(KEYLT_BUILD_BENCHMARKS)
find_package(benchmark CONFIG REQUIRED)
add_executable(KeyboardBenchmark "benchmark/rk84_benchmark.cpp")
target_include_directories(KeyboardBenchmark PRIVATE "${KEYLT_INCLUDE_DIR}")
target_link_libraries(KeyboardBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)
endif()

