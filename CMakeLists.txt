cmake_minimum_required(VERSION 3.16)

# Overridable so the release pipeline can stamp the git tag onto the build
# without anyone having to edit this file before tagging.
set(BLINK_VERSION "0.2.0" CACHE STRING "Version stamped onto the blink library")

project(blink VERSION ${BLINK_VERSION} LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BLINK_BUILD_TESTS "Build the KeyboardTest suite" ON)
option(BLINK_BUILD_BENCHMARKS "Build the KeyboardBenchmark suite" ON)
option(BLINK_BUILD_DEMO "Build the interactive demo application" ON)
# Package managers build one linkage at a time; release archives ship both.
option(BLINK_BUILD_SHARED "Build the shared library" ON)
option(BLINK_BUILD_STATIC "Build the static library" ON)
# The C ABI in blink.h is the supported interface. The internal C++ headers are
# installed as well by default, because gif_to_sk80 and other static consumers
# still drive the classes directly; turn this off for an FFI-only package.
option(BLINK_INSTALL_CXX_HEADERS "Install the internal C++ headers alongside the C API" ON)

if(NOT BLINK_BUILD_SHARED AND NOT BLINK_BUILD_STATIC)
    message(FATAL_ERROR "At least one of BLINK_BUILD_SHARED / BLINK_BUILD_STATIC must be ON")
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

set(BLINK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/main")
set(BLINK_DEMO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/demo")

# ---------------------------------------------------------------------------
# Platform USB backend
# ---------------------------------------------------------------------------
if(WIN32)
    # Windows talks to the device through the native SetupAPI/hidsdi stack.
    set(USB_SOURCES "src/blink/usb_functions_win.cpp")
    set(BLINK_HIDAPI_PROVIDER "none")
else()
    set(USB_SOURCES "src/blink/usb_functions_linux.cpp")
    # vcpkg installs a CMake package config; prefer it so every CI leg resolves
    # hidapi identically regardless of what the host distro happens to ship.
    find_package(hidapi CONFIG QUIET)
    if(hidapi_FOUND)
        set(BLINK_HIDAPI_PROVIDER "config")
        if(TARGET hidapi::libusb)
            set(BLINK_HIDAPI_TARGET hidapi::libusb)
        else()
            set(BLINK_HIDAPI_TARGET hidapi::hidapi)
        endif()
    else()
        # Fallback for distro builds against libhidapi-dev.
        set(BLINK_HIDAPI_PROVIDER "pkgconfig")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(HIDAPI REQUIRED IMPORTED_TARGET hidapi-libusb)
        set(BLINK_HIDAPI_TARGET PkgConfig::HIDAPI)
    endif()
endif()

# stb is header-only and is consumed only from blink's own translation units.
find_path(STB_INCLUDE_DIRS "stb_image.h")
if(NOT STB_INCLUDE_DIRS)
    find_package(Stb REQUIRED)
    set(STB_INCLUDE_DIRS "${Stb_INCLUDE_DIR}")
endif()

# Main is kind of a demo app meant for showcasing the project and testing blink.dll
# It's nice keeping main completely isolated from the source files involved with blink and vice versa
if(BLINK_BUILD_DEMO)
add_executable(main "src/demo/main.cpp"
	"src/demo/blink_loader.cpp" "src/demo/blink_loader.h"
	"src/demo/activities/push_to_light.cpp" "src/demo/activities/push_to_light.h"
	"src/demo/activities/cycle_keyids.cpp" "src/demo/activities/cycle_keyids.h"
	"include/main/misc.h" "src/demo/choose_keyboard.cpp" "src/demo/choose_keyboard.h"
	"src/demo/choose_activity.cpp" "src/demo/choose_activity.h"
	"src/demo/util.cpp" "src/demo/util.h"
	"src/demo/activities/test_single_key.cpp" "src/demo/activities/test_single_key.h")
target_include_directories(main PRIVATE "${BLINK_INCLUDE_DIR}" "${BLINK_DEMO_DIR}")
# The demo resolves every entry point through dlopen/LoadLibrary rather than
# linking, so blink.h must not decorate the declarations with dllimport.
target_compile_definitions(main PRIVATE BLINK_STATIC)
if(NOT WIN32)
    target_link_libraries(main PRIVATE dl)
endif()
endif()


# blink is what other programs link against in order to perform USB work.
# The same translation units are compiled into a shared library (for the
# dlopen/LoadLibrary consumers this project ships) and a static archive.
set(BLINK_SOURCES
	"src/blink/blink_capi.cpp" "include/main/blink.h"
	"src/blink/print.cpp" "include/main/print.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	${USB_SOURCES} "include/main/usb_functions.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	"include/main/keyboards/abstract_keyboard.h"
    "include/main/keyboards/sk80/sk80.h"

	"include/main/keyboards/sk80/constants_sk80.h" "include/main/keyboards/sk80/messages_sk80.h" "include/main/keyboards/sk80/key_mappings_sk80.h"
    "include/main/keyboards/rk84/constants_rk84.h"
	"src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h"
	"include/main/keyboards/rk84/key_mappings_rk84.h"
	"include/main/misc.h"
	"src/blink/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h"
	"src/blink/keyboards/keyboard_spec_factory.cpp" "include/main/keyboards/keyboard_spec_factory.h" "include/main/keyboards/rk84/messages_rk84.h" "src/blink/keyboards/sk80/sk80.cpp"
	"src/blink/keyboard_lcd.cpp" "include/main/keyboard_lcd.h"
	"src/blink/usb_io_real.cpp" "include/main/usb_io.h")

set(BLINK_LIBRARY_TARGETS "")

if(BLINK_BUILD_SHARED)
    add_library(blink SHARED ${BLINK_SOURCES})
    list(APPEND BLINK_LIBRARY_TARGETS blink)
    set_target_properties(blink PROPERTIES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "${PROJECT_VERSION_MAJOR}")
    # Only the shared build exports symbols.
    target_compile_definitions(blink PRIVATE BUILD_DLL)
endif()

if(BLINK_BUILD_STATIC)
    add_library(blink_static STATIC ${BLINK_SOURCES})
    list(APPEND BLINK_LIBRARY_TARGETS blink_static)
    # libblink.so and libblink.a coexist happily, but on MSVC blink.lib is
    # already taken by the shared target's import library.
    if(MSVC AND BLINK_BUILD_SHARED)
        set_target_properties(blink_static PROPERTIES OUTPUT_NAME "blink_static")
    else()
        set_target_properties(blink_static PROPERTIES OUTPUT_NAME "blink")
    endif()
    # Consumers of the static archive must see plain declarations rather than
    # __declspec(dllimport).
    target_compile_definitions(blink_static PUBLIC BLINK_STATIC)
endif()

foreach(blink_target IN LISTS BLINK_LIBRARY_TARGETS)
    target_include_directories(${blink_target}
        PUBLIC
            "$<BUILD_INTERFACE:${BLINK_INCLUDE_DIR}>"
            # Both entries so that consumers may spell the include either
            # `#include <blink/blink.h>` or `#include <blink.h>`.
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/blink>"
        PRIVATE
            "${STB_INCLUDE_DIRS}")
    set_target_properties(${blink_target} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
    # Reported by blink_version_string() so a foreign caller can check what it
    # actually loaded.
    target_compile_definitions(${blink_target} PRIVATE
        BLINK_VERSION_STRING="${PROJECT_VERSION}")
endforeach()

if(NOT WIN32)
    # The shared library fully encapsulates hidapi; the static archive cannot,
    # so it propagates the dependency to whoever links it.
    if(BLINK_BUILD_SHARED)
        target_link_libraries(blink PRIVATE ${BLINK_HIDAPI_TARGET})
        # vcpkg hands us static hidapi/libusb archives, whose objects carry
        # default visibility. Without this, libblink.so would re-export the
        # whole libusb ABI and could interpose on a consumer's own copy of it
        # at runtime.
        target_link_options(blink PRIVATE "LINKER:--exclude-libs,ALL")
    endif()
    if(BLINK_BUILD_STATIC)
        target_link_libraries(blink_static PUBLIC ${BLINK_HIDAPI_TARGET})
    endif()
endif()

# gif_to_sk80 uploads GIF/image files to the SK80 LCD display.
# It drives the internal C++ classes (KeyboardLcd, RealUsbIO) rather than the
# extern "C" API, and those are deliberately not exported from the shared
# library, so it links the static archive and ships self-contained.
if(BLINK_BUILD_STATIC)
    add_executable(gif_to_sk80 "src/gif_to_sk80/main.cpp")
    target_link_libraries(gif_to_sk80 PRIVATE blink_static)
    if(NOT WIN32)
        target_link_libraries(gif_to_sk80 PRIVATE dl)
    endif()
endif()

# ---------------------------------------------------------------------------
# Install / export so downstream projects can `find_package(blink CONFIG)`
# ---------------------------------------------------------------------------
install(TARGETS ${BLINK_LIBRARY_TARGETS}
    EXPORT blinkTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

if(BLINK_BUILD_STATIC)
    install(TARGETS gif_to_sk80 RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

# The public C ABI. This is the only header a foreign-language consumer needs,
# and the only one guaranteed to match the exported symbols.
install(FILES "${BLINK_INCLUDE_DIR}/blink.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/blink")

if(BLINK_INSTALL_CXX_HEADERS)
    # The internal C++ headers. Usable only against the static archive: the
    # classes they declare are hidden in the shared library.
    install(DIRECTORY "${BLINK_INCLUDE_DIR}/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/blink"
        FILES_MATCHING PATTERN "*.h")
endif()

install(EXPORT blinkTargets
    FILE blinkTargets.cmake
    NAMESPACE blink::
    DESTINATION "${CMAKE_INSTALL_DATADIR}/blink")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/blinkConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/blinkConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/blink")

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/blinkConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/blinkConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/blinkConfigVersion.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/blink")

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(BLINK_BUILD_TESTS)
enable_testing()

# Add GoogleTest
add_executable(KeyboardTest
	"test/keyboard_test.cpp" "test/keyboard_rk84_test.cpp" "test/keyboard_sk80_test.cpp"
	"src/demo/blink_loader.cpp" "src/demo/blink_loader.h"
	"src/blink/blink_capi.cpp" "include/main/blink.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	"src/blink/print.cpp" "include/main/print.h"
	${USB_SOURCES} "include/main/usb_functions.h"
	"src/blink/messages.cpp" "include/main/messages.h"
    "include/main/keyboards/sk80/sk80.h"
	"include/main/keyboards/sk80/constants_sk80.h" "include/main/keyboards/sk80/messages_sk80.h" "include/main/keyboards/sk80/key_mappings_sk80.h"
    "src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h"
    "include/main/keyboards/rk84/constants_rk84.h" "include/main/keyboards/rk84/messages_rk84.h"
	"include/main/keyboards/abstract_keyboard.h"
	"include/main/misc.h"
	"src/blink/keyboards/keyboard_spec_factory.cpp" "include/main/keyboards/keyboard_spec_factory.h"
	"src/blink/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h"
    "src/demo/choose_keyboard.cpp" "src/demo/choose_keyboard.h" "src/demo/choose_activity.cpp" "src/demo/choose_activity.h"
    "src/demo/util.cpp" "src/demo/util.h" "src/demo/activities/test_single_key.cpp" "src/demo/activities/test_single_key.h" "src/blink/keyboards/sk80/sk80.cpp"
	"test/gif_to_sk80_test.cpp"
	"src/blink/keyboard_lcd.cpp" "include/main/keyboard_lcd.h" "include/main/usb_io.h"
	"src/blink/usb_io_real.cpp")
target_include_directories(KeyboardTest PRIVATE
    "${BLINK_INCLUDE_DIR}"
    "${BLINK_DEMO_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/test"
    "${STB_INCLUDE_DIRS}")

find_package(GTest CONFIG REQUIRED)
target_link_libraries(KeyboardTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_compile_definitions(KeyboardTest PRIVATE
    TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    BLINK_STATIC
    BLINK_VERSION_STRING="${PROJECT_VERSION}")

if(NOT WIN32)
    target_link_libraries(KeyboardTest PRIVATE ${BLINK_HIDAPI_TARGET} dl)
endif()

add_test(NAME AllTestsInMain COMMAND KeyboardTest)
endif()

# ---------------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------------
if(BLINK_BUILD_BENCHMARKS)
find_package(benchmark CONFIG REQUIRED)
add_executable(KeyboardBenchmark "benchmark/rk84_benchmark.cpp")
target_include_directories(KeyboardBenchmark PRIVATE "${BLINK_INCLUDE_DIR}")
target_link_libraries(KeyboardBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)
endif()

